#include "spidevice.h"

#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <iomanip>
#include <vector>
#include <cstring>

namespace shv {

SpiDevice::SpiDevice(bool debug)
	: m_debug(debug)
{
}

void SpiDevice::open(const char *device_name, uint8_t mode, uint32_t speed, uint8_t bits, uint16_t delay_usec) throw(std::runtime_error)
{
	close();
	if(m_debug) {
		std::cerr << "opening device: " << device_name  << std::endl;
	}
	m_fd = ::open(device_name, O_RDWR);
	if (m_fd < 0) {
		perror("can't open device");
		throw std::runtime_error(std::string("cannot open '") + device_name + "' for write");
	}
	//spi mode
	int ret = ioctl(m_fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1) {
		perror("can't set spi mode");
		throw std::runtime_error(std::string("can't set spi mode to: ") + std::to_string(mode));
	}
	ret = ioctl(m_fd, SPI_IOC_RD_MODE, &m_mode);
	if (ret == -1) {
		perror("can't get spi mode");
		throw std::runtime_error(std::string("can't get spi mode"));
	}

	// bits per word
	ret = ioctl(m_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1) {
		perror("can't set bits per word");
		throw std::runtime_error(std::string("can't set bits per word to: ") + std::to_string(bits));
	}
	ret = ioctl(m_fd, SPI_IOC_RD_BITS_PER_WORD, &m_bits);
	if (ret == -1) {
		perror("can't get bits per word");
		throw std::runtime_error(std::string("can't get bits per word"));
	}

	// max speed hz
	ret = ioctl(m_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1) {
		perror("can't set max speed hz");
		throw std::runtime_error(std::string("can't set set max speed to: ") + std::to_string(bits) + " Hz");
	}
	ret = ioctl(m_fd, SPI_IOC_RD_MAX_SPEED_HZ, &m_speed);
	if (ret == -1) {
		perror("can't get max speed hz");
		throw std::runtime_error(std::string("can't get set max speed"));
	}
	m_delay = delay_usec;
	if(m_debug) {
		std::cerr << "device: " << device_name  << std::endl;
		std::cerr << '\t' << "spi mode: " << (int)m_mode << std::endl;
		std::cerr << '\t' << "bits per word: " << (int)m_bits << std::endl;
		std::cerr << '\t' << "max speed [Hz]: " << (int)m_speed << std::endl;
		std::cerr << '\t' << "delay [usec]: " << (int)m_delay << std::endl;
	}
}

void SpiDevice::close()
{
	if(m_fd > 0) {
		if(m_debug) {
			std::cerr << "closing SPI device: " << m_fd << std::endl;
		}
		::close(m_fd);
		m_fd = 0;
	}
}

static void dump_buffer(const SpiDevice::Buffer &buffer)
{
	for (size_t i = 0; i < buffer.size(); ++i) {
		if (i > 0 && (i % 16) == 0)
			fprintf(stderr, "\n");
		fprintf(stderr, "%02x ", buffer[i]);
	}
	fprintf(stderr, "\n");
}

void SpiDevice::transfer(const Buffer &tx_data, Buffer *p_rx_data) throw(std::runtime_error)
{
	if(m_debug) {
		std::cerr << "sending:\n";
		dump_buffer(tx_data);
	}

	if(p_rx_data)
		p_rx_data->resize(tx_data.size());

	struct spi_ioc_transfer tr;
	::memset(&tr, 0, sizeof(tr));
	tr.delay_usecs = m_delay;
	tr.speed_hz = m_speed;
	tr.bits_per_word = m_bits;

	size_t max_spi_packet_size = 1024 * 2 * 2;
	size_t n_sent = 0;
	while(n_sent < tx_data.size()) {
		size_t len = tx_data.size() - n_sent;
		if(len > max_spi_packet_size)
			len = max_spi_packet_size;
		tr.tx_buf = (unsigned long)(tx_data.data() + n_sent);
		tr.rx_buf = (unsigned long)(p_rx_data? p_rx_data->data() + n_sent: nullptr);
		tr.len = len;
		if(m_debug)
			std::cerr << "sending " << len << " bytes of data" << std::endl;
		int ret = ioctl(m_fd, SPI_IOC_MESSAGE(1), &tr);
		if (ret < 1) {
			perror("can't send spi message");
			throw std::runtime_error(std::string("can't make SPI transfer"));
		}
		n_sent += len;
	}
	if(m_debug) {
		if(p_rx_data) {
			std::cerr << "received:\n";
			dump_buffer(*p_rx_data);
		}
	}
}

}
