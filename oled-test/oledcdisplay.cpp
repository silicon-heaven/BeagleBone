#include "oledcdisplay.h"
#include "seps114a.h"

#include <time.h>

namespace {
constexpr uint8_t OLED_C_SIZE = 96;
void delay_ms(int ms)
{
	struct timespec req;
	req.tv_sec = ms / 1000;
	req.tv_nsec = ms * 1000000L;
	nanosleep(&req, (struct timespec *)NULL);
}
}

OledCDisplay::OledCDisplay()
{

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

void OledCDisplay::drawBox(uint8_t x1, uint8_t y1, uint8_t width, uint8_t height, uint8_t r, uint8_t g, uint8_t b)
{
	r = r >> 3;
	g = g >> 2;
	b = b >> 3;

	sendCommand(SEPS114A_MEM_X1, x1);
	sendCommand(SEPS114A_MEM_X2, x1 + width - 1);
	sendCommand(SEPS114A_MEM_Y1, y1);
	sendCommand(SEPS114A_MEM_Y2, y1 + height - 1);
	accessDDRAM();
	SpiDevice::Buffer buff;
	buff.resize(height * width * 2);
	int n = 0;
	for (int j = 0; j < height; ++j) {
		for (size_t i = 0; i < width; ++i) {
			// rrrrrggg gggbbbbb
			buff[n] = (r << 3) | (g >> 3);
			buff[n+1] = (g << 5) | b;
			n += 2;
		}
	}
	sendData(buff);
}

void OledCDisplay::demo()
{
	sendCommand(0x1D, 0x02);                //Set Memory Read/Write mode
	int color = 0xFF0000;
	for (int offset = 0; offset < OLED_C_SIZE / 2; offset+=5) {
		int width = OLED_C_SIZE - 2 * offset;
		uint8_t r = (color >> 16);
		uint8_t g = (color >> 8);
		uint8_t b = color;
		drawBox(offset, offset, width, width, r, g, b);
		color >>= 2;
	}
	delay_ms(5000);
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
