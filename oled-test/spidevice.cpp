#include "spidevice.h"

#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <iomanip>
#include <vector>
#include <cstring>

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

void SpiDevice::transfer(const Buffer &tx_data, Buffer *p_rx_data) throw(std::runtime_error)
{
	Buffer tmp_rx_data;
	Buffer &rx_data = p_rx_data? *p_rx_data: tmp_rx_data;
	rx_data.resize(tx_data.size());

	struct spi_ioc_transfer tr;
	::memset(&tr, 0, sizeof(tr));
	tr.tx_buf = (unsigned long)tx_data.data();
	tr.rx_buf = (unsigned long)rx_data.data();
	tr.len = tx_data.size();
	tr.delay_usecs = m_delay;
	tr.speed_hz = m_speed;
	tr.bits_per_word = m_bits;

	if(m_debug) {
		std::cerr << "sending: " << std::setw(2) << std::hex << std::setfill('0');
		for (size_t i = 0; i < tx_data.size(); ++i) {
			std::cerr << ' ' << (int)(((uint8_t*)tr.tx_buf)[i]);
		}
		std::cerr << std::endl;
	}

	int ret = ioctl(m_fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) {
		perror("can't send spi message");
		throw std::runtime_error(std::string("can't make SPI transfer"));
	}

	if(m_debug) {
		std::cerr << "received: " << std::setw(2) << std::hex << std::setfill('0');
		for (size_t i = 0; i < rx_data.size(); ++i) {
			std::cerr << ' ' << (int)(((uint8_t*)tr.rx_buf)[i]);
		}
		std::cerr << std::endl;
	}
}
