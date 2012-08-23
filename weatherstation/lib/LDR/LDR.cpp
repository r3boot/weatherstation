#include <WConstants.h>
#include <WProgram.h>

#include <stdlib.h>
#include <avr/pgmspace.h>

#include "LDR.h"

// Pins
uint8_t LDR::ldrPin = 8; 	// Light sensor pin

// Output data
float LDR::light = 0.0; 	// Light in lux

// Internal constants
const float LDR::LDR_RES = 10.0; 				// Resistor value in front of the photocell
const float LDR::PWM_TICK = 0.0048828125; 		// Each pwm level corresponds to PWM_TICK voltage change
const float LDR::MAX_VOLTAGE = 5.11; 			// Measured voltage on 5v bus

// Internal variables
int ldr_pwm_value = 0; 							// PWM value for LDR pin
float ldr_Vout = 0; 							// LDR pin Vout reading

LDR::LDR()
{
}

void LDR::calculate()
{
	int ldr_pwm_value = analogRead(ldrPin);
	float ldr_Vout = MAX_VOLTAGE - (ldr_pwm_value * PWM_TICK);

	light = 500 / (LDR_RES * ((MAX_VOLTAGE - ldr_Vout) / ldr_Vout));
}

void LDR::begin(uint8_t ldr)
{
	ldrPin = ldr;
	pinMode(ldrPin, INPUT);
	calculate();
}
