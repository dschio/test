//============================================================================
// Name        : sckComm.cpp
// Author      : dschio
// Version     :
// Date		   : 07/21/18
// Copyright   : 2018
// Description : classes / functions for dealing with the serial port.
//				 This reads data from the com port byte by byte.
//				 As each byte comes in, it gets sent up to be decoded in
//			     the overriden level 2 decode module
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "serial_kb.h"


// generic serial port class

// initialize the port
void SerialComm::initPort( int baud, bool rs485 )
{
	struct serial_rs485 rs485conf;
	struct termios settings;

	tcgetattr(m_CommFd, &settings);

	cfsetospeed(&settings, baud);
	cfmakeraw(&settings);

	//settings.c_cflag &= ~CREAD;


	tcflush(m_CommFd, TCIFLUSH);
	tcsetattr(m_CommFd, TCSANOW, &settings);

	if( rs485 )
	{
		/* Enable RS485 mode: */
		rs485conf.flags |= SER_RS485_ENABLED | SER_RS485_RTS_ON_SEND;
		rs485conf.flags &= ~SER_RS485_RX_DURING_TX;

		if( ioctl(m_CommFd, TIOCSRS485, &rs485conf) < 0 )
		{
			/* Error handling. See errno. */
			printf("error\n");
		}
	}
}

int SerialComm::enableRead( bool enable )
{
	struct termios settings;
	int ret;

	tcgetattr(m_CommFd, &settings);
	//tcdrain(m_CommFd);

	if( enable )
	{
		settings.c_cflag |= CREAD;
	}
	else
		settings.c_cflag &= ~CREAD;

	//ret = tcsetattr(m_CommFd, TCSAFLUSH, &settings);
	ret = tcsetattr(m_CommFd, TCSANOW, &settings);

	//tcflush(m_CommFd, TCIOFLUSH);

	return ret;
}

int SerialComm::write( char * buff, int count )
{
	unsigned long lsr;
	int ret;
	int i=0;

	// before we write, turn off the receiver
	enableRead(false);

	// then write it
	ret = ::write( GetFd(), buff, count );

	// before we re-enable the receiver, wait enough time for the
	// data to be sent out of the transmitter.
	// at 57600, this is about 175 usec per byte.
	int delay = count * 170;
	usleep(delay);

	// after the initial delay, wait to see that the transmitter is empty
	do {
		ioctl (GetFd(), TIOCSERGETLSR, &lsr);
		i++;
	} while(!(lsr & TIOCSER_TEMT));

	// then enable the receiver again
	enableRead(true);

	return ret;
}
