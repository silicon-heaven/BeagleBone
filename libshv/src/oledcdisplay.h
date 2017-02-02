#ifndef SHV_OLEDCDISPLAY_H
#define SHV_OLEDCDISPLAY_H

#include "shvglobal.h"

#include "gpio.h"
#include "spidevice.h"
#include "xpm.h"

#include <stdexcept>
#include <tuple>
#include <functional>

namespace shv {

class Xpm;

class SHV_DECL_EXPORT OledCDisplay
{
public:
	static constexpr int OLED_C_SIZE = 96;
	enum class MikroBusSlot {Invalid = 0, Number1, Number2};
	enum class OutputType {OLED, XPM};
	enum class Font {Latin1_8x8};

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
		Rect(const Point &left_top, const Size &size) : p(left_top), sz(size) {}

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

public:
	OledCDisplay();
	void init(MikroBusSlot mikrobus_slot_number, OutputType output_type) throw(std::runtime_error);

	bool isXpmOutput() const {return m_xpmFrameBuffer.size() > 0;}

	void drawBox(uint8_t x1, uint8_t y1, uint8_t width, uint8_t height, const Xpm::Color &color = Xpm::Color::White);
	void drawBox(const Rect &rect, const Xpm::Color &color = Xpm::Color::White) {drawBox(rect.x1(), rect.y1(), rect.width(), rect.height(), color);}
	void drawText(uint8_t x1, uint8_t y1, const std::string &text, const Xpm::Color &fg = Xpm::Color::White, const Xpm::Color &bg = Xpm::Color::Black, Font font = Font::Latin1_8x8);
	void drawText(const Point &pos, const std::string &text, const XpmFont &font, std::function<Xpm::Color(const Xpm::Color &)> color_convert = nullptr);
	void drawXpmImage(const Xpm &xpm, const Point &dest_point = Point(), std::function<Xpm::Color(const Xpm::Color &xpm_color)> color_convert = nullptr);
	void drawXpmImage(const Xpm &xpm, const Rect &src_rect, const Point &dest_point, std::function<Xpm::Color(const Xpm::Color &xpm_color)> color_convert = nullptr);
	void demo();

	void writeXpmFile(const std::string &file_name = std::string());

private:
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

	void copyXpmToSpiBufer(const Xpm &xpm
						   , SpiDevice::Buffer &buff
						   , const Rect &src_rect
						   , const Size &buff_size
						   , const Point &buff_pos
						   , std::function<Xpm::Color (const Xpm::Color &)> color_convert = nullptr);

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

	static OledCDisplay::Color xpmColorToOled(const Xpm::Color &xpm_color);

	static const XpmFont &fontXpm(Font font);
private:
	Gpio m_pinRST;
	Gpio m_pinCS;
	Gpio gpio_OLED_DC;
	Gpio gpio_OLED_RW;
	SpiDevice m_spi;

	std::vector<Xpm::Color> m_xpmFrameBuffer;
};

inline const OledCDisplay::Point operator+(const OledCDisplay::Point &p1, const OledCDisplay::Point &p2) { return OledCDisplay::Point(p1.x + p2.x, p1.y + p2.y); }
inline const OledCDisplay::Point operator-(const OledCDisplay::Point &p1, const OledCDisplay::Point &p2) { return OledCDisplay::Point(p1.x - p2.x, p1.y - p2.y); }

}

#endif // OLEDCDISPLAY_H
