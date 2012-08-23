#ifndef BMP085_h
#define BMP085_h

class BMP085
{
	private:
		static int sdaPin;
		static int sclPin;
		static const unsigned char oversampling_setting;
		static const unsigned char pressure_waittime[4];
		static int ac1;
		static int ac2;
		static int ac3;
		static unsigned int ac4;
		static unsigned int ac5;
		static unsigned int ac6;
		static int b1;
		static int b2;
		static int mb;
		static int mc;
		static int md;
	public:
		BMP085();
		static float temperature;
		static long pressure;
		void write_register(unsigned char r, unsigned char v);
		char read_register(unsigned char r);
		int read_int_register(unsigned char r);
		unsigned int read_ut();
		long read_up();
		void get_cal_data();
		void calculate();
		void begin(uint8_t, uint8_t);
};

#endif
