// RF22.cpp
//
// Copyright (C) 2011 Mike McCauley
// $Id: RF22.cpp,v 1.12 2011/02/15 01:18:03 mikem Exp $

#include <RF22.h>
#include <SPI.h>

// Interrupt vectors for the 2 Arduino interrupt pins
// Each interrupt can be handled by a different instance of RF22, allowing you to have
// 2 RF22s per Arduino
RF22* RF22::_RF22ForInterrupt[2] = {0, 0};

// These are indexed by the values of ModemConfigChoice
// Canned modem configurations generated with 
// 'http://www.hoperf.com/upfile/RF22B 23B 31B 42B 43B Register Settings_RevB1-v5.xls'
// Stored in flash (program) memory to save SRAM
PROGMEM static const RF22::ModemConfig MODEM_CONFIG_TABLE[] =
{
    { 0x2b, 0x03, 0xf4, 0x20, 0x41, 0x89, 0x00, 0x36, 0x40, 0x0a, 0x1d, 0x80, 0x60, 0x10, 0x62, 0x2c, 0x00, 0x08 }, // Unmodulated carrier
    { 0x2b, 0x03, 0xf4, 0x20, 0x41, 0x89, 0x00, 0x36, 0x40, 0x0a, 0x1d, 0x80, 0x60, 0x10, 0x62, 0x2c, 0x33, 0x08 }, // FSK, PN9 random modulation, 2, 5

    //  1c,   1f,   20,   21,   22,   23,   24,   25,   2c,   2d,   2e,   58,   69,   6e,   6f,   70,   71,   72
    // FSK, No Manchester, Max Rb err <1%, Xtal Tol 20ppm
    { 0x2b, 0x03, 0xf4, 0x20, 0x41, 0x89, 0x00, 0x36, 0x40, 0x0a, 0x1d, 0x80, 0x60, 0x10, 0x62, 0x2c, 0x22, 0x08 }, // 2, 5
    { 0x1b, 0x03, 0x41, 0x60, 0x27, 0x52, 0x00, 0x07, 0x40, 0x0a, 0x1e, 0x80, 0x60, 0x13, 0xa9, 0x2c, 0x22, 0x3a }, // 2.4, 36
    { 0x1d, 0x03, 0xa1, 0x20, 0x4e, 0xa5, 0x00, 0x13, 0x40, 0x0a, 0x1e, 0x80, 0x60, 0x27, 0x52, 0x2c, 0x22, 0x48 }, // 4.8, 45
    { 0x1e, 0x03, 0xd0, 0x00, 0x9d, 0x49, 0x00, 0x45, 0x40, 0x0a, 0x20, 0x80, 0x60, 0x4e, 0xa5, 0x2c, 0x22, 0x48 }, // 9.6, 45
    { 0x2b, 0x03, 0x34, 0x02, 0x75, 0x25, 0x07, 0xff, 0x40, 0x0a, 0x1b, 0x80, 0x60, 0x9d, 0x49, 0x2c, 0x22, 0x0f }, // 19.2, 9.6
    { 0x02, 0x03, 0x68, 0x01, 0x3a, 0x93, 0x04, 0xd5, 0x40, 0x0a, 0x1e, 0x80, 0x60, 0x09, 0xd5, 0x0c, 0x22, 0x1f }, // 38.4, 19.6
    { 0x06, 0x03, 0x45, 0x01, 0xd7, 0xdc, 0x07, 0x6e, 0x40, 0x0a, 0x2d, 0x80, 0x60, 0x0e, 0xbf, 0x0c, 0x22, 0x2e }, // 57.6. 28.8
    { 0x8a, 0x03, 0x60, 0x01, 0x55, 0x55, 0x02, 0xad, 0x40, 0x0a, 0x50, 0x80, 0x60, 0x20, 0x00, 0x0c, 0x22, 0xc8 }, // 125, 125

    // GFSK, No Manchester, Max Rb err <1%, Xtal Tol 20ppm
    // These differ from FSK only in register 71, for the modulation type
    { 0x2b, 0x03, 0xf4, 0x20, 0x41, 0x89, 0x00, 0x36, 0x40, 0x0a, 0x1d, 0x80, 0x60, 0x10, 0x62, 0x2c, 0x23, 0x08 }, // 2, 5
    { 0x1b, 0x03, 0x41, 0x60, 0x27, 0x52, 0x00, 0x07, 0x40, 0x0a, 0x1e, 0x80, 0x60, 0x13, 0xa9, 0x2c, 0x23, 0x3a }, // 2.4, 36
    { 0x1d, 0x03, 0xa1, 0x20, 0x4e, 0xa5, 0x00, 0x13, 0x40, 0x0a, 0x1e, 0x80, 0x60, 0x27, 0x52, 0x2c, 0x23, 0x48 }, // 4.8, 45
    { 0x1e, 0x03, 0xd0, 0x00, 0x9d, 0x49, 0x00, 0x45, 0x40, 0x0a, 0x20, 0x80, 0x60, 0x4e, 0xa5, 0x2c, 0x23, 0x48 }, // 9.6, 45
    { 0x2b, 0x03, 0x34, 0x02, 0x75, 0x25, 0x07, 0xff, 0x40, 0x0a, 0x1b, 0x80, 0x60, 0x9d, 0x49, 0x2c, 0x23, 0x0f }, // 19.2, 9.6
    { 0x02, 0x03, 0x68, 0x01, 0x3a, 0x93, 0x04, 0xd5, 0x40, 0x0a, 0x1e, 0x80, 0x60, 0x09, 0xd5, 0x0c, 0x23, 0x1f }, // 38.4, 19.6
    { 0x06, 0x03, 0x45, 0x01, 0xd7, 0xdc, 0x07, 0x6e, 0x40, 0x0a, 0x2d, 0x80, 0x60, 0x0e, 0xbf, 0x0c, 0x23, 0x2e }, // 57.6. 28.8
    { 0x8a, 0x03, 0x60, 0x01, 0x55, 0x55, 0x02, 0xad, 0x40, 0x0a, 0x50, 0x80, 0x60, 0x20, 0x00, 0x0c, 0x23, 0xc8 }, // 125, 125

    // OOK, No Manchester, Max Rb err <1%, Xtal Tol 20ppm
    { 0x51, 0x03, 0x68, 0x00, 0x3a, 0x93, 0x01, 0x3d, 0x2c, 0x11, 0x28, 0x80, 0x60, 0x09, 0xd5, 0x2c, 0x21, 0x08 }, // 1.2, 75
    { 0xc8, 0x03, 0x39, 0x20, 0x68, 0xdc, 0x00, 0x6b, 0x2a, 0x08, 0x2a, 0x80, 0x60, 0x13, 0xa9, 0x2c, 0x21, 0x08 }, // 2.4, 335
    { 0xc8, 0x03, 0x9c, 0x00, 0xd1, 0xb7, 0x00, 0xd4, 0x29, 0x04, 0x29, 0x80, 0x60, 0x27, 0x52, 0x2c, 0x21, 0x08 }, // 4.8, 335
    { 0xb8, 0x03, 0x9c, 0x00, 0xd1, 0xb7, 0x00, 0xd4, 0x28, 0x82, 0x29, 0x80, 0x60, 0x4e, 0xa5, 0x2c, 0x21, 0x08 }, // 9.6, 335
    { 0xa8, 0x03, 0x9c, 0x00, 0xd1, 0xb7, 0x00, 0xd4, 0x28, 0x41, 0x29, 0x80, 0x60, 0x9d, 0x49, 0x2c, 0x21, 0x08 }, // 19.2, 335
    { 0x98, 0x03, 0x9c, 0x00, 0xd1, 0xb7, 0x00, 0xd4, 0x28, 0x20, 0x29, 0x80, 0x60, 0x09, 0xd5, 0x0c, 0x21, 0x08 }, // 38.4, 335
    { 0x98, 0x03, 0x96, 0x00, 0xda, 0x74, 0x00, 0xdc, 0x28, 0x1f, 0x29, 0x80, 0x60, 0x0a, 0x3d, 0x0c, 0x21, 0x08 }, // 40, 335

};

RF22::RF22(uint8_t slaveSelectPin, uint8_t interrupt)
{
    _slaveSelectPin = slaveSelectPin;
    _interrupt = interrupt;
    _idleMode = RF22_XTON; // Default idle state is READY mode
    _mode = RF22_MODE_IDLE; // We start up in idle mode
    _rxGood = 0;
    _rxBad = 0;
    _txGood = 0;
}

boolean RF22::init()
{
    // Wait for RF22 POR (up to 16msec)
    delay(16);

    // Initialise the slave select pin
    pinMode(_slaveSelectPin, OUTPUT);
    digitalWrite(_slaveSelectPin, HIGH);
  
    // start the SPI library:
    // Note the RF22 wants mode 0, MSB first and default to 1 Mbps
    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV16);  // (16 Mhz / 16) = 1 MHz

    // Software reset the device
    reset();

    // Get the device type and check it
    // This also tests whether we are really connected to a device
    _deviceType = spiRead(RF22_REG_00_DEVICE_TYPE);
    if (   _deviceType != RF22_DEVICE_TYPE_RX_TRX
        && _deviceType != RF22_DEVICE_TYPE_TX)
	return false;

	// Set up interrupt handler
    if (_interrupt == 0)
    {
	_RF22ForInterrupt[0] = this;
	attachInterrupt(0, RF22::isr0, LOW);  
    }
    else if (_interrupt == 1)
    {
	_RF22ForInterrupt[1] = this;
	attachInterrupt(1, RF22::isr1, LOW);  
    }
    else
	return false;
 
    clearTxBuf();
    clearRxBuf();
  
    // Most of these are the POR default
    spiWrite(RF22_REG_7D_TX_FIFO_CONTROL2, RF22_TXFFAEM_THRESHOLD);
    spiWrite(RF22_REG_7E_RX_FIFO_CONTROL,  RF22_RXFFAFULL_THRESHOLD);
    spiWrite(RF22_REG_30_DATA_ACCESS_CONTROL, RF22_ENPACRX | RF22_ENPACTX | RF22_ENCRC | RF22_CRC_CRC_16_IBM);
    // Configure the message headers
    // Here we set up the standard packet format for use by the RF22 library
    // 8 nibbles preamble
    // 2 SYNC words 2d, d4
    // Header length 4 (to, from, id, flags)
    // 1 octet of data length (0 to 255)
    // 0 to 255 octets data
    // 2 CRC octets as CRC16(IBM), computed on the header, length and data
    // On reception the to address is check for validity against RF22_REG_3F_CHECK_HEADER3
    // or the broadcast address of 0xff
    // If no changes are made after this, the transmitted
    // to address will be 0xff, the from address will be 0xff
    // and all such messages will be accepted. This permits the out-of the box
    // RF22 config to act as an unaddresed, unreliable datagram service
    spiWrite(RF22_REG_32_HEADER_CONTROL1, RF22_BCEN_HEADER3 | RF22_HDCH_HEADER3);
    spiWrite(RF22_REG_33_HEADER_CONTROL2, RF22_HDLEN_4 | RF22_SYNCLEN_2);
    setPreambleLength(8);
    uint8_t syncwords[] = { 0x2d, 0xd4 };
    setSyncWords(syncwords, sizeof(syncwords));
    setPromiscuous(false); 
    // Check the TO header against RF22_DEFAULT_NODE_ADDRESS
    spiWrite(RF22_REG_3F_CHECK_HEADER3, RF22_DEFAULT_NODE_ADDRESS);
    // Set the default transmit header values
    setHeaderTo(RF22_DEFAULT_NODE_ADDRESS);
    setHeaderFrom(RF22_DEFAULT_NODE_ADDRESS);
    setHeaderId(0);
    setHeaderFlags(0);

    // Ensure the antenna can be switched automatically according to transmit and receive
    // This assumes GPIO0(out) is connected to TX_ANT(in) to enable tx antenna during transmit
    // This assumes GPIO1(out) is connected to RX_ANT(in) to enable rx antenna during receive
    spiWrite (RF22_REG_0B_GPIO_CONFIGURATION0, 0x12) ; // TX state
    spiWrite (RF22_REG_0C_GPIO_CONFIGURATION1, 0x15) ; // RX state

    // Enable interrupts
    spiWrite(RF22_REG_05_INTERRUPT_ENABLE1, RF22_ENTXFFAEM | RF22_ENRXFFAFULL | RF22_ENPKSENT | RF22_ENPKVALID | RF22_ENCRCERROR | RF22_ENFFERR);
    spiWrite(RF22_REG_06_INTERRUPT_ENABLE2, RF22_ENPREAVAL);

    // Set some defaults. An innocuous ISM frequency
    setFrequency(434.0);
//    setFrequency(900.0);
    // Some slow, reliable default speed and modulation
    setModemConfig(FSK_Rb2_4Fd36);
//    setModemConfig(FSK_Rb125Fd125);
    // Minimum power
    setTxPower(RF22_TXPOW_8DBM);
//    setTxPower(RF22_TXPOW_17DBM);

    return true;
}

// C++ level interrupt handler for this instance
void RF22::handleInterrupt()
{
    uint8_t _lastInterruptFlags[2];
    // Read the interrupt flags which clears the interrupt
    spiBurstRead(RF22_REG_03_INTERRUPT_STATUS1, _lastInterruptFlags, 2);

#if 0
    Serial.print("interrupt ");
    Serial.print(_lastInterruptFlags[0], HEX);
    Serial.print(" ");
    Serial.println(_lastInterruptFlags[1], HEX);
    if (_lastInterruptFlags[0] == 0 && _lastInterruptFlags[1] == 0)
	Serial.println("FUNNY: no interrupt!");
#endif

    if (_lastInterruptFlags[0] & RF22_IFFERROR)
    {
//	Serial.println("IFFERROR");    
	// REVISIT: need to reset FIFOs and maybe restart any transmission
	resetFifos();
	clearRxBuf();
	_mode = RF22_MODE_IDLE;
	// REVISIT?? for retransmission?
    }
    // Caution, any delay here may cause a FF underflow or overflow
    if (_lastInterruptFlags[0] & RF22_ITXFFAEM)
    {
	// See if more data has to be loaded into the Tx FIFO 
	sendNextFragment();
//	Serial.println("TXFFAEM");  
    }
    if (_lastInterruptFlags[0] & RF22_IRXFFAFULL)
    {
	// Caution, any delay here may cause a FF overflow
	// Read some data from the Rx FIFO
	readNextFragment();
//	Serial.println("IRXFFAFULL"); 
    }
    if (_lastInterruptFlags[0] & RF22_IEXT)
    {
	// This is not enabled by the base code, but users may want to enable it
	handleExternalInterrupt();
//	Serial.println("IEXT"); 
    }
    if (_lastInterruptFlags[1] & RF22_IWUT)
    {
	// This is not enabled by the base code, but users may want to enable it
	handleWakeupTimerInterrupt();
//	Serial.println("IWUT"); 
    }
    if (_lastInterruptFlags[0] & RF22_IPKSENT)
    {
//	Serial.println("PKSENT");   
	_txGood++; 
	// Transmission does not automatically clear the tx buffer.
	// Could retransmit if we wanted
	_txPacketSent = true;
	// RF22 transitions automatically to Idle
	_mode = RF22_MODE_IDLE;
    }
    if (_lastInterruptFlags[0] & RF22_IPKVALID)
    {
//	Serial.println("IPKVALID");    
	uint8_t len = spiRead(RF22_REG_4B_RECEIVED_PACKET_LENGTH);
	// May have already read one or more fragments
	// Get any remaining unread octets, based on the expected length
	len -= _bufLen;
	spiBurstRead(RF22_REG_7F_FIFO_ACCESS, _buf + _bufLen, len);
	_rxGood++;
	_bufLen += len;
	_mode = RF22_MODE_IDLE;
	_rxBufValid = true;
    }
    if (_lastInterruptFlags[0] & RF22_ICRCERROR)
    {
//	Serial.println("ICRCERR");  
	_rxBad++;
	clearRxBuf();
	resetRxFifo();
	_mode = RF22_MODE_IDLE;
	setModeRx(); // Keep trying
    }
    if (_lastInterruptFlags[1] & RF22_ENPREAVAL)
    {
//	Serial.println("ENPREAVAL");  
	_lastRssi = spiRead(RF22_REG_26_RSSI);
	clearRxBuf();
    }
}

// These are low level functions that call the interrupt handler for the correct
// instance of RF22.
// 2 interrupts allows us to have 2 different devices
void RF22::isr0()
{
    if (_RF22ForInterrupt[0])
	_RF22ForInterrupt[0]->handleInterrupt();
}
void RF22::isr1()
{
    if (_RF22ForInterrupt[1])
	_RF22ForInterrupt[1]->handleInterrupt();
}

void RF22::reset()
{
    spiWrite(RF22_REG_07_OPERATING_MODE1, RF22_SWRES);
    // Wait for it to settle
    delay(1); // SWReset time is nominally 100usec
}

uint8_t RF22::spiRead(uint8_t reg)
{
    digitalWrite(_slaveSelectPin, LOW);
    SPI.transfer(reg & ~RF22_SPI_WRITE_MASK); // Send the address with the write mask off
    uint8_t val = SPI.transfer(0); // The written value is ignored, reg value is read
    digitalWrite(_slaveSelectPin, HIGH);
    return val;
}

void RF22::spiWrite(uint8_t reg, uint8_t val)
{
    digitalWrite(_slaveSelectPin, LOW);
    SPI.transfer(reg | RF22_SPI_WRITE_MASK); // Send the address with the write mask on
    SPI.transfer(val); // New value follows
    digitalWrite(_slaveSelectPin, HIGH);
}

void RF22::spiBurstRead(uint8_t reg, uint8_t* dest, uint8_t len)
{
    digitalWrite(_slaveSelectPin, LOW);
    SPI.transfer(reg & ~RF22_SPI_WRITE_MASK); // Send the start address with the write mask off
    while (len--)
	*dest++ = SPI.transfer(0);
    digitalWrite(_slaveSelectPin, HIGH);
}

void RF22::spiBurstWrite(uint8_t reg, uint8_t* src, uint8_t len)
{
    digitalWrite(_slaveSelectPin, LOW);
    SPI.transfer(reg | RF22_SPI_WRITE_MASK); // Send the start address with the write mask on
    while (len--)
	SPI.transfer(*src++);
    digitalWrite(_slaveSelectPin, HIGH);
}

uint8_t RF22::statusRead()
{
    return spiRead(RF22_REG_02_DEVICE_STATUS);
}

uint8_t RF22::adcRead(uint8_t adcsel,
                      uint8_t adcref ,
                      uint8_t adcgain, 
                      uint8_t adcoffs)
{
    uint8_t configuration = adcsel | adcref | (adcgain & RF22_ADCGAIN);
    spiWrite(RF22_REG_0F_ADC_CONFIGURATION, configuration | RF22_ADCSTART);
    spiWrite(RF22_REG_10_ADC_SENSOR_AMP_OFFSET, adcoffs);

    // Conversion time is nominally 305usec
    // Wait for the DONE bit
    while (!(spiRead(RF22_REG_0F_ADC_CONFIGURATION) & RF22_ADCDONE))
	;
    // Return the value  
    return spiRead(RF22_REG_11_ADC_VALUE);
}

uint8_t RF22::temperatureRead(uint8_t tsrange, uint8_t tvoffs)
{
    spiWrite(RF22_REG_12_TEMPERATURE_SENSOR_CALIBRATION, tsrange | RF22_ENTSOFFS);
    spiWrite(RF22_REG_13_TEMPERATURE_VALUE_OFFSET, tvoffs);
    return adcRead(RF22_ADCSEL_INTERNAL_TEMPERATURE_SENSOR | RF22_ADCREF_BANDGAP_VOLTAGE); 
}

uint16_t RF22::wutRead()
{
    uint8_t buf[2];
    spiBurstRead(RF22_REG_17_WAKEUP_TIMER_VALUE1, buf, 2);
    return ((uint16_t)buf[0] << 8) | buf[1]; // Dont rely on byte order
}

// RFM-22 doc appears to be wrong: WUT for wtm = 10000, r, = 0, d = 0 is about 1 sec
void RF22::setWutPeriod(uint16_t wtm, uint8_t wtr, uint8_t wtd)
{
    uint8_t period[3];

    period[0] = ((wtr & 0xf) << 2) | (wtd & 0x3);
    period[1] = wtm >> 8;
    period[2] = wtm & 0xff;
    spiBurstWrite(RF22_REG_14_WAKEUP_TIMER_PERIOD1, period, sizeof(period));
}

// Returns true if centre + (fhch * fhs) is within limits
// Caution, different versions of the RF22 suport different max freq
// so YMMV
boolean RF22::setFrequency(float centre)
{
    uint8_t fbsel = RF22_SBSEL;
    if (centre < 240.0 || centre > 960.0) // 930.0 for early silicon
	return false;
    if (centre >= 480.0)
    {
	centre /= 2;
	fbsel |= RF22_HBSEL;
    }
    centre /= 10.0;
    float integerPart = floor(centre);
    float fractionalPart = centre - integerPart;

    uint8_t fb = (uint8_t)integerPart - 24; // Range 0 to 23
    fbsel |= fb;
    uint16_t fc = fractionalPart * 64000;
    spiWrite(RF22_REG_73_FREQUENCY_OFFSET1, 0);  // REVISIT
    spiWrite(RF22_REG_74_FREQUENCY_OFFSET2, 0);
    spiWrite(RF22_REG_75_FREQUENCY_BAND_SELECT, fbsel);
    spiWrite(RF22_REG_76_NOMINAL_CARRIER_FREQUENCY1, fc >> 8);
    spiWrite(RF22_REG_77_NOMINAL_CARRIER_FREQUENCY0, fc & 0xff);
    return !(statusRead() & RF22_FREQERR);
}

// Step size in 10kHz increments
// Returns true if centre + (fhch * fhs) is within limits
boolean RF22::setFHStepSize(uint8_t fhs)
{
    spiWrite(RF22_REG_7A_FREQUENCY_HOPPING_STEP_SIZE, fhs);
    return !(statusRead() & RF22_FREQERR);
}

// Adds fhch * fhs to centre frequency
// Returns true if centre + (fhch * fhs) is within limits
boolean RF22::setFHChannel(uint8_t fhch)
{
    spiWrite(RF22_REG_79_FREQUENCY_HOPPING_CHANNEL_SELECT, fhch);
    return !(statusRead() & RF22_FREQERR);
}

uint8_t RF22::rssiRead()
{
    return spiRead(RF22_REG_26_RSSI);
}

uint8_t RF22::ezmacStatusRead()
{
    return spiRead(RF22_REG_31_EZMAC_STATUS);
}

void RF22::setMode(uint8_t mode)
{
    spiWrite(RF22_REG_07_OPERATING_MODE1, mode);
}

void RF22::setModeIdle()
{
    if (_mode != RF22_MODE_IDLE)
    {
	setMode(_idleMode);
	_mode = RF22_MODE_IDLE;
    }
}

void RF22::setModeRx()
{
    if (_mode != RF22_MODE_RX)
    {
	setMode(_idleMode | RF22_RXON);
	_mode = RF22_MODE_RX;
    }
}

void RF22::setModeTx()
{
    if (_mode != RF22_MODE_TX)
    {
	setMode(_idleMode | RF22_TXON);
	_mode = RF22_MODE_TX;
    }
}

void RF22::setTxPower(uint8_t power)
{
    spiWrite(RF22_REG_6D_TX_POWER, power);
}

// Sets registers from a canned modem configuration structure
void RF22::setModemRegisters(ModemConfig* config)
{
    spiWrite(RF22_REG_1C_IF_FILTER_BANDWIDTH,                    config->reg_1c);
    spiWrite(RF22_REG_1F_CLOCK_RECOVERY_GEARSHIFT_OVERRIDE,      config->reg_1f);
    spiBurstWrite(RF22_REG_20_CLOCK_RECOVERY_OVERSAMPLING_RATE, &config->reg_20, 6);
    spiBurstWrite(RF22_REG_2C_OOK_COUNTER_VALUE_1,              &config->reg_2c, 3);
    spiWrite(RF22_REG_58_CHARGE_PUMP_CURRENT_TRIMMING,           config->reg_58);
    spiWrite(RF22_REG_69_AGC_OVERRIDE1,                          config->reg_69);
    spiBurstWrite(RF22_REG_6E_TX_DATA_RATE1,                    &config->reg_6e, 5);
}

// Set one of the canned FSK Modem configs
// Returns true if its a valid choice
boolean RF22::setModemConfig(ModemConfigChoice index)
{
    if (index > (sizeof(MODEM_CONFIG_TABLE) / sizeof(ModemConfig)))
        return false;

    RF22::ModemConfig cfg;
    memcpy_P(&cfg, &MODEM_CONFIG_TABLE[index], sizeof(RF22::ModemConfig));
    setModemRegisters(&cfg);

    return true;
}

// REVISIT: top bit is in Header Control 2 0x33
void RF22::setPreambleLength(uint8_t nibbles)
{
    spiWrite(RF22_REG_34_PREAMBLE_LENGTH, nibbles);
}

// Caution doesnt set sync word len in Header Control 2 0x33
void RF22::setSyncWords(uint8_t* syncWords, uint8_t len)
{
    spiBurstWrite(RF22_REG_36_SYNC_WORD3, syncWords, len);
}

void RF22::clearRxBuf()
{
    _bufLen = 0;
    _rxBufValid = false;
}

boolean RF22::available()
{
    setModeRx();
    return _rxBufValid;
}

// Blocks until a valid message is received
void RF22::waitAvailable()
{
    while (!available())
	;
}

// Blocks until a valid message is received or timeout expires
// Return true if there is a message available
bool RF22::waitAvailableTimeout(uint16_t timeout)
{
    unsigned long endtime = millis() + timeout;
    while (millis() < endtime)
	if (available())
	    return true;
    return false;
}

void RF22::waitPacketSent()
{
    while (!_txPacketSent)
	;
}

boolean RF22::recv(uint8_t* buf, uint8_t* len)
{
    if (!available())
	return false;
    if (*len > _bufLen)
	*len = _bufLen;
    memcpy(buf, _buf, *len);
    clearRxBuf();
    return true;
}

void RF22::clearTxBuf()
{
    _bufLen = 0;
    _txBufSentIndex = 0;
    _txPacketSent = false;
}

void RF22::startTransmit()
{
    sendNextFragment(); // Actually the first fragment
    spiWrite(RF22_REG_3E_PACKET_LENGTH, _bufLen); // Total length that will be sent
    setModeTx(); // Start the transmitter, turns off the receiver
}

boolean RF22::send(uint8_t* data, uint8_t len)
{
    setModeIdle();
    fillTxBuf(data, len);
    startTransmit();
    return true;
}

boolean RF22::fillTxBuf(uint8_t* data, uint8_t len)
{
    clearTxBuf();
    return appendTxBuf(data, len);
}

boolean RF22::appendTxBuf(uint8_t* data, uint8_t len)
{
    if (((uint16_t)_bufLen + len) > RF22_MAX_MESSAGE_LEN)
	return false;
    memcpy(_buf + _bufLen, data, len);
    _bufLen += len;
    return true;
}

// Assumption: there is currently <= RF22_TXFFAEM_THRESHOLD bytes in the Tx FIFO
void RF22::sendNextFragment()
{
    if (_txBufSentIndex < _bufLen)
    {
	// Some left to send
	uint8_t len = _bufLen - _txBufSentIndex;
	// But dont send too much
	if (len > (RF22_FIFO_SIZE - RF22_TXFFAEM_THRESHOLD - 1))
	    len = (RF22_FIFO_SIZE - RF22_TXFFAEM_THRESHOLD - 1);
	spiBurstWrite(RF22_REG_7F_FIFO_ACCESS, _buf + _txBufSentIndex, len);
	_txBufSentIndex += len;
    }
}

// Assumption: there are at least RF22_RXFFAFULL_THRESHOLD in the RX FIFO
// That means it should only be called after a RXAFULL interrupt
void RF22::readNextFragment()
{
    if (((uint16_t)_bufLen + RF22_RXFFAFULL_THRESHOLD) > RF22_MAX_MESSAGE_LEN)
    {
	// Hmmm receiver overflow. Should never occur
	return;
    }
    // Read the RF22_RXFFAFULL_THRESHOLD octets that should be there
    spiBurstRead(RF22_REG_7F_FIFO_ACCESS, _buf + _bufLen, RF22_RXFFAFULL_THRESHOLD);
    _bufLen += RF22_RXFFAFULL_THRESHOLD;
}

// Clear the FIFOs
void RF22::resetFifos()
{
    spiWrite(RF22_REG_08_OPERATING_MODE2, RF22_FFCLRRX | RF22_FFCLRTX);
    spiWrite(RF22_REG_08_OPERATING_MODE2, 0);
}

// Clear the Rx FIFO
void RF22::resetRxFifo()
{
    spiWrite(RF22_REG_08_OPERATING_MODE2, RF22_FFCLRRX);
    spiWrite(RF22_REG_08_OPERATING_MODE2, 0);
}

// CLear the TX FIFO
void RF22::resetTxFifo()
{
    spiWrite(RF22_REG_08_OPERATING_MODE2, RF22_FFCLRTX);
    spiWrite(RF22_REG_08_OPERATING_MODE2, 0);
}

// Default implmentation does nothing. Override if you wish
void RF22::handleExternalInterrupt()
{
}

// Default implmentation does nothing. Override if you wish
void RF22::handleWakeupTimerInterrupt()
{
}

void RF22::setHeaderTo(uint8_t to)
{
    spiWrite(RF22_REG_3A_TRANSMIT_HEADER3, to);
}

void RF22::setHeaderFrom(uint8_t from)
{
    spiWrite(RF22_REG_3B_TRANSMIT_HEADER2, from);
}

void RF22::setHeaderId(uint8_t id)
{
    spiWrite(RF22_REG_3C_TRANSMIT_HEADER1, id);
}

void RF22::setHeaderFlags(uint8_t flags)
{
    spiWrite(RF22_REG_3D_TRANSMIT_HEADER0, flags);
}

uint8_t RF22::headerTo()
{
    return spiRead(RF22_REG_47_RECEIVED_HEADER3);
}

uint8_t RF22::headerFrom()
{
    return spiRead(RF22_REG_48_RECEIVED_HEADER2);
}

uint8_t RF22::headerId()
{
    return spiRead(RF22_REG_49_RECEIVED_HEADER1);
}

uint8_t RF22::headerFlags()
{
    return spiRead(RF22_REG_4A_RECEIVED_HEADER0);
}

uint8_t RF22::lastRssi()
{
    return _lastRssi;
}

void RF22::setPromiscuous(boolean promiscuous)
{
    spiWrite(RF22_REG_43_HEADER_ENABLE3, promiscuous ? 0x00 : 0xff);
}