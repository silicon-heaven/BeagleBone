#include "oledcdisplay.h"
#include "seps114a.h"
#include "xpm.h"

#include <time.h>
#include <stdio.h>
#include <iostream>

namespace {

void delay_ms(int ms)
{
	struct timespec req;
	req.tv_sec = ms / 1000;
	req.tv_nsec = ms * 1000000L;
	nanosleep(&req, (struct timespec *)NULL);
}

#ifdef XPM_OUTPUT
void string_replace(std::string& str, char from, char to)
{
	for (size_t i = 0; i < str.length(); ++i) {
		if(str[i] == from)
			str[i] = to;
	}
}
#endif
/*
bool string_replace(std::string& str, const std::string& from, const std::string& to)
{
	size_t start_pos = str.find(from);
	if(start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}
*/
OledCDisplay::Color xpmColorToOled(uint8_t r, uint8_t g, uint8_t b)
{
	return OledCDisplay::Color::fromRGB(r, g, b);
}

namespace font {
#include "font/latin1_8x8.xpm"
}

}

OledCDisplay::Rect OledCDisplay::Rect::displayIntersection() const
{
	if(x1() >= OLED_C_SIZE || y1() >= OLED_C_SIZE)
		return Rect();

	Rect ret = *this;
	if(ret.x2() >= OLED_C_SIZE)
		ret.setX2(OLED_C_SIZE - 1);
	if(ret.y2() >= OLED_C_SIZE)
		ret.setY2(OLED_C_SIZE - 1);
	return ret;
}

OledCDisplay::Color OledCDisplay::Color::Black = OledCDisplay::Color::fromRGB(0x00, 0x00, 0x00);
OledCDisplay::Color OledCDisplay::Color::White = OledCDisplay::Color::fromRGB(0xff, 0xff, 0xff);
OledCDisplay::Color OledCDisplay::Color::Red = OledCDisplay::Color::fromRGB(0xff, 0x00, 0x00);
OledCDisplay::Color OledCDisplay::Color::Green = OledCDisplay::Color::fromRGB(0x00, 0xff, 0x00);
OledCDisplay::Color OledCDisplay::Color::Blue = OledCDisplay::Color::fromRGB(0x00, 0x00, 0xff);
OledCDisplay::Color OledCDisplay::Color::Magenta = OledCDisplay::Color::fromRGB(0xff, 0x00, 0xff);
OledCDisplay::Color OledCDisplay::Color::Cyan = OledCDisplay::Color::fromRGB(0x00, 0xff, 0xff);
OledCDisplay::Color OledCDisplay::Color::Yellow = OledCDisplay::Color::fromRGB(0xff, 0xff, 0x00);

OledCDisplay::Color OledCDisplay::Color::fromRGB(uint8_t r, uint8_t g, uint8_t b)
{
	Color c;
	// rrrrrggg gggbbbbb
	c.value = (r & ~7) | (g >> 5);
	c.value <<= 8;
	c.value = c.value | (((g << 3) & ~31) | (b >> 3));
	return c;
}

std::tuple<uint8_t, uint8_t, uint8_t> OledCDisplay::Color::toRGB() const
{
	return std::make_tuple(
				(uint8_t)((value >> 8) & ~7)
				, (uint8_t)((value >> 3) & ~3)
				, (uint8_t)(value << 3)
				);
}

OledCDisplay::OledCDisplay()
	: m_spi(false)
{
#ifdef XPM_OUTPUT
	m_xpmFrameBuffer.resize(OLED_C_SIZE * OLED_C_SIZE);
#endif
}

void OledCDisplay::spiWrite(uint8_t byte)
{
	m_spi.transfer(SpiDevice::Buffer{byte});
}

void OledCDisplay::spiWrite(const SpiDevice::Buffer &buff)
{
	m_spi.transfer(buff);
}

void OledCDisplay::sendCommand(uint8_t reg_index, uint8_t reg_value)
{
	//Select index addr
	m_pinCS.setValue(0);
	gpio_OLED_DC.setValue(0);
	spiWrite(reg_index);
	m_pinCS.setValue(1);
	//Write data to reg
	m_pinCS.setValue(0);
	gpio_OLED_DC.setValue(1);
	spiWrite(reg_value);
	m_pinCS.setValue(1);
}

//Send data to OLED C display
void OledCDisplay::sendData(uint8_t data_value)
{
	m_pinCS.setValue(0);
	gpio_OLED_DC.setValue(1);
	spiWrite(data_value);
	m_pinCS.setValue(1);
}

void OledCDisplay::sendData(const SpiDevice::Buffer &buff)
{
	m_pinCS.setValue(0);
	gpio_OLED_DC.setValue(1);
	spiWrite(buff);
	m_pinCS.setValue(1);
}

//Sekvence before writing data to memory
void OledCDisplay::accessDDRAM()
{
	m_pinCS.setValue(0);
	gpio_OLED_DC.setValue(0);
	spiWrite(0x08);
	m_pinCS.setValue(1);
}

const Xpm &OledCDisplay::fontXpm(OledCDisplay::Font font, int &char_w, int &char_h)
{
	if(font == Font::Latin1_8x8) {
		static Xpm xpm;
		if(!xpm.isValid())
			xpm.wrapData(font::latin1_8x8_xpm);
		char_w = 8;
		char_h = 8;
		return xpm;
	}
	throw std::runtime_error("unsupported font");
	char_w = 0;
	char_h = 0;
	static Xpm null_xpm;
	return null_xpm;
}

// OLED_RST <--- MikroBus RST Reset
// OLED_CS  <--- MikroBus CS  Chip select
// OLED_DC  <--- MikroBus PWM Data/Command
// OLED_RW  <--- MikroBus AN  Read/Write

/*
[   36.432606] pinctrl-single 44e10800.pinmux: pin 44e10990.0 already requested by 48038000.mcasp; cannot claim for 481a0000.spi
[   36.444350] pinctrl-single 44e10800.pinmux: pin-100 (481a0000.spi) status -22
[   36.451692] pinctrl-single 44e10800.pinmux: could not request pin 100 (44e10990.0) from group pinmux_bb_spi1_pins  on device pinctrl-single
[   36.464461] omap2_mcspi 481a0000.spi: Error applying setting, reverse things back

FIXED by disabling HDMI(mcasp) in /boot/uEnv.txt by uncommenting

##BeagleBone Black: HDMI (Audio/Video) disabled:
dtb=am335x-boneblack-emmc-overlay.dtb

*/
void OledCDisplay::init(MikroBusSlot mikrobus_slot_number) throw(std::runtime_error)
{
#ifdef XPM_OUTPUT
	return;
#endif
	const char *spi_device_name = nullptr;
	if(mikrobus_slot_number == MikroBusSlot::Number1) {
		m_pinRST.setNumber(1, 28); // P9.12 GPIO1_28
		m_pinCS.setNumber(3, 17); // P9.28 GPIO3_17
		gpio_OLED_DC.setNumber(1, 18); // P9.14 GPIO1_18
		gpio_OLED_RW.setNumber(1, 29); // P8.26 GPIO1_29
		spi_device_name = "/dev/spidev1.0";
	}
	else if(mikrobus_slot_number == MikroBusSlot::Number2) {
		m_pinRST.setNumber(1, 17); // P9.23 GPIO1_17
		m_pinCS.setNumber(0, 07); // P9.42 GPIO0_7
		gpio_OLED_DC.setNumber(1, 19); // P9.16 GPIO1_19
		gpio_OLED_RW.setNumber(1, 15); // P8.15 GPIO1_15
		spi_device_name = "/dev/spidev1.1";
	}
	else {
		throw std::runtime_error("unsupported mikro bus slot number: " + std::to_string((int)mikrobus_slot_number));
	}

	if(m_pinRST.checkExported()) m_pinRST.setDirection(Gpio::Direction::Out);
	if(m_pinCS.checkExported()) m_pinCS.setDirection(Gpio::Direction::Out);
	if(gpio_OLED_DC.checkExported()) gpio_OLED_DC.setDirection(Gpio::Direction::Out);
	if(gpio_OLED_RW.checkExported()) gpio_OLED_RW.setDirection(Gpio::Direction::Out);

	uint8_t spi_mode = 0;
	//spi_mode |= SPI_LOOP; //loopback
	//spi_mode |= SPI_CPHA; //clock phase
	//spi_mode |= SPI_CPOL; //clock polarity
	//spi_mode |= SPI_LSB_FIRST;
	//spi_mode |= SPI_CS_HIGH;
	//spi_mode |= SPI_3WIRE; //SI/SO signals shared
	//spi_mode |= SPI_NO_CS;
	//spi_mode |= SPI_READY;

	m_spi.open(spi_device_name, spi_mode);

	gpio_OLED_RW.setValue(0);

	m_pinRST.setValue(0);
	delay_ms(10);
	m_pinRST.setValue(1);
	delay_ms(10);
	/*  Soft reser */
	sendCommand(SEPS114A_SOFT_RESET,0x00);
	/* Standby ON/OFF*/
	sendCommand(SEPS114A_STANDBY_ON_OFF,0x01);          // Standby on
	delay_ms(5);                                           // Wait for 5ms (1ms Delay Minimum)
	sendCommand(SEPS114A_STANDBY_ON_OFF,0x00);          // Standby off
	delay_ms(5);                                           // 1ms Delay Minimum (1ms Delay Minimum)
	/* Display OFF */
	sendCommand(SEPS114A_DISPLAY_ON_OFF,0x00);
	/* Set Oscillator operation */
	sendCommand(SEPS114A_ANALOG_CONTROL,0x00);          // using external resistor and internal OSC
	/* Set frame rate */
	sendCommand(SEPS114A_OSC_ADJUST,0x03);              // frame rate : 95Hz
	/* Set active display area of panel */
	sendCommand(SEPS114A_DISPLAY_X1,0x00);
	sendCommand(SEPS114A_DISPLAY_X2,OLED_C_SIZE - 1);
	sendCommand(SEPS114A_DISPLAY_Y1,0x00);
	sendCommand(SEPS114A_DISPLAY_Y2,OLED_C_SIZE - 1);
	/* Select the RGB data format and set the initial state of RGB interface port */
	sendCommand(SEPS114A_RGB_IF,0x00);                 // RGB 8bit interface
	/* Set RGB polarity */
	sendCommand(SEPS114A_RGB_POL,0x00);
	/* Set display mode control */
	sendCommand(SEPS114A_DISPLAY_MODE_CONTROL,0x80);   // SWAP:BGR, Reduce current : Normal, DC[1:0] : Normal
	/* Set MCU Interface */
	sendCommand(SEPS114A_CPU_IF,0x00);                 // MPU External interface mode, 8bits
	/* Set Memory Read/Write mode */
	sendCommand(SEPS114A_MEMORY_WRITE_READ,0x00);
	/* Set row scan direction */
	sendCommand(SEPS114A_ROW_SCAN_DIRECTION,0x00);     // Column : 0 --> Max, Row : 0 --> Max
	/* Set row scan mode */
	sendCommand(SEPS114A_ROW_SCAN_MODE,0x00);          // Alternate scan mode
	/* Set column current */
	sendCommand(SEPS114A_COLUMN_CURRENT_R,0x6E);
	sendCommand(SEPS114A_COLUMN_CURRENT_G,0x4F);
	sendCommand(SEPS114A_COLUMN_CURRENT_B,0x77);
	/* Set row overlap */
	sendCommand(SEPS114A_ROW_OVERLAP,0x00);            // Band gap only
	/* Set discharge time */
	sendCommand(SEPS114A_DISCHARGE_TIME,0x01);         // Discharge time : normal discharge
	/* Set peak pulse delay */
	sendCommand(SEPS114A_PEAK_PULSE_DELAY,0x00);
	/* Set peak pulse width */
	sendCommand(SEPS114A_PEAK_PULSE_WIDTH_R,0x02);
	sendCommand(SEPS114A_PEAK_PULSE_WIDTH_G,0x02);
	sendCommand(SEPS114A_PEAK_PULSE_WIDTH_B,0x02);
	/* Set precharge current */
	sendCommand(SEPS114A_PRECHARGE_CURRENT_R,0x14);
	sendCommand(SEPS114A_PRECHARGE_CURRENT_G,0x50);
	sendCommand(SEPS114A_PRECHARGE_CURRENT_B,0x19);
	/* Set row scan on/off  */
	sendCommand(SEPS114A_ROW_SCAN_ON_OFF,0x00);        // Normal row scan
	/* Set scan off level */
	sendCommand(SEPS114A_SCAN_OFF_LEVEL,0x04);         // VCC_C*0.75
	/* Set memory access point */
	sendCommand(SEPS114A_DISPLAYSTART_X,0x00);
	sendCommand(SEPS114A_DISPLAYSTART_Y,0x00);
	/* Display ON */
	sendCommand(SEPS114A_DISPLAY_ON_OFF,0x01);
}

void OledCDisplay::writeBox(uint8_t x1, uint8_t y1, uint8_t width, uint8_t height, const SpiDevice::Buffer &buff)
{
	//std::cerr << "writeBox" << (int)x1 << ", " << (int)y1 << ", " << (int)width << ", " << (int)height << ", size: " << buff.size() << std::endl;
#ifdef XPM_OUTPUT
	//int x2 = x1 + width - 1;
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			int buff_ix = 2 * (i * width + j);
			Color c;
			c.value = buff[buff_ix];
			c.value <<= 8;
			c.value = c.value | buff[buff_ix + 1];
			m_xpmFrameBuffer[(y1 + i) * OLED_C_SIZE + x1 + j] = c;
		}
	}
#else
	sendCommand(SEPS114A_MEM_X1, x1);
	sendCommand(SEPS114A_MEM_X2, x1 + width - 1);
	sendCommand(SEPS114A_MEM_Y1, y1);
	sendCommand(SEPS114A_MEM_Y2, y1 + height - 1);
	accessDDRAM();
	sendData(buff);
#endif
}

void OledCDisplay::drawBox(uint8_t x1, uint8_t y1, uint8_t width, uint8_t height, const Color &color)
{
	SpiDevice::Buffer buff;
	buff.resize(height * width * 2);
	int n = 0;
	for (int j = 0; j < height; ++j) {
		for (size_t i = 0; i < width; ++i) {
			// rrrrrggg gggbbbbb
			buff[n] = color.value / 256;
			buff[n+1] = color.value % 256;
			n += 2;
		}
	}
	writeBox(x1, y1, width, height, buff);
}

void OledCDisplay::drawText(uint8_t x1, uint8_t y1, const std::string &text, const OledCDisplay::Color &fg, const OledCDisplay::Color &bg, OledCDisplay::Font font)
{
	int char_w;
	int char_h;
	const Xpm font_xpm = fontXpm(font, char_w, char_h);

	Rect disp_rect(x1, y1, text.length() * char_w, char_h);
	disp_rect = disp_rect.displayIntersection();
	SpiDevice::Buffer buff;
	buff.resize(disp_rect.height() * disp_rect.width() * 2);
	for (size_t char_ix = 0; char_ix < text.length(); ++char_ix) {
		int char_ascii = (uint8_t)text[char_ix];
		int glyph_row = (char_ascii / 16)  * char_h;
		int glyph_col = (char_ascii % 16) * char_w;
		Rect glyph_xpm_rect(glyph_col, glyph_row, char_w, char_h);
		//Rect glyph_dest_rect(char_ix * char_w, 0, char_w, char_h);

		copyXpmToSpiBufer(font_xpm
						  , buff
						  , glyph_xpm_rect
						  , disp_rect.size()
						  , Point(char_ix * char_w, 0)
						  , [fg, bg](uint8_t r, uint8_t g, uint8_t b) -> OledCDisplay::Color
		{
			Xpm::Color xpm_color(r, g, b);
			OledCDisplay::Color c = (xpm_color == Xpm::Color::Black)? fg: bg;
			return c;
		});

	}
	writeBox(disp_rect, buff);
}

void OledCDisplay::drawXpmImage(uint8_t x1, uint8_t y1, const char **xpm_data)
{
	try {
		Xpm xpm;
		xpm.wrapData(xpm_data);

		Rect disp_rect(x1, y1, xpm.width(), xpm.height());
		disp_rect = disp_rect.displayIntersection();
		SpiDevice::Buffer buff;
		buff.resize(disp_rect.height() * disp_rect.width() * 2);

		copyXpmToSpiBufer(xpm
						  , buff
						  , Rect(0, 0, xpm.width(), xpm.height())
						  , disp_rect.size()
						  , Point(0, 0)
						  , xpmColorToOled);

		writeBox(disp_rect, buff);
	}
	catch (std::runtime_error &e) {
		std::cerr << e.what();
	}
}

void OledCDisplay::copyXpmToSpiBufer(const Xpm &xpm
									 , SpiDevice::Buffer &buff
									 , const OledCDisplay::Rect &src_rect
									 , const Size &buff_size
									 , const Point &buff_pos
									 , std::function<OledCDisplay::Color(uint8_t r, uint8_t g, uint8_t b)> color_convert)
{
	int max_height = buff_size.height - buff_pos.y;
	int max_width = buff_size.width - buff_pos.x;
	for (int j = 0; j < src_rect.height() && j < max_height; ++j) {
		for (int k = 0; k < src_rect.width() && k < max_width; ++k) {
			const Xpm::Color &xpm_color = xpm.colorAt(src_rect.y1() + j, src_rect.x1() + k);
			int buff_ix = ((j + buff_pos.y) * buff_size.width + (buff_size.width - (buff_pos.x + k) - 1)) * 2;
			OledCDisplay::Color color = color_convert(xpm_color.r, xpm_color.g, xpm_color.b);
			buff[buff_ix] = color.value / 256;
			buff[buff_ix+1] = color.value % 256;
		}
	}
}

void OledCDisplay::demo()
{
	/*
	sendCommand(SEPS114A_MEMORY_WRITE_READ, 0x02);                //Set Memory Read/Write mode
	int color = 0xFF0000;
	for (int offset = 0; offset < OLED_C_SIZE / 2; offset+=5) {
		int width = OLED_C_SIZE - 2 * offset;
		uint8_t r = (color >> 16);
		uint8_t g = (color >> 8);
		uint8_t b = color;
		drawBox(offset, offset, width, width, Color::fromRGB(r, g, b));
		color >>= 2;
	}
	delay_ms(5000);
	*/
	for (int j = 0; j < 10; ++j) {
		for(int i=0; i<8; i++){
			sendCommand(SEPS114A_SCREEN_SAVER_MODE,i);
			sendCommand(SEPS114A_SCREEN_SAVER_CONTEROL,0x88);
			delay_ms(5000);
			sendCommand(SEPS114A_SCREEN_SAVER_CONTEROL,0x00);
		}
		delay_ms(5000);
	}
}

#ifdef XPM_OUTPUT
void OledCDisplay::writeXpmFile(const std::string &file_name)
{
	std::string fn = file_name;
	if(!fn.length()) {
		static int no = 0;
		fn = std::to_string(++no) + ".xpm";
	}
	std::FILE* fp = std::fopen(fn.c_str(), "w");
	if(!fp) {
		perror("Error open XPM file for write");
		throw std::runtime_error("Error open XPM file for write");
	}
	char cch1 = '#';
	char cch = cch1;
	// scan colors
	std::map<uint16_t, char> color_map;
	for(const Color &clr : m_xpmFrameBuffer) {
		if(color_map.count(clr.value) == 0) {
			color_map[clr.value] = cch;
			if(cch < '~') {
				cch++;
			}
			else {
				std::cerr << "Max color count exceeded!";
				break;
			}
		}
	}
	std::fprintf(fp, "/* XPM */\n");
	std::string arr_name = file_name;
	string_replace(arr_name, '.', '_');
	std::fprintf(fp, "static const char * a%s[] = {\n", arr_name.c_str());
	std::fprintf(fp, "\"%d %d %d 1\",\n", OLED_C_SIZE, OLED_C_SIZE, (int)color_map.size() + 1);
	std::fprintf(fp, "\"  c None\",\n");
	for(const auto &kv : color_map) {
		std::tuple<uint8_t, uint8_t, uint8_t> rgb = Color(kv.first).toRGB();
		std::fprintf(fp, "\"%c c #%02x%02x%02x\",\n", kv.second, std::get<0>(rgb), std::get<1>(rgb), std::get<2>(rgb));
		//fprintf(stderr, "%04x -> %02x %02x %02x\n", kv.first, std::get<0>(rgb), std::get<1>(rgb), std::get<2>(rgb));
	}
	for (int j = 0; j < OLED_C_SIZE; ++j) {
		std::fprintf(fp, "\"");
		for (int i = OLED_C_SIZE - 1; i >= 0; --i) {
			char c = ' ';
			const Color &clr = m_xpmFrameBuffer[j * OLED_C_SIZE + i];
			auto it = color_map.find(clr.value);
			if(it != color_map.end())
				c = it->second;
			std::fprintf(fp, "%c", c);
		}
		if(j == OLED_C_SIZE - 1)
			std::fprintf(fp, "\"};\n");
		else
			std::fprintf(fp, "\",\n");
	}
}

#endif







