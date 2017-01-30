#include "oledcdisplay.h"

//#include "images/elektroline.xpm"
#include "images/tram.xpm"

int main()
{
	OledCDisplay disp;
	disp.init(OledCDisplay::MikroBusSlot::Number2);
	//disp.drawBox(0, 0, 50, 60, OledCDisplay::Color::White);
	//disp.drawXpmImage(0, 0, elektroline_xpm);
	disp.drawXpmImage(0, 0, tram_xpm);
	//disp.drawText(20, 5, "BB & EL", OledCDisplay::Color::White, OledCDisplay::Color::fromRGB(0x18, 0x5c, 0x28));
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
