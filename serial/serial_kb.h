//============================================================================
// Name        : sckComm.h
// Author      : dschio
// Version     :
// Date		   : 07/21/18
// Copyright   : 2018
// Description : classes / functions for dealing with the serial port.
//				 This reads data from the com port byte by byte.
//				 As each byte comes in, it gets sent up to be decoded in
//			     the overriden level 2 decode module
//============================================================================

#ifndef SERIAL_KB_H_
#define SERIAL_KB_H_

#include <fcntl.h>    /* For O_RDWR */

#define MAX_BUFF_SIZE	1024

// generic serial port class

class SerialComm
{
public:
	SerialComm() :
		m_CommFd( -1 )
	{}

	SerialComm( const char * port ) :
		m_CommFd(-1)
	{
		// open the comm port
		m_CommFd = open(port, O_RDWR);
	}

	virtual ~SerialComm() {}

	void initPort( int baud, bool rs485 = true );
	int GetFd() { return m_CommFd; }
	int enableRead( bool enable = true );
	int write( char * buff, int count );



private:
	int m_CommFd;

};


#endif /* SERIAL_KB_H_ */
