#include <WConstants.h>
#include <WProgram.h>

#include <stdlib.h>
#include <avr/pgmspace.h>

#include "HIH4000.h"

// Pins
uint8_t HIH4000::huPin = 8; 	// Light sensor pin

// Output data
float HIH4000::humidity = 0; 	// Wind direction

// Internal constants
const float HIH4000::HIH4000_RES = 10.0; 				// Resistor value in front of the photocell
const float HIH4000::PWM_TICK = 0.0048828125; 		// Each pwm level corresponds to PWM_TICK voltage change
const float HIH4000::ZERO_VOLTAGE = 0.829919; 		// Voltage at 0C
const float HIH4000::MEASURED_VOLTAGE = 5.11; 			// Measured voltage on 5v bus

// Internal variables
int HIH4000::hu_pwm_value = 0; 							// PWM value for humidity pin
float HIH4000::max_voltage = 0; 						// Max voltage for a given temperature

HIH4000::HIH4000()
{
}

void HIH4000::calculate(float temperature)
{
	hu_pwm_value = analogRead(huPin);
	max_voltage = (3.135-(0.003062 * temperature)); // TODO: hard-define magic numbers
	humidity = (((((float)hu_pwm_value / 1023.0 ) * MEASURED_VOLTAGE) - ZERO_VOLTAGE) / max_voltage) * 100;
}

void HIH4000::begin(uint8_t hu)
{
	huPin = hu;

	pinMode(huPin, INPUT);
}
