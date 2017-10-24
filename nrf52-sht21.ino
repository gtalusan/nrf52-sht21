#include <BLEPeripheral.h>
#include <SHT21.h>

#include <nrf_nvic.h>

BLEPeripheral ble;

BLEService tempService("babe");
BLEFloatCharacteristic tempCharacteristic("2a6e", BLERead | BLENotify);
BLEDescriptor tempDescriptor("2901", "Temperature");

BLEFloatCharacteristic humidityCharacteristic("2a6f", BLERead | BLENotify);
BLEDescriptor humidityDescriptor("2901", "Humidity");

SHT21 sht21;

static unsigned long then = millis();
static int _connected = 0;

void setup(void)
{
	ble.setDeviceName("BLE/SHT21");
	ble.setAdvertisedServiceUuid(tempService.uuid());
	ble.addAttribute(tempService);
	ble.addAttribute(tempCharacteristic);
	ble.addAttribute(tempDescriptor);
	ble.addAttribute(humidityCharacteristic);
	ble.addAttribute(humidityDescriptor);
	ble.setEventHandler(BLEConnected, [=] (BLECentral &central) {
		_connected++;
	});
	ble.setEventHandler(BLEDisconnected, [=] (BLECentral &central) {
		_connected--;
		sd_nvic_SystemReset();
	});
	ble.begin();
}

void loop(void)
{
	ble.poll();
	unsigned long now = millis();
	if (_connected > 0) {
		if (now - then >= 25 * 1000) {
			sht21.begin();
			tempCharacteristic.setValue(sht21.getTemperature());
			humidityCharacteristic.setValue(sht21.getHumidity());
			sht21.end();
			then = now;
		}
	}
	else {		
		sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
		sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
		sd_app_evt_wait();
	}
}
