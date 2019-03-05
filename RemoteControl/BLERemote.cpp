#include "mbed.h"
#include "global.h"
#include "DisplayThread.h"
#include "CapSenseThread.h"
#include "BLERemote.h"
#include <events/mbed_events.h>

#include "ble/BLE.h"
#include "ble/gap/Gap.h"
#include "ble/GattClient.h"
#include "ble/DiscoveredService.h"
#include "ble/DiscoveredCharacteristic.h"
#include "ble/CharacteristicDescriptorDiscovery.h"
#include "pretty_printer.h"

//#define dbg_printf(...)
#define dbg_printf printf

//EventQueue event_queue(/* event count */ 10 * EVENTS_EVENT_SIZE);

const UUID BLERemote::GAME_SERVICE_UUID("5b19bae4-e452-a996-f14a-84c8094dc021");
const UUID BLERemote::WATER_LEFT_CHARACTERISTIC_UUID("6397ca92-5f02-1eb9-3d4a-316b4300501e");
const UUID BLERemote::WATER_RIGHT_CHARACTERISTIC_UUID("02c095ac-d980-f682-df4f-8d3db03e9e1b");
const UUID BLERemote::PUMP_LEFT_CHARACTERISTIC_UUID("66bb5c46-c5c9-7495-0d4e-206effd357ad");
const UUID BLERemote::PUMP_RIGHT_CHARACTERISTIC_UUID("7cf6e97f-5ae4-6694-a14e-07086ebf9b76");
const char BLERemote::DEVICE_NAME[] = "Remote";


void BLERemote::start() {
	dbg_printf("blethread: Started BLE\n");
	sendDisplayMessage(BLE_SCREEN,BLE_START);
	_ble.onEventsToProcess(as_cb(&Self::schedule_ble_events));
	_ble.gap().setEventHandler(this);
	_ble.init(this, &BLERemote::on_init_complete);
	_event_queue.call_every(20, this,&BLERemote::processSwipeQueue);
	_event_queue.dispatch_forever();
}

/** Callback triggered when the ble initialization process has finished */
void BLERemote::on_init_complete(BLE::InitializationCompleteCallbackContext *params) {
	print_mac_address();
	start_advertising();
}

void BLERemote::start_advertising() {

	dbg_printf("blethread: Started Advertising\n");
	ble::AdvertisingParameters adv_parameters(ble::advertising_type_t::CONNECTABLE_UNDIRECTED);
	_adv_data_builder.setFlags();

	const uint8_t mfgDataBytes[] = {0x31, 0x01, 'R', 'e', 'm', 'o', 't', 'e'};
	Span<const uint8_t> mfgData(mfgDataBytes,8);

	_adv_data_builder.setManufacturerSpecificData (mfgData);
	_adv_data_builder.setName(DEVICE_NAME);
	_ble.gap().setAdvertisingParameters(ble::LEGACY_ADVERTISING_HANDLE,adv_parameters);

	_ble.gap().setAdvertisingPayload(ble::LEGACY_ADVERTISING_HANDLE,
																	 _adv_data_builder.getAdvertisingData());
	_ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE,
															ble::adv_duration_t(10),
															0);

	sendDisplayMessage(BLE_SCREEN,BLE_ADVERTISE);

}

void BLERemote::onConnectionComplete(const ble::ConnectionCompleteEvent &event)
{
	dbg_printf("blethread: onConnectionComplete\n");
	sendDisplayMessage(BLE_SCREEN,BLE_CONNECT);
	_connection_handle = event.getConnectionHandle();

	dbg_printf("blethread: onConnectionComplete\n");

	_ble.gattClient().onServiceDiscoveryTermination(as_cb(&Self::discovery_termination));
	_ble.gattClient().launchServiceDiscovery(
		event.getConnectionHandle(),
		0,
		as_cb(&Self::characteristic_discovery),
		GAME_SERVICE_UUID);
}

void BLERemote::characteristic_discovery(const DiscoveredCharacteristic *characteristicP) {
	if(characteristicP->getUUID() == WATER_LEFT_CHARACTERISTIC_UUID)
	{
		dbg_printf("bleThread: Found water left %d\n",characteristicP->getValueHandle());
		_waterLevelLeft = *characteristicP;
	}

	if(characteristicP->getUUID() == WATER_RIGHT_CHARACTERISTIC_UUID)
	{
		dbg_printf("bleThread:Found water right %d\n",characteristicP->getValueHandle());
		_waterLevelRight = *characteristicP;
	}

	if(characteristicP->getUUID() == PUMP_LEFT_CHARACTERISTIC_UUID)
	{
		dbg_printf("bleThread:Found pump left %d\n",characteristicP->getValueHandle());
		_pumpLeft = *characteristicP;
	}

	if(characteristicP->getUUID() == PUMP_RIGHT_CHARACTERISTIC_UUID)
	{
		dbg_printf("bleThread:Found pump right %d\n",characteristicP->getValueHandle());
		_pumpRight = *characteristicP;
	}
}


void BLERemote::discovery_termination(Gap::Handle_t connectionHandle) {
	dbg_printf("terminated SD for handle %u\r\n", connectionHandle);

	ble_error_t error = _waterLevelLeft.discoverDescriptors(
		as_cb(&Self::when_descriptor_discovered),
		as_cb(&Self::when_descriptor_discovery_ends));
	(void)error;
}

void BLERemote::when_descriptor_discovered(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t* event)
{
	if(event->characteristic.getDeclHandle() == _waterLevelLeft.getDeclHandle() && event->descriptor.getUUID().getShortUUID() == BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG)
	{
		_waterLeftCCCD = event->descriptor.getAttributeHandle();
		dbg_printf("Found waterleft CCCD %d\n",_waterLeftCCCD);
	}

	if(event->characteristic.getDeclHandle() == _waterLevelRight.getDeclHandle() && event->descriptor.getUUID().getShortUUID() == BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG)
	{
		_waterRightCCCD = event->descriptor.getAttributeHandle();
		dbg_printf("Found waterright CCCD %d\n",_waterRightCCCD);

	}
}

void BLERemote::when_descriptor_discovery_ends(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *event) {

	if(event->characteristic == _waterLevelLeft)
	{
		dbg_printf("blethread: found water level left cccd\n");
		ble_error_t error = _waterLevelRight.discoverDescriptors(
			as_cb(&Self::when_descriptor_discovered),
			as_cb(&Self::when_descriptor_discovery_ends));
		(void)error;
	}

	if(event->characteristic == _waterLevelRight)
	{
		dbg_printf("blethread: found water level right cccd\n");
		sendDisplayMessage(GAME_SCREEN,INIT_BLE);
		gameMode = MODE_GAME;

		_ble.gattClient().onDataWritten().add(as_cb(&Self::when_waterright_cccd_written));
		uint16_t cccd_value = 0x01;
		dbg_printf("blethread: writing right CCCD %d\n",_waterRightCCCD);
		_ble.gattClient().write(
			GattClient::GATT_OP_WRITE_REQ,
			_connection_handle,
			_waterRightCCCD,
			sizeof(cccd_value),
			reinterpret_cast<uint8_t*>(&cccd_value)
			);
	}

}

void BLERemote::when_waterright_cccd_written(const GattWriteCallbackParams* event)
{
	dbg_printf("blethread: Right CCCD  written... \n");

	_ble.gattClient().onDataWritten().detach(as_cb(&Self::when_waterright_cccd_written));
	_ble.gattClient().onDataWritten().add(as_cb(&Self::when_waterleft_cccd_written));

	uint16_t cccd_value = 0x01;

	_ble.gattClient().write(
		GattClient::GATT_OP_WRITE_REQ,
		_connection_handle,
		_waterLeftCCCD,
		sizeof(cccd_value),
		reinterpret_cast<uint8_t*>(&cccd_value));
}

void BLERemote::when_waterleft_cccd_written(const GattWriteCallbackParams* event)
{
	dbg_printf("blethread: Left CCCD  written... \n");
	_ble.gattClient().onDataWritten().detach(as_cb(&Self::when_waterleft_cccd_written));
	_ble.gattClient().onDataRead().add(as_cb(&Self::when_characteristic_read));
	_ble.gattClient().onHVX().add(as_cb(&Self::when_characteristic_changed));

}


void BLERemote::processSwipeQueue()
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
				ble_error_t bterr = _pumpLeft.write(1,&val);
				dbg_printf("BTErr = %d\n", bterr);
			}
			else
			{
				dbg_printf("right\n");
				val = (uint8_t)*swipeMsg;
				ble_error_t bterr = _pumpRight.write(1,&val);
				dbg_printf("BTErr = %d\n", bterr);
			}

			swipePool.free(swipeMsg);
		}
	} while (evt.status == osEventMessage);
}


void BLERemote::when_characteristic_read(const GattReadCallbackParams *read_event)
{
	if(read_event->handle == _waterLevelLeft.getValueHandle() )
	{
		_waterLeft = read_event->data[0];
	}
	else if(read_event->handle == _waterLevelRight.getValueHandle() )
	{
		_waterRight = read_event->data[0];
	}
	else
	{
		printf("Characteristic value at %u equal to: ", read_event->handle);
		for (size_t i = 0; i <  read_event->len; ++i) {
			printf("0x%02X ", read_event->data[i]);
		}
		printf(".\r\n");
	}

	sendDisplayMessage(GAME_SCREEN,WATER_VALUE,_waterLeft,_waterRight);

}

void BLERemote::when_characteristic_changed(const GattHVXCallbackParams* event)
{
	if(event->handle == _waterLevelLeft.getValueHandle() )
	{
		_waterLeft = event->data[0];
	}
	else if(event->handle == _waterLevelRight.getValueHandle() )
	{
		_waterRight = event->data[0];
	}
	else
	{
		printf("Change on attribute %u: new value = ", event->handle);
		for (size_t i = 0; i < event->len; ++i) {
			printf("0x%02X ", event->data[i]);
		}
		printf(".\r\n");
	}

	sendDisplayMessage(GAME_SCREEN,WATER_VALUE,_waterLeft,_waterRight);

}

void BLERemote::onDisconnectionComplete(const ble::DisconnectionCompleteEvent&) {
	_ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
	dbg_printf("bleThread: DisconnectionCompleteEvent\n");

	gameMode = MODE_CONNECT_BLE;
	sendDisplayMessage(BLE_SCREEN,BLE_START);
	start_advertising();
}



/** Schedule processing of events from the BLE middleware in the event queue. */
void BLERemote::schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context) {
	_event_queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}
