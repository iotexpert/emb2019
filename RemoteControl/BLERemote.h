#pragma once

#include "mbed.h"

#include "ble/BLE.h"
#include "ble/gap/Gap.h"
#include "ble/GattClient.h"
#include "ble/DiscoveredService.h"
#include "ble/DiscoveredCharacteristic.h"
#include "ble/CharacteristicDescriptorDiscovery.h"


extern void bleThread();

class BLERemote :   private mbed::NonCopyable<BLERemote>,
	public ble::Gap::EventHandler {

typedef BLERemote Self;

public:
BLERemote(BLE &ble, events::EventQueue &event_queue) :
	_ble(ble),
	_connection_handle(),
	_event_queue(event_queue),
	connectedStatus(false),
	waterLeft(0),
	waterRight(0),
	_adv_data_builder(_adv_buffer) {
}
void start();

private:

template<typename ContextType>
FunctionPointerWithContext<ContextType> as_cb(
	void (Self::*member)(ContextType context)
	) {
	return makeFunctionPointer(this, member);
}

void on_init_complete(BLE::InitializationCompleteCallbackContext *params);
void start_advertising();
void onConnectionComplete(const ble::ConnectionCompleteEvent &event);
void characteristic_discovery(const DiscoveredCharacteristic *characteristicP);
void discovery_termination(Gap::Handle_t connectionHandle);

void when_descriptor_discovered(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t* event);
void when_descriptor_discovery_ends(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *event);
void when_waterright_cccd_written(const GattWriteCallbackParams* event);
void when_waterleft_cccd_written(const GattWriteCallbackParams* event);

// Game Play
void processSwipeQueue();
void when_characteristic_read(const GattReadCallbackParams *read_event);
void when_characteristic_changed(const GattHVXCallbackParams* event);

void onDisconnectionComplete(const ble::DisconnectionCompleteEvent&);

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

uint8_t waterLeft;
uint8_t waterRight;
uint8_t _adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
ble::AdvertisingDataBuilder _adv_data_builder;

static const UUID GAME_SERVICE_UUID;
static const UUID WATER_LEFT_CHARACTERISTIC_UUID;
static const UUID WATER_RIGHT_CHARACTERISTIC_UUID;
static const UUID PUMP_LEFT_CHARACTERISTIC_UUID;
static const UUID PUMP_RIGHT_CHARACTERISTIC_UUID;
static const char DEVICE_NAME[];

};
