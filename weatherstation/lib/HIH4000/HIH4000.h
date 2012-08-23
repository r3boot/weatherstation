#ifndef HIH4000_h
#define HIH4000_h

class HIH4000
{
	private:
		static const float HIH4000_RES;
		static const float PWM_TICK;
		static const float MAX_VOLTAGE;
		static const float ZERO_VOLTAGE;
		static const float MEASURED_VOLTAGE;
		static int hu_pwm_value;
		static float max_voltage;
	public:
		HIH4000();
		static uint8_t huPin;
		static float humidity;
		void calculate(float temperature);
		void begin(uint8_t hu);
};

#endif
