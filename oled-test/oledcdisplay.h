#ifndef OLEDCDISPLAY_H
#define OLEDCDISPLAY_H

#include "gpio.h"
#include "spidevice.h"

#include <stdexcept>

class OledCDisplay
{
public:
	enum class MikroBusSlot {Number1, Number2};
public:
	OledCDisplay();
	void init(MikroBusSlot mikrobus_slot_number) throw(std::runtime_error);
	void drawBox(uint8_t x1, uint8_t y1, uint8_t width, uint8_t height, uint8_t r, uint8_t g, uint8_t b);
	void demo();
private:
	void spiWrite(uint8_t byte);
	void spiWrite(const SpiDevice::Buffer &buff);
	void sendCommand(uint8_t reg_index, uint8_t reg_value);
	void sendData(uint8_t data_value);
	void sendData(const SpiDevice::Buffer &buff);
	void accessDDRAM();
private:
	Gpio m_pinRST;
	Gpio m_pinCS;
	Gpio gpio_OLED_DC;
	Gpio gpio_OLED_RW;
	SpiDevice m_spi;
};

#endif // OLEDCDISPLAY_H
