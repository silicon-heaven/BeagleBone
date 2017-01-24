#include "gpio.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

bool Gpio::checkExported() throw(std::runtime_error)
{
	if(dirExists("/sys/class/gpio/gpio" + std::to_string(m_number)))
		return false;
	writeFile("/sys/class/gpio/export", std::to_string(m_number));
	return true;
}

void Gpio::setDirection(Gpio::Direction direction) throw(std::runtime_error)
{
	std::string fn = "/sys/class/gpio/gpio" + std::to_string(m_number) + "/direction";
	writeFile(fn, direction == Direction::In? "in": "out");
}

void Gpio::setValue(bool val) throw(std::runtime_error)
{
	std::string fn = "/sys/class/gpio/gpio" + std::to_string(m_number) + "/value";
	writeFile(fn, val? "1": "0");
}

void Gpio::writeFile(const std::string &file_name, const std::string &str) throw(std::runtime_error)
{
	int fd = open(file_name.c_str(), O_WRONLY);
	if(fd < 0)
		throw std::runtime_error(std::string("cannot open '") + file_name + "' for write");
	int n = write(fd, str.c_str(), str.length());
	if(n < (int)str.length())
		throw std::runtime_error(std::string("cannot write to ") + file_name);
	close(fd);
}

bool Gpio::dirExists(const std::string &dir_name)
{
	struct stat sb;
	if (stat(dir_name.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
		return true;
	return false;
}
