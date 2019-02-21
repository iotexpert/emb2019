
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

EventQueue event_queue(/* event count */ 10 * EVENTS_EVENT_SIZE);

const UUID               GAME_SERVICE_UUID("5b19bae4-e452-a996-f14a-84c8094dc021");
const UUID  WATER_LEFT_CHARACTERISTIC_UUID("6397ca92-5f02-1eb9-3d4a-316b4300501e");
const UUID WATER_RIGHT_CHARACTERISTIC_UUID("02c095ac-d980-f682-df4f-8d3db03e9e1b");
const UUID   PUMP_LEFT_CHARACTERISTIC_UUID("66bb5c46-c5c9-7495-0d4e-206effd357ad");
const UUID  PUMP_RIGHT_CHARACTERISTIC_UUID("7cf6e97f-5ae4-6694-a14e-07086ebf9b76");
static const char DEVICE_NAME[] = "Remote";


class LEDDemo :           private mbed::NonCopyable<LEDDemo>,
                          public ble::Gap::EventHandler {

typedef LEDDemo Self;

public:
LEDDemo(BLE &ble, events::EventQueue &event_queue) :
	_ble(ble),
  _connection_handle(),
	_event_queue(event_queue),
	connectedStatus(false),
	_adv_data_builder(_adv_buffer) {
}


void start() {
  dbg_printf("blethread: Started BLE\n");
	_ble.gap().setEventHandler(this);
	_ble.init(this, &LEDDemo::on_init_complete);
  _event_queue.call_every(20, this,&LEDDemo::processSwipeQueue);

	_event_queue.dispatch_forever();
}

private:
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

private:

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


  template<typename ContextType>
FunctionPointerWithContext<ContextType> as_cb(
    void (Self::*member)(ContextType context)
) {
    return makeFunctionPointer(this, member);
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


void when_characteristic_read(const GattReadCallbackParams *read_event)
    {

        if(read_event->handle == waterLevelLeft.getValueHandle() )
        {
            waterLeft = read_event->data[0];
        }
        else if(read_event->handle == waterLevelRight.getValueHandle() )
        {
            waterRight = read_event->data[0];
        }
        else
        {
          printf("Characteristic value at %u equal to: ", read_event->handle);
          for (size_t i = 0; i <  read_event->len; ++i) {
              printf("0x%02X ", read_event->data[i]);
          }
          printf(".\r\n");
        }

        DisplayMessage_t *msg;
        msg = displayPool.alloc();
        msg->command = GAME_SCREEN;
        msg->type = WATER_VALUE;
        msg->val1 = waterLeft;
        msg->val2 = waterRight;
        displayQueue.put(msg);

    }

void when_characteristic_changed(const GattHVXCallbackParams* event)
{
	if(event->handle == waterLevelLeft.getValueHandle() )
	{
		waterLeft = event->data[0];
	}
	else if(event->handle == waterLevelRight.getValueHandle() )
	{
		waterRight = event->data[0];
	}
	else
	{
		printf("Change on attribute %u: new value = ", event->handle);
		for (size_t i = 0; i < event->len; ++i) {
			printf("0x%02X ", event->data[i]);
		}
		printf(".\r\n");
	}

  DisplayMessage_t *msg;
  msg = displayPool.alloc();
  msg->command = GAME_SCREEN;
  msg->type = WATER_VALUE;
  msg->val1 = waterLeft;
  msg->val2 = waterRight;
  displayQueue.put(msg);



}

void when_descriptor_discovered(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t* event)
{
	  if(event->characteristic.getDeclHandle() == waterLevelLeft.getDeclHandle() && event->descriptor.getUUID().getShortUUID() == BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG)
    {
      waterLeftCCCD = event->descriptor.getAttributeHandle();
      dbg_printf("Found waterleft CCCD %d\n",waterLeftCCCD);
    }

    if(event->characteristic.getDeclHandle() == waterLevelRight.getDeclHandle() && event->descriptor.getUUID().getShortUUID() == BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG)
    {
      waterRightCCCD = event->descriptor.getAttributeHandle();
      dbg_printf("Found waterright CCCD %d\n",waterRightCCCD);

    }
}

void when_descriptor_discovery_ends(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *event) {

	if(event->characteristic == waterLevelLeft)
	{
		dbg_printf("blethread: found water level left cccd\n");
		ble_error_t error = waterLevelRight.discoverDescriptors(
			as_cb(&Self::when_descriptor_discovered),
			as_cb(&Self::when_descriptor_discovery_ends));
		(void)error;
	}

	if(event->characteristic == waterLevelRight)
	{
		dbg_printf("blethread: found water level right cccd\n");

		DisplayMessage_t *msg;
		msg = displayPool.alloc();
		msg->command = GAME_SCREEN;
		msg->type = INIT_BLE;
		displayQueue.put(msg);
		gameMode = MODE_GAME;


    _ble.gattClient().onDataWritten().add(as_cb(&Self::when_waterright_cccd_written));

		uint16_t cccd_value = 0x01;
		dbg_printf("blethread: writing right CCCD %d\n",waterRightCCCD);
		ble_error_t error =   _ble.gattClient().write(
			GattClient::GATT_OP_WRITE_REQ,
			_connection_handle,
			waterRightCCCD,
			sizeof(cccd_value),
			reinterpret_cast<uint8_t*>(&cccd_value)
			);

		dbg_printf("CCCD right error=%d\n",error);

	}

}

void when_waterright_cccd_written(const GattWriteCallbackParams* event)
{
  dbg_printf("blethread: Right CCCD  written... \n");

  _ble.gattClient().onDataWritten().detach(as_cb(&Self::when_waterright_cccd_written));
  _ble.gattClient().onDataWritten().add(as_cb(&Self::when_waterleft_cccd_written));

  uint16_t cccd_value = 0x01;

  _ble.gattClient().write(
    GattClient::GATT_OP_WRITE_REQ,
    _connection_handle,
    waterLeftCCCD,
    sizeof(cccd_value),
    reinterpret_cast<uint8_t*>(&cccd_value));
}

void when_waterleft_cccd_written(const GattWriteCallbackParams* event)
{
  dbg_printf("blethread: Left CCCD  written... \n");
  _ble.gattClient().onDataWritten().detach(as_cb(&Self::when_waterleft_cccd_written));
  _ble.gattClient().onDataRead().add(as_cb(&Self::when_characteristic_read));
  _ble.gattClient().onHVX().add(as_cb(&Self::when_characteristic_changed));

}

void discovery_termination(Gap::Handle_t connectionHandle) {
    dbg_printf("terminated SD for handle %u\r\n", connectionHandle);

    ble_error_t error = waterLevelLeft.discoverDescriptors(
          as_cb(&Self::when_descriptor_discovered),
          as_cb(&Self::when_descriptor_discovery_ends));
          (void)error;
}



void onDisconnectionComplete(const ble::DisconnectionCompleteEvent&) {
	_ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
	dbg_printf("bleThread: DisconnectionCompleteEvent\n");

	connectedStatus = false;
	gameMode = MODE_CONNECT_BLE;
	DisplayMessage_t *msg;
	msg = displayPool.alloc();
	msg->command = BLE_SCREEN;
	msg->type = BLE_START;
	displayQueue.put(msg);

	start_advertising();
}

void onConnectionComplete(const ble::ConnectionCompleteEvent &event)
{
	dbg_printf("blethread: onConnectionComplete\n");
	DisplayMessage_t *msg;
	msg = displayPool.alloc();
	msg->command = BLE_SCREEN;
	msg->type = BLE_CONNECT;
	displayQueue.put(msg);

	connectedStatus = true;
  _connection_handle = event.getConnectionHandle();
	// Start service discovery

	dbg_printf("blethread: onConnectionComplete\n");

	_ble.gattClient().onServiceDiscoveryTermination(as_cb(&Self::discovery_termination));
	_ble.gattClient().launchServiceDiscovery(
							event.getConnectionHandle(),
							as_cb(&Self::service_discovery),
							as_cb(&Self::characteristic_discovery),
						GAME_SERVICE_UUID);
}

void onAdvertisingEnd(const ble::AdvertisingEndEvent &event)
{
	dbg_printf("Advertising end\n" );
}

private:
BLE &_ble;
Gap::Handle_t _connection_handle;
events::EventQueue &_event_queue;
bool connectedStatus;

DiscoveredCharacteristic waterLevelLeft;
DiscoveredCharacteristic waterLevelRight;
DiscoveredCharacteristic pumpRight;
DiscoveredCharacteristic pumpLeft;

GattAttribute::Handle_t waterLeftCCCD;
GattAttribute::Handle_t waterRightCCCD;


uint8_t waterLeft=0;
uint8_t waterRight=0;

uint8_t _adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
ble::AdvertisingDataBuilder _adv_data_builder;
};

/** Schedule processing of events from the BLE middleware in the event queue. */
void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context) {
	event_queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
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

	LEDDemo remote(ble, event_queue);
	remote.start();

}
