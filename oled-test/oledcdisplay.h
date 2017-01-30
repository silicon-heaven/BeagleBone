#ifndef OLEDCDISPLAY_H
#define OLEDCDISPLAY_H

#include "gpio.h"
#include "spidevice.h"

#include <stdexcept>
#include <tuple>
#include <functional>

class Xpm;

class OledCDisplay
{
public:
	static constexpr int OLED_C_SIZE = 96;
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
	void drawXpmImage(uint8_t x1, uint8_t y1, const char **xpm_data);
	void demo();
#ifdef XPM_OUTPUT
	void writeXpmFile(const std::string &file_name = std::string());
#endif
private:
	struct Point
	{
		int x = 0;
		int y = 0;

		Point() {}
		Point(int x, int y) : x(x), y(y) {}
	};
	struct Size
	{
		int width = 0;
		int height = 0;

		Size() {}
		Size(int w, int h) : width(w), height(h) {}
	};
	struct Rect
	{
		Point p;
		Size sz;

		Rect() {}
		Rect(int x, int y, int w, int h) : p(x, y), sz(w, h) {}

		int x1() const {return p.x;}
		int y1() const {return p.y;}
		int x2() const {return p.x + sz.width - 1;}
		void setX2(int xx) {int w = xx - x1() + 1; if(w < 0) w = 0; sz.width = w;}
		int y2() const {return p.y + sz.height - 1;}
		void setY2(int xx) {int h = xx - y1() + 1; if(h < 0) h = 0; sz.height = h;}
		const Point& leftTop() const {return p;}
		const Size& size() const {return sz;}
		int width() const {return sz.width;}
		int height() const {return sz.height;}

		Rect displayIntersection() const;
	};

	void copyXpmToSpiBufer(const Xpm &xpm
						   , SpiDevice::Buffer &buff
						   , const Rect &src_rect
						   , const Size &buff_size
						   , const Point &buff_pos
						   , std::function<Color(uint8_t r, uint8_t g, uint8_t b)> color_convert);

	void writeBox(uint8_t x1, uint8_t y1, uint8_t width, uint8_t height, const SpiDevice::Buffer &buff);
	void writeBox(const Rect &rect, const SpiDevice::Buffer &buff) {
		writeBox(rect.x1(), rect.y1(), rect.width(), rect.height(), buff);
	}

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
