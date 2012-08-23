#ifndef LDR_h
#define LDR_h

class LDR
{
	private:
		static const float LDR_RES;
		static const float PWM_TICK;
		static const float MAX_VOLTAGE;
	public:
		LDR();
		static uint8_t ldrPin;
		static float light;
		void calculate();
		void begin(uint8_t ldr);
};

#endif
