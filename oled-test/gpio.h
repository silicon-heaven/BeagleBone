#ifndef GPIO_H
#define GPIO_H

#include <stdexcept>

class Gpio
{
public:
	enum class Direction {In, Out};

	Gpio() {}
	Gpio(int gpio_number) {setNumber(gpio_number);}
	Gpio(int gpio_number1, int gpio_number2) {setNumber(gpio_number1, gpio_number2);}

	void setNumber(int gpio_number) {m_number = gpio_number;}
	void setNumber(int gpio_number1, int gpio_number2) {m_number = gpio_number1 * 32 + gpio_number2;}
	bool checkExported() throw(std::runtime_error);
	void setDirection(Direction direction) throw(std::runtime_error);
	void setValue(bool val) throw(std::runtime_error);
private:
	void writeFile(const std::string &file_name, const std::string &str) throw(std::runtime_error);
	bool dirExists(const std::string &dir_name);
private:
	int m_number = -1;
};

#endif // GPIO_H
