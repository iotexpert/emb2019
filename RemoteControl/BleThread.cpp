
#include "mbed.h"
#include "global.h"
#include "DisplayThread.h"
#include "CapSenseThread.h"
#include <events/mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "ble/gap/Gap.h"
#include "ble/GattClient.h"
#include "ble/DiscoveredService.h"
#include "ble/DiscoveredCharacteristic.h"
#include "ble/CharacteristicDescriptorDiscovery.h"

#include "pretty_printer.h"

//#define dbg_printf(...)
#define dbg_printf printf

const static char DEVICE_NAME[] = "Remote";
EventQueue event_queue(/* event count */ 10 * EVENTS_EVENT_SIZE);


static DiscoveredCharacteristic waterLevelLeft;
static DiscoveredCharacteristic waterLevelRight;
static DiscoveredCharacteristic pumpRight;
static DiscoveredCharacteristic pumpLeft;


//#define __UUID_CONTROLLER                                       0x21, 0xc0, 0x4d, 0x09, 0xc8, 0x84, 0x4a, 0xf1, 0x96, 0xa9, 0x52, 0xe4, 0xe4, 0xba, 0x19, 0x5b

const UUID               GAME_SERVICE_UUID("5b19bae4-e452-a996-f14a-84c8094dc021");
const UUID  WATER_LEFT_CHARACTERISTIC_UUID("6397ca92-5f02-1eb9-3d4a-316b4300501e");
const UUID WATER_RIGHT_CHARACTERISTIC_UUID("02c095ac-d980-f682-df4f-8d3db03e9e1b");
const UUID   PUMP_LEFT_CHARACTERISTIC_UUID("66bb5c46-c5c9-7495-0d4e-206effd357ad");
const UUID  PUMP_RIGHT_CHARACTERISTIC_UUID("7cf6e97f-5ae4-6694-a14e-07086ebf9b76");

void service_discovery(const DiscoveredService *service) {
    if (service->getUUID().shortOrLong() == UUID::UUID_TYPE_SHORT) {
        printf("S UUID-%x attrs[%u %u]\r\n", service->getUUID().getShortUUID(), service->getStartHandle(), service->getEndHandle());
    } else {
        printf("S UUID-");
        const uint8_t *longUUIDBytes = service->getUUID().getBaseUUID();
        for (unsigned i = 0; i < UUID::LENGTH_OF_LONG_UUID; i++) {
            printf("%02x", longUUIDBytes[i]);
        }
        printf(" attrs[%u %u]\r\n", service->getStartHandle(), service->getEndHandle());
    }
}


void characteristic_discovery(const DiscoveredCharacteristic *characteristicP) {
		if(characteristicP->getUUID() == WATER_LEFT_CHARACTERISTIC_UUID)
		{
			dbg_printf("Found water left %d\n",characteristicP->getValueHandle());
			waterLevelLeft = *characteristicP;
		}

		if(characteristicP->getUUID() == WATER_RIGHT_CHARACTERISTIC_UUID)
		{
			dbg_printf("Found water right %d\n",characteristicP->getValueHandle());
			waterLevelRight = *characteristicP;

		}

		if(characteristicP->getUUID() == PUMP_LEFT_CHARACTERISTIC_UUID)
		{
			dbg_printf("Found pump left %d\n",characteristicP->getValueHandle());
			pumpLeft = *characteristicP;

		}

		if(characteristicP->getUUID() == PUMP_RIGHT_CHARACTERISTIC_UUID)
		{
			dbg_printf("Found pump right %d\n",characteristicP->getValueHandle());
			pumpRight = *characteristicP;

		}

}

#if 0
void when_descriptor_discovered(const DiscoveryCallbackParams_t* event)
{
		printf("\tDescriptor discovered at %u, UUID: ", event->descriptor.getAttributeHandle());
	//	print_uuid(event->descriptor.getUUID());
		printf(".\r\n");
		if (event->descriptor.getUUID() == BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG) {
			#if 0

				_descriptor_handle = event->descriptor.getAttributeHandle();
				_client->terminateCharacteristicDescriptorDiscovery(
						event->characteristic
				);
				#endif
		}
}

void when_descriptor_discovery_ends(const TerminationCallbackParams_t *event) {
	dbg_printf("blethread: descriptor discovery over\n");
	/*
		// shall never happen but happen with android devices ...
		// process the next charateristic
		if (!_descriptor_handle) {
				printf("\tWarning: characteristic with notify or indicate attribute without CCCD.\r\n");
				process_next_characteristic();
				return;
		}

		Properties_t properties = _it->value.getProperties();

		uint16_t cccd_value =
				(properties.notify() << 0) | (properties.indicate() << 1);

		ble_error_t error = _client->write(
				GattClient::GATT_OP_WRITE_REQ,
				_connection_handle,
				_descriptor_handle,
				sizeof(cccd_value),
				reinterpret_cast<uint8_t*>(&cccd_value)
		);

		if (error) {
				printf(
						"Error: cannot initiate write of CCCD %u due to %u.\r\n",
						_descriptor_handle, error
				);
				stop();
		}
		*/
}

#endif

void discovery_termination(Gap::Handle_t connectionHandle) {
    dbg_printf("terminated SD for handle %u\r\n", connectionHandle);
		DisplayMessage_t *msg;

		msg = displayPool.alloc();
		msg->command = GAME_SCREEN;
		msg->type = INIT_BLE;
		displayQueue.put(msg);
		gameMode = MODE_GAME;

		// Turn on the CCCDs....
}


class LEDDemo : ble::Gap::EventHandler {
public:
LEDDemo(BLE &ble, events::EventQueue &event_queue) :
	_ble(ble),
	_event_queue(event_queue),
	connectedStatus(false),
	_adv_data_builder(_adv_buffer) {
}

void start() {
  dbg_printf("blethread: Started BLE\n");
	_ble.gap().setEventHandler(this);
	_ble.init(this, &LEDDemo::on_init_complete);
	_event_queue.dispatch_forever();
}

private:
/** Callback triggered when the ble initialization process has finished */
void on_init_complete(BLE::InitializationCompleteCallbackContext *params) {
	if (params->error != BLE_ERROR_NONE) {
		printf("Ble initialization failed.");
		return;
	}
	print_mac_address();
  //dbg_printf("blethread: calling start advertising\n");
	start_advertising();
}

void start_advertising() {
	/* Create advertising parameters and payload */

  dbg_printf("blethread: Started Advertising\n");
  ble::AdvertisingParameters adv_parameters(ble::advertising_type_t::CONNECTABLE_UNDIRECTED);
	_adv_data_builder.setFlags();

	const uint8_t mfgDataBytes[] = {0x31, 0x01, 'R', 'e', 'm', 'o', 't', 'e'};
	Span<const uint8_t> mfgData(mfgDataBytes,8);

	_adv_data_builder.setManufacturerSpecificData (mfgData);
	_adv_data_builder.setName(DEVICE_NAME);

	ble_error_t error = _ble.gap().setAdvertisingParameters(
		ble::LEGACY_ADVERTISING_HANDLE,
		adv_parameters
		);

	if (error) {
		printf("_ble.gap().setAdvertisingParameters() failed\r\n");
		return;
	}

	error = _ble.gap().setAdvertisingPayload(
		ble::LEGACY_ADVERTISING_HANDLE,
		_adv_data_builder.getAdvertisingData()
		);

	if (error) {
		printf("_ble.gap().setAdvertisingPayload() failed\r\n");
		return;
	}

	error = _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE,
																			ble::adv_duration_t(10),
																			0);

	if (error) {
		printf("_ble.gap().startAdvertising() failed\r\n");
		return;
	}
	DisplayMessage_t *msg;
	msg = displayPool.alloc();
	msg->command = BLE_SCREEN;
	msg->type = BLE_ADVERTISE;
	displayQueue.put(msg);

}

private:    /* Event handler */

void onDisconnectionComplete(const ble::DisconnectionCompleteEvent&) {
	_ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
	dbg_printf("DisconnectionCompleteEvent\n");
	connectedStatus = false;

}

void onConnectionComplete   (const ble::ConnectionCompleteEvent &event)
{
	dbg_printf("blethread: onConnectionComplete\n");
	DisplayMessage_t *msg;
	msg = displayPool.alloc();
	msg->command = BLE_SCREEN;
	msg->type = BLE_CONNECT;
	displayQueue.put(msg);

	connectedStatus = true;

	// Start service discovery

	dbg_printf("blethread: onConnectionComplete\n");

	_ble.gattClient().onServiceDiscoveryTermination(discovery_termination);
	_ble.gattClient().launchServiceDiscovery(
							event.getConnectionHandle(),
							service_discovery,
							characteristic_discovery,
						GAME_SERVICE_UUID);


}

void onAdvertisingEnd(const ble::AdvertisingEndEvent &event)
{
	dbg_printf("Advertising end\n" );
}

private:
BLE &_ble;
events::EventQueue &_event_queue;
bool connectedStatus;

//    UUID _led_uuid;
//    LEDService *_led_service;

uint8_t _adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
ble::AdvertisingDataBuilder _adv_data_builder;
};

/** Schedule processing of events from the BLE middleware in the event queue. */
void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context) {
	event_queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}

void processSwipeQueue()
{
	int32_t *swipeMsg;
	uint8_t val;
	osEvent evt;
	do {
		evt = swipeQueue.get(0);
		if (evt.status == osEventMessage) {
			swipeMsg = (int32_t *)evt.value.p;
			dbg_printf("bleThread:Received swipe=%d\n",(int)*swipeMsg);
			if(*swipeMsg < 0)
			{
				dbg_printf("left\n");
				val = (uint8_t)(*swipeMsg * -1);
//				ble_error_t bterr = pumpLeft.write(1,&val);
				ble_error_t bterr = pumpLeft.write(1,&val);
				dbg_printf("BTErr = %d\n", bterr);
			}
			else
			{
				dbg_printf("right\n");
				val = (uint8_t)*swipeMsg;
				ble_error_t bterr = pumpRight.write(1,&val);
				dbg_printf("BTErr = %d\n", bterr);
			}

			swipePool.free(swipeMsg);
		}
	} while (evt.status == osEventMessage);
}

void bleThread()
{
	dbg_printf("bleThread: Starting BLE Thread\n");

	DisplayMessage_t *msg;
	msg = displayPool.alloc();
	msg->command = BLE_SCREEN;
	msg->type = BLE_START;
	displayQueue.put(msg);

	BLE &ble = BLE::Instance();
	ble.onEventsToProcess(schedule_ble_events);
	event_queue.call_every(20, processSwipeQueue);

	LEDDemo remote(ble, event_queue);
	remote.start();

}
