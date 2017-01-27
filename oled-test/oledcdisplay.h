#ifndef OLEDCDISPLAY_H
#define OLEDCDISPLAY_H

#include "gpio.h"
#include "spidevice.h"

#include <stdexcept>
#include <tuple>

class Xpm;

class OledCDisplay
{
public:
	enum class MikroBusSlot {Number1, Number2};
	enum class Font {Latin1_8x8};
	struct Color
	{
		uint16_t value = 0;

		static Color Black;
		static Color White;
		static Color Red;
		static Color Green;
		static Color Blue;
		static Color Magenta;
		static Color Cyan;
		static Color Yellow;

		Color(uint16_t v = 0) : value(v) {}
		static Color fromRGB(uint8_t r, uint8_t g, uint8_t b);
		bool operator==(const Color &o) const {return value == o.value;}
		std::tuple<uint8_t, uint8_t, uint8_t> toRGB() const;
	};
	static Color ColorBlack;
public:
	OledCDisplay();
	void init(MikroBusSlot mikrobus_slot_number) throw(std::runtime_error);
	void drawBox(uint8_t x1, uint8_t y1, uint8_t width, uint8_t height, const Color &color = Color::White);
	void drawText(uint8_t x1, uint8_t y1, const std::string &text, const Color &fg = Color::White, const Color &bg = Color::Black, Font font = Font::Latin1_8x8);
	void demo();
#ifdef XPM_OUTPUT
	void writeXpmFile(const std::string &file_name = std::string());
#endif
private:
	void writeBox(uint8_t x1, uint8_t y1, uint8_t width, uint8_t height, const SpiDevice::Buffer &buff);
	void spiWrite(uint8_t byte);
	void spiWrite(const SpiDevice::Buffer &buff);
	void sendCommand(uint8_t reg_index, uint8_t reg_value);
	void sendData(uint8_t data_value);
	void sendData(const SpiDevice::Buffer &buff);
	void accessDDRAM();

	static const Xpm& fontXpm(Font font, int &char_w, int &char_h);
private:
	Gpio m_pinRST;
	Gpio m_pinCS;
	Gpio gpio_OLED_DC;
	Gpio gpio_OLED_RW;
	SpiDevice m_spi;
#ifdef XPM_OUTPUT
	std::vector<Color> m_xpmFrameBuffer;
#endif
};

#endif // OLEDCDISPLAY_H
