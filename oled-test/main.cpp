#include <shv/oledcdisplay.h>
#include <shv/xpm.h>

namespace {
#include "images/elektroline.xpm"
#include "images/tram.xpm"
}

void delay_ms(int ms)
{
	struct timespec req;
	req.tv_sec = ms / 1000;
	req.tv_nsec = (ms % 1000) * 1000000L;
	nanosleep(&req, (struct timespec *)NULL);
}

int main()
{
	shv::OledCDisplay disp;
	disp.init(shv::OledCDisplay::MikroBusSlot::Number2, shv::OledCDisplay::OutputType::OLED);
	disp.drawXpmImage(shv::Xpm::fromWrappedData(elektroline_xpm));
	disp.drawText(5, 5, "Ele", shv::Xpm::Color::White, shv::Xpm::Color::fromRGB(0x18, 0x5c, 0x28));
	//disp.drawText(0, 10, "123456789012", OledCDisplay::Color::White, OledCDisplay::Color::fromRGB(0x18, 0x5c, 0x28));
	//delay_ms(5000);
	//disp.drawBox(0, 0, OledCDisplay::OLED_C_SIZE, OledCDisplay::OLED_C_SIZE, OledCDisplay::Color::White);
	disp.drawXpmImage(shv::Xpm::fromWrappedData(tram_xpm));
	//disp.drawBox(0, 0, 50, 60, OledCDisplay::Color::Red);
	//disp.drawBox(10, 10, 50, 60, OledCDisplay::Color::Green);
	//disp.drawBox(20, 20, 50, 60, OledCDisplay::Color::Blue);
#ifdef XPM_OUTPUT
	disp.writeXpmFile();
#else
	//while(true)
	//	disp.demo();
#endif
	return 0;
}
