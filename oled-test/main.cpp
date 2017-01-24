/*
 * Project name:
 *	  OLED C Click
 * Copyright:
 *	  (c) mikroElektronika, 2014.
 * Revision History:
 *	  20140919:
 *	  - Initial release (BD);
 * Description:
 *	  OLED C click lets you add a small but bright and crisp display to your design.
 *	  It carries a passive matrix OLED display capable of displaying over 65k colors, and a SEPS114A display driver and controller IC.
 *	  OLED C click communicates with the target board through mikroBUS SPI lines: CS, SCK, and MOSI (SDI); and PWM (A/C) and RST lines.
 *
 *	  They are bright, have a wide viewing angle and low power consumption (with a 20mA maximum).
 *	  The display on OLED C click is 19.8 x 19.8mm with a 96 x 96px resolution.
 *	  The SEPS114A controller has built-in functionalities like screen saver (vertical scroll, horizontal panning, fade in/out),
 *	  programmable panel size, power save mode and so forth.
 *
 *	  OLED C click uses a 3.3V power supply.
 *
 * Test configuration:
 *	  MCU:             STM32F107VC
 *					   http://www.st.com/st-web-ui/static/active/en/resource/technical/document/reference_manual/CD00171190.pdf
 *	  Dev. Board:      EasyMx PRO v7 for STM32
 *					   http://www.mikroe.com/eng/products/view/852/easymx-pro-v7-for-stm32/
 *	  Oscillator:      HS-PLL 72.0000 MHz
 *	  ext. modules:    OLED C click
 *					   http://www.mikroe.com/click/oled-c/
 *	  SW:              mikroC PRO for ARM
 *					   http://www.mikroe.com/mikroc/arm/
 * NOTES:
 *	 - Place OLED C Click board in the mikroBUS socket 1.
 */

#include "seps114a.h"
#include "gpio.h"
#include "spidevice.h"

#include <iostream>
#include <time.h>

static int mikrobus_slot_number = 2;

static Gpio gpio_OLED_RST;
static Gpio gpio_OLED_CS;
static Gpio gpio_OLED_DC;
static Gpio gpio_OLED_RW;

static const char *spi_device_name = "/dev/spidev1.1";
static SpiDevice spi(true);

static const uint8_t OLED_C_SIZE = 96;

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

void delay_ms(int ms)
{
	struct timespec req;
	req.tv_sec = ms / 1000;
	req.tv_nsec = ms * 1000000L;
	nanosleep(&req, (struct timespec *)NULL);
}

// Init BB function
void initBB()
{
	if(mikrobus_slot_number == 1) {
		gpio_OLED_RST.setNumber(1, 28); // P9.12 GPIO1_28
		gpio_OLED_CS.setNumber(3, 17); // P9.28 GPIO3_17
		gpio_OLED_DC.setNumber(1, 18); // P9.14 GPIO1_18
		gpio_OLED_RW.setNumber(1, 29); // P8.26 GPIO1_29
	}
	else if(mikrobus_slot_number == 2) {
		gpio_OLED_RST.setNumber(1, 17); // P9.23 GPIO1_17
		gpio_OLED_CS.setNumber(0, 07); // P9.42 GPIO0_7
		gpio_OLED_DC.setNumber(1, 19); // P9.16 GPIO1_19
		gpio_OLED_RW.setNumber(1, 15); // P8.15 GPIO1_15
	}
	else {
		std::cerr << "unsupported mikro bus slot number:" << mikrobus_slot_number << "\n";
		exit(1);
	}

	if(gpio_OLED_RST.checkExported()) gpio_OLED_RST.setDirection(Gpio::Direction::Out);
	if(gpio_OLED_CS.checkExported()) gpio_OLED_CS.setDirection(Gpio::Direction::Out);
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

	spi.open(spi_device_name, spi_mode);
	gpio_OLED_RW.setValue(false);
	//Delay_ms(100);
}

void spi_write(uint8_t byte)
{
	spi.transfer(SpiDevice::Buffer{byte});
}

void spi_write(const SpiDevice::Buffer &buff)
{
	spi.transfer(buff);
}

//Send command to OLED C display
void OLED_C_sendCommand(unsigned char reg_index, unsigned char reg_value)
{
	//Select index addr
	gpio_OLED_CS.setValue(0);
	gpio_OLED_DC.setValue(0);
	spi_write(reg_index);
	gpio_OLED_CS.setValue(1);
	//Write data to reg
	gpio_OLED_CS.setValue(0);
	gpio_OLED_DC.setValue(1);
	spi_write(reg_value);
	gpio_OLED_CS.setValue(1);
}

//Send data to OLED C display
void OLED_C_sendData(unsigned char data_value)
{
	gpio_OLED_CS.setValue(0);
	gpio_OLED_DC.setValue(1);
	spi_write(data_value);
	gpio_OLED_CS.setValue(1);
}

void OLED_C_sendData(const SpiDevice::Buffer &buff)
{
	gpio_OLED_CS.setValue(0);
	gpio_OLED_DC.setValue(1);
	spi_write(buff);
	gpio_OLED_CS.setValue(1);
}

// Init sequence for 96x96 OLED color module
void OLED_C_init()
{
	gpio_OLED_RST.setValue(0);
	delay_ms(10);
	gpio_OLED_RST.setValue(1);
	delay_ms(10);
	/*  Soft reser */
	OLED_C_sendCommand(SEPS114A_SOFT_RESET,0x00);
	/* Standby ON/OFF*/
	OLED_C_sendCommand(SEPS114A_STANDBY_ON_OFF,0x01);          // Standby on
	delay_ms(5);                                           // Wait for 5ms (1ms Delay Minimum)
	OLED_C_sendCommand(SEPS114A_STANDBY_ON_OFF,0x00);          // Standby off
	delay_ms(5);                                           // 1ms Delay Minimum (1ms Delay Minimum)
	/* Display OFF */
	OLED_C_sendCommand(SEPS114A_DISPLAY_ON_OFF,0x00);
	/* Set Oscillator operation */
	OLED_C_sendCommand(SEPS114A_ANALOG_CONTROL,0x00);          // using external resistor and internal OSC
	/* Set frame rate */
	OLED_C_sendCommand(SEPS114A_OSC_ADJUST,0x03);              // frame rate : 95Hz
	/* Set active display area of panel */
	OLED_C_sendCommand(SEPS114A_DISPLAY_X1,0x00);
	OLED_C_sendCommand(SEPS114A_DISPLAY_X2,OLED_C_SIZE - 1);
	OLED_C_sendCommand(SEPS114A_DISPLAY_Y1,0x00);
	OLED_C_sendCommand(SEPS114A_DISPLAY_Y2,OLED_C_SIZE - 1);
	/* Select the RGB data format and set the initial state of RGB interface port */
	OLED_C_sendCommand(SEPS114A_RGB_IF,0x00);                 // RGB 8bit interface
	/* Set RGB polarity */
	OLED_C_sendCommand(SEPS114A_RGB_POL,0x00);
	/* Set display mode control */
	OLED_C_sendCommand(SEPS114A_DISPLAY_MODE_CONTROL,0x80);   // SWAP:BGR, Reduce current : Normal, DC[1:0] : Normal
	/* Set MCU Interface */
	OLED_C_sendCommand(SEPS114A_CPU_IF,0x00);                 // MPU External interface mode, 8bits
	/* Set Memory Read/Write mode */
	OLED_C_sendCommand(SEPS114A_MEMORY_WRITE_READ,0x00);
	/* Set row scan direction */
	OLED_C_sendCommand(SEPS114A_ROW_SCAN_DIRECTION,0x00);     // Column : 0 --> Max, Row : 0 --> Max
	/* Set row scan mode */
	OLED_C_sendCommand(SEPS114A_ROW_SCAN_MODE,0x00);          // Alternate scan mode
	/* Set column current */
	OLED_C_sendCommand(SEPS114A_COLUMN_CURRENT_R,0x6E);
	OLED_C_sendCommand(SEPS114A_COLUMN_CURRENT_G,0x4F);
	OLED_C_sendCommand(SEPS114A_COLUMN_CURRENT_B,0x77);
	/* Set row overlap */
	OLED_C_sendCommand(SEPS114A_ROW_OVERLAP,0x00);            // Band gap only
	/* Set discharge time */
	OLED_C_sendCommand(SEPS114A_DISCHARGE_TIME,0x01);         // Discharge time : normal discharge
	/* Set peak pulse delay */
	OLED_C_sendCommand(SEPS114A_PEAK_PULSE_DELAY,0x00);
	/* Set peak pulse width */
	OLED_C_sendCommand(SEPS114A_PEAK_PULSE_WIDTH_R,0x02);
	OLED_C_sendCommand(SEPS114A_PEAK_PULSE_WIDTH_G,0x02);
	OLED_C_sendCommand(SEPS114A_PEAK_PULSE_WIDTH_B,0x02);
	/* Set precharge current */
	OLED_C_sendCommand(SEPS114A_PRECHARGE_CURRENT_R,0x14);
	OLED_C_sendCommand(SEPS114A_PRECHARGE_CURRENT_G,0x50);
	OLED_C_sendCommand(SEPS114A_PRECHARGE_CURRENT_B,0x19);
	/* Set row scan on/off  */
	OLED_C_sendCommand(SEPS114A_ROW_SCAN_ON_OFF,0x00);        // Normal row scan
	/* Set scan off level */
	OLED_C_sendCommand(SEPS114A_SCAN_OFF_LEVEL,0x04);         // VCC_C*0.75
	/* Set memory access point */
	OLED_C_sendCommand(SEPS114A_DISPLAYSTART_X,0x00);
	OLED_C_sendCommand(SEPS114A_DISPLAYSTART_Y,0x00);
	/* Display ON */
	OLED_C_sendCommand(SEPS114A_DISPLAY_ON_OFF,0x01);
}

//Sekvence before writing data to memory
void DDRAM_access()
{
	gpio_OLED_CS.setValue(0);
	gpio_OLED_DC.setValue(0);
	spi_write(0x08);
	gpio_OLED_CS.setValue(1);
}

//Set memory area(address) to write a display data
void OLED_C_MemorySize(char X1, char X2, char Y1, char Y2)
{
	OLED_C_sendCommand(SEPS114A_MEM_X1,X1);
	OLED_C_sendCommand(SEPS114A_MEM_X2,X2);
	OLED_C_sendCommand(SEPS114A_MEM_Y1,Y1);
	OLED_C_sendCommand(SEPS114A_MEM_Y2,Y2);
}

//Select color
void OLED_C_Color(char color_msb, char color_lsb )
{
	OLED_C_sendData(color_msb);
	OLED_C_sendData(color_lsb);
}

void OLED_C_Background()
{
	OLED_C_sendCommand(0x1D, 0x02);                //Set Memory Read/Write mode
	int step_no = 0;
	int step_size = 5;
	{
		int offset = step_no * step_size;
		int width = OLED_C_SIZE - 2 * offset;

		OLED_C_MemorySize(offset, offset + width - 1, offset, offset + width - 1);

		DDRAM_access();
		for (int j = 0; j < width; ++j) {
			SpiDevice::Buffer buff;
			buff.resize(width * 2);
			for (size_t i = 0; i < buff.size(); i+=2) {
				buff[i] = 0xFF;
				buff[i+1] = 0xFF;
			}
			OLED_C_sendData(buff);
		}
		/*
		for(int j=0;j<9216;j++){
			OLED_C_Color(0xFF,0xFF);
		}
		*/
	}
	//delay_ms(1000);

	OLED_C_MemorySize(0x05,0x5A,0x05,0x5A);
	DDRAM_access();
	for(int j=0;j<8100;j++){
		OLED_C_Color(0x0C,0xC0);
	}
	//delay_ms(1000);

	OLED_C_MemorySize(0x0A,0x55,0x0A,0x55);
	DDRAM_access();
	for(int j=0;j<7225;j++){
		OLED_C_Color(0xFF,0x00);
	}
	//delay_ms(1000);

	OLED_C_MemorySize(0x0F,0x50,0x0F,0x50);
	DDRAM_access();
	for(int j=0;j<6400;j++){
		OLED_C_Color(0x80,0x00);
	}
	// delay_ms(1000);

	OLED_C_MemorySize(0x14,0x4B,0x14,0x4B);
	DDRAM_access();
	for(int j=0;j<5625;j++){
		OLED_C_Color(0xF8,0x00);
	}
	//delay_ms(1000);

	OLED_C_MemorySize(0x19,0x46,0x19,0x46);
	DDRAM_access();
	for(int j=0;j<4900;j++){
		OLED_C_Color(0x00,0xFF);
	}
	// delay_ms(1000);
	OLED_C_MemorySize(0x1E,0x41,0x1E,0x41);
	DDRAM_access();
	for(int j=0;j<4225;j++){
		OLED_C_Color(0x80,0xFF);
	}
	delay_ms(5000);
}

int main()
{
	initBB();
	//UART1_Init(56000);
	OLED_C_init();
	OLED_C_Background();

	while(1){
		for(int i=0; i<8; i++){
			OLED_C_sendCommand(SEPS114A_SCREEN_SAVER_MODE,i);
			OLED_C_sendCommand(SEPS114A_SCREEN_SAVER_CONTEROL,0x88);
			delay_ms(5000);
			OLED_C_sendCommand(SEPS114A_SCREEN_SAVER_CONTEROL,0x00);
		}
		delay_ms(5000);
	}
	return 0;
}
