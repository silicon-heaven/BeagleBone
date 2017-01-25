#include "oledcdisplay.h"

int main()
{
	OledCDisplay disp;
	disp.init(OledCDisplay::MikroBusSlot::Number2);
	disp.demo();
	return 0;
}
