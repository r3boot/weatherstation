#include <WConstants.h>
#include <WProgram.h>
#include <Wire.h>

#include <stdlib.h>
#include <avr/pgmspace.h>

#include "BMP085.h"

#define I2C_ADDRESS 0x77

float BMP085::temperature = 0; 		// Temperature
long BMP085::pressure = 0; 			// Pressure

int BMP085::sdaPin = 4;
int BMP085::sclPin = 5;

const unsigned char BMP085::oversampling_setting = 3; //oversamplig for measurement
const unsigned char BMP085::pressure_waittime[4] = { 5, 8, 14, 26 };

int BMP085::ac1 = 0;
int BMP085::ac2 = 0;
int BMP085::ac3 = 0;
unsigned int BMP085::ac4 = 0;
unsigned int BMP085::ac5 = 0;
unsigned int BMP085::ac6 = 0;
int BMP085::b1 = 0;
int BMP085::b2 = 0;
int BMP085::mb = 0;
int BMP085::mc = 0;
int BMP085::md = 0;

BMP085::BMP085()
{
}

void BMP085::write_register(unsigned char r, unsigned char v)
{
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.send(r);
  Wire.send(v);
  Wire.endTransmission();
}

char BMP085::read_register(unsigned char r)
{
  unsigned char v;
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.send(r);  // register to read
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDRESS, 1); // read a byte
  while(!Wire.available()) {
    // waiting
  }
  v = Wire.receive();
  return v;
}

int BMP085::read_int_register(unsigned char r)
{
  unsigned char msb, lsb;
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.send(r);  // register to read
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDRESS, 2); // read a byte
  while(!Wire.available()) {
    // waiting
  }
  msb = Wire.receive();
  while(!Wire.available()) {
    // waiting
  }
  lsb = Wire.receive();
  return (((int)msb<<8) | ((int)lsb));
}

unsigned int BMP085::read_ut() {
	write_register(0xf4,0x2e);
	delay(5); //longer than 4.5 ms
	return read_int_register(0xf6);
}

long BMP085::read_up() {
	write_register(0xf4,0x34+(oversampling_setting<<6));
  delay(pressure_waittime[oversampling_setting]);
  
  unsigned char msb, lsb, xlsb;
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.send(0xf6);  // register to read
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDRESS, 3); // read a byte
  while(!Wire.available()) {
    // waiting
  }
  msb = Wire.receive();
  while(!Wire.available()) {
    // waiting
  }
  lsb |= Wire.receive();
  while(!Wire.available()) {
    // waiting
  }
  xlsb |= Wire.receive();
  return (((long)msb<<16) | ((long)lsb<<8) | ((long)xlsb)) >>(8-oversampling_setting);
}

void BMP085::get_cal_data() {
  ac1 = read_int_register(0xAA);
  ac2 = read_int_register(0xAC);
  ac3 = read_int_register(0xAE);
  ac4 = read_int_register(0xB0);
  ac5 = read_int_register(0xB2);
  ac6 = read_int_register(0xB4);
  b1 = read_int_register(0xB6);
  b2 = read_int_register(0xB8);
  mb = read_int_register(0xBA);
  mc = read_int_register(0xBC);
  md = read_int_register(0xBE);
}

void BMP085::calculate()
{
  int  ut= read_ut();
  long up = read_up();
   long x1, x2, x3, b3, b5, b6, p;
   unsigned long b4, b7;

   //calculate the temperature
   x1 = ((long)ut - ac6) * ac5 >> 15;
   x2 = ((long) mc << 11) / (x1 + md);
   b5 = x1 + x2;
   temperature = (float)((b5 + 8) >> 4) * .1;
   // temperature = ((b5 + 8) >> 4);
   
   //calculate the pressure
   b6 = b5 - 4000;
   x1 = (b2 * (b6 * b6 >> 12)) >> 11; 
   x2 = ac2 * b6 >> 11;
   x3 = x1 + x2;
   b3 = (((int32_t) ac1 * 4 + x3)<<oversampling_setting + 2) >> 2;
   x1 = ac3 * b6 >> 13;
   x2 = (b1 * (b6 * b6 >> 12)) >> 16;
   x3 = ((x1 + x2) + 2) >> 2;
   b4 = (ac4 * (uint32_t) (x3 + 32768)) >> 15;
   b7 = ((uint32_t) up - b3) * (50000 >> oversampling_setting);
   p = b7 < 0x80000000 ? (b7 * 2) / b4 : (b7 / b4) * 2;
   
   x1 = (p >> 8) * (p >> 8);
   x1 = (x1 * 3038) >> 16;
   x2 = (-7357 * p) >> 16;
   pressure = p + ((x1 + x2 + 3791) >> 4);
}

void BMP085::begin(uint8_t sda, uint8_t scl)
{
	sdaPin = sda;
	sclPin = scl;

	get_cal_data();
}
