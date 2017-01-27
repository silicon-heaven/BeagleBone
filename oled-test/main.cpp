#include "oledcdisplay.h"

int main()
{
	OledCDisplay disp;
	disp.init(OledCDisplay::MikroBusSlot::Number2);
	//disp.drawBox(0, 0, 50, 60, OledCDisplay::Color::White);
	disp.drawText(0, 0, "Jaro");
	//disp.drawBox(10, 20, 50, 60, OledCDisplay::Color::Red);
#ifdef XPM_OUTPUT
	disp.writeXpmFile();
#else
	while(true)
		disp.demo();
#endif
	return 0;
}
