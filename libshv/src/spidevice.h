#ifndef SHV_SPIDEVICE_H
#define SHV_SPIDEVICE_H

#include "shvglobal.h"

#include <stdexcept>
#include <vector>

namespace shv {

class SHV_DECL_EXPORT SpiDevice
{
public:
	using Buffer = std::vector<uint8_t>;
public:
	SpiDevice(bool debug = false);

	void open(const char *device_name, uint8_t mode, uint32_t speed = 500000, uint8_t bits = 8, uint16_t delay_usec = 0) throw(std::runtime_error);
	void close();
	void transfer(const Buffer &tx_data, Buffer *p_rx_data = nullptr) throw(std::runtime_error);
private:
	bool m_debug = false;
	int m_fd = -1;
	uint8_t m_mode = 0;
	uint32_t m_speed = 0;
	uint8_t m_bits = 0;
	uint16_t m_delay = 0; //< usec
};

}

#endif // SPIDEVICE_H
