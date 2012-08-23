// #include <SoftwareSerial.h>
#include <Wire.h> // needed for bmp085
#include <BMP085.h>
#include <ADS80422.h>
#include <LDR.h>
#include <HIH4000.h>
#include <SoftwareSerial.h>
#include <avr/interrupt.h>  
#include <avr/io.h>


// Host id for this unit
uint8_t HOST_ID = 12;

// Pin definitions
uint8_t txPin = 1; // Transmitter input
uint8_t huPin = 3; // HIH4000 Humidity sensor input
uint8_t sdaPin = 4; // BMP085 2W data in/output
uint8_t sclPin = 5; // BMP085 2W clock input

uint8_t wsPin = 6; // ADS80422 Anemometer input
uint8_t wdPin = 0; // ADS80422 Wind direction input
uint8_t rfPin = 7; // ADS80422 Rain gauge input

uint8_t ldrPin = 2; // LDRPhotoresistor output

long baudrate = 1200;

int t_start = 0;
int t_end = 0;

long checksum = 0;

// Protocol definitions
uint8_t MSG_START = 0xFC;
uint8_t MSG_END = 0xCF;
uint8_t MSG_NULL = 0xFD;
uint8_t HOSTID = 0x42;
uint8_t MSG_DELAY = 10; 	// delay in ms between each FSK packet

enum sensors
{
	TEMPERATURE = 0x01,
	PRESSURE = 0x02,
	HUMIDITY = 0x03,
	RAINFALL = 0x04,
	WINDSPEED = 0x05,
	WINDDIRECTION = 0x06,
	LIGHT = 0x07,
};

BMP085 bmp = BMP085();
HIH4000 hih = HIH4000();
ADS80422 wsa = ADS80422();
LDR ldr = LDR();

uint8_t get_long_multiplier(long value) {
	if (value > 255) {
		return (uint8_t)(value / 255);
	} else {
		return 0;
	}
}

uint8_t get_long_difference(long value, uint8_t multiplier) {
	if (value > 0) {
		return (unsigned int)value - ((int)multiplier * 255);
		/*
		if ((int)multiplier > 0) {
			Serial.println("here");
			return (uint8_t)(value - ((int)multiplier * 255));
		} else {
			Serial.println("here2");
			return (uint8_t)value;
		}
		*/
	} else {
		return 0;
	}
}

uint8_t get_float_multiplier(float value) {
	value = (long)(value * 100);
	if (value >= 255) {
		return get_long_multiplier(value);
	} else {
		return 0;
	}
}

uint8_t get_float_difference(float value, uint8_t multiplier) {
	value = (long)(value * 100);
	if (value > 0) {
		return get_long_difference(value, multiplier);
	} else {
		return 0;
	}
}

void transmit_fsk_no_checksum(uint8_t value) {
	Serial.write((uint8_t)value);
	Serial.write((uint8_t)~value);
	delay(MSG_DELAY);
}

void transmit_fsk(uint8_t value) {
	checksum = checksum + value;
	Serial.write((uint8_t)value);
	Serial.write((uint8_t)~value);
	delay(MSG_DELAY);
}

void transmit_float(float value) {
	uint8_t m, d = 0;

	m = get_float_multiplier(value);
	d = get_float_difference(value, m);
	transmit_fsk(m);
	transmit_fsk(d);

	/*
	Serial.println();
	Serial.println((int)m);
	Serial.println((int)d);
	Serial.println();
	*/
}

void transmit_long(long value) {
	uint8_t m, d = 0;

	m = get_long_multiplier(value);
	d = get_long_difference(value, m);
	transmit_fsk(m);
	transmit_fsk(d);
}

void transmit_long_no_checksum(long value) {
	uint8_t m, d = 0;

	m = get_long_multiplier(value);
	d = get_long_difference(value, m);
	transmit_fsk_no_checksum(m);
	transmit_fsk_no_checksum(d);
}

void transmit_values() {

	//Serial.begin(baudrate);
	//transmit_fsk_no_checksum(MSG_NOP);
	//transmit_fsk_no_checksum(MSG_NOP);
	
	transmit_fsk_no_checksum(MSG_START);
	transmit_fsk_no_checksum(MSG_START);
	transmit_fsk(HOST_ID);

	transmit_float(bmp.temperature);
	// transmit_float(1.0);

	transmit_long(bmp.pressure);
	// transmit_long(2);

	transmit_float(hih.humidity);
	// transmit_float(3.0);

	transmit_float(ldr.light);
	// transmit_long(4);

	transmit_float(wsa.wind_speed);
	//transmit_float(5.0);

	transmit_long(wsa.wind_direction);
	// transmit_long(6);

	transmit_float(wsa.rainfall);
	// transmit_float(7.0);

	/*
	Serial.println();
	Serial.print("checksum: ");
	Serial.println(checksum);
	Serial.println();
	*/
	transmit_long_no_checksum(checksum);
	checksum = 0;

	transmit_fsk_no_checksum(MSG_END);
	transmit_fsk_no_checksum(MSG_END);

	// delay(1);
	//Serial.end();

}

void setup()
{
	delay(10);
	pinMode(txPin, OUTPUT);

	Serial.begin(baudrate);
	Serial.print("initializing: ");

	//hih.begin(huPin);
	wsa.begin(wsPin, wdPin, rfPin);
	ldr.begin(ldrPin);
	bmp.begin(sdaPin, sclPin);
	Serial.println("done");
}

void loop()
{

	t_start = millis();

	bmp.calculate();
	// hih.calculate(bmp.temperature);
	wsa.calculate();
	ldr.calculate();

	Serial.println("");
	Serial.println("Sending values");
	transmit_values();
	/*
	// transmit_long(HOSTID, HOST_ID);
	transmit_float(TEMPERATURE, bmp.temperature);
	transmit_long(PRESSURE, bmp.pressure);
	transmit_long(HUMIDITY, hih.humidity);
	transmit_long(LIGHT, ldr.light);
	transmit_float(WINDSPEED, wsa.wind_speed);
	transmit_long(WINDDIRECTION, wsa.wind_direction);
	transmit_float(RAINFALL, wsa.rainfall);
	*/

	/*
	Serial.print(HOST_ID, DEC);
	Serial.print(",");
	Serial.print(bmp.temperature);
	Serial.print(",");
	Serial.print(bmp.pressure);
	Serial.print(",");
	Serial.print(hih.humidity);
	Serial.print(",");
	Serial.print(ldr.light);
	Serial.print(",");
	Serial.print(wsa.wind_speed);
	Serial.print(",");
	Serial.print(wsa.wind_direction);
	Serial.print(",");
	Serial.print(wsa.rainfall);
	Serial.print(",");
	data_csum = HOST_ID + bmp.temperature + bmp.pressure + hih.humidity + ldr.light + wsa.wind_speed + wsa.rainfall + wsa.wind_direction;
	Serial.println(data_csum);
	*/

	t_end = millis();
	delay(1000 - (t_end - t_start) - 1);

}
