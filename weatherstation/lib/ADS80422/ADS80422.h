#ifndef ADS80422_h
#define ADS80422_h

class ADS80422
{
	private:
		static const float ANEMO_VALUE;
		static const float RAIN_VALUE;
		static float anemo_click_value;
		static float rain_click_value;
		void setup_interrupts();
	public:
		ADS80422();
		static uint8_t wdPin;
		static uint8_t wsPin;
		static uint8_t rfPin;
		static float wind_speed;
		static int wind_direction;
		static float rainfall;
		static volatile int anemo_clicks;
		static volatile int rain_clicks;
		void calculate_wind_speed();
		void calculate_wind_direction();
		void calculate_rainfall();
		void calculate();
		void begin(uint8_t, uint8_t, uint8_t);
};

#endif
