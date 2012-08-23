#include <WConstants.h>
#include <WProgram.h>
#include <Wire.h>

#include <stdlib.h>
#include <avr/pgmspace.h>

#include "ADS80422.h"

// Pins
uint8_t ADS80422::wdPin = 4; 	// Wind direction pin
uint8_t ADS80422::wsPin = 6; 	// Wind speed pin
uint8_t ADS80422::rfPin = 7; 	// Rainfall pin

// Output data
float ADS80422::wind_speed = 0;		// Wind speed in M/s
int ADS80422::wind_direction = 0; 	// Wind direction
float ADS80422::rainfall = 0; 		// Rain fall in MM/s

// Internal constants
const float ADS80422::ANEMO_VALUE = 2.387; 			// 1 click = 2.387km/h
const float ADS80422::RAIN_VALUE = 0.2794; 			// 1 click = 0.2794mm

// Internal variables
float ADS80422::anemo_click_value = (ANEMO_VALUE * 1000) / 3600; 	// M/s
float ADS80422::rain_click_value = RAIN_VALUE;    				// mm TODO: fix this
volatile int ADS80422::anemo_clicks = 0; 									// Number of anemo meter clicks
volatile int ADS80422::rain_clicks = 0; 									// Number of rainfall clicks

ISR(PCINT2_vect)
{
	if (PIND & B01000000)
	{
		ADS80422::anemo_clicks += 1;
	}
	else if (PIND & B10000000)
	{
		ADS80422::rain_clicks += 1;
	}

}

ADS80422::ADS80422()
{
}

void ADS80422::calculate_wind_speed()
{
	wind_speed = anemo_clicks * anemo_click_value;
	anemo_clicks = 0;
}


void ADS80422::calculate_wind_direction()
{
	int wd_pwm_value = analogRead(wdPin);

	// Error correction
	switch (wd_pwm_value) {
		case 785:
			wd_pwm_value = 786;
			break;
		case 703:
			wd_pwm_value = 786;
			break;
		case 704:
			wd_pwm_value = 786;
			break;
		case 887:
			wd_pwm_value = 888;
			break;
		case 889:
			wd_pwm_value = 888;
			break;
		case 828:
			wd_pwm_value = 945;
			break;
		case 829:
			wd_pwm_value = 945;
			break;
		case 600:
			wd_pwm_value = 945;
			break;
		case 946:
			wd_pwm_value = 945;
			break;
		case 243:
			wd_pwm_value = 286;
			break;
		case 244:
			wd_pwm_value = 286;
			break;
		case 124:
			wd_pwm_value = 286;
			break;
		case 125:
			wd_pwm_value = 286;
			break;
		case 182:
			wd_pwm_value = 183;
			break;
		case 64:
			wd_pwm_value = 91;
			break;
		case 92:
			wd_pwm_value = 91;
			break;
		case 82:
			wd_pwm_value = 91;
			break;
		case 460:
			wd_pwm_value = 461;
			break;
		case 405:
			wd_pwm_value = 786;
			break;
		case 599:
			wd_pwm_value = 945;
			break;
		case 630:
			wd_pwm_value = 631;
			break;
	}

	switch (wd_pwm_value) {
		case 786: 		// N
			wind_direction = 1;
			break;
		case 461: 		// NE
			wind_direction = 2;
			break;
		case 91: 		// E
			wind_direction = 3;
			break;
		case 183: 		// SE
			wind_direction = 4;
			break;
		case 286: 		// S
			wind_direction = 5;
			break;
		case 631: 		// SW
			wind_direction = 6;
			break;
		case 945: 		// W
			wind_direction = 7;
			break;
		case 888: 		// NW
			wind_direction = 8;
			break;
	}
}

void ADS80422::calculate_rainfall()
{
	rainfall = rain_clicks * rain_click_value;
	rain_clicks = 0;
}

void ADS80422::calculate()
{
	calculate_wind_speed();
	calculate_wind_direction();
	calculate_rainfall();
}

void ADS80422::setup_interrupts()
{
	// Enable pin change interrupts on pins 6 & 7
	PCICR |= (1 << PCIE2);
	PCMSK2 |= (1 << PCINT22);
	PCMSK2 |= (1 << PCINT23);
}

void ADS80422::begin(uint8_t ws, uint8_t wd, uint8_t rf)
{
	wsPin = ws;
	wdPin = wd;
	rfPin = rf;

	setup_interrupts();
	calculate();
}
