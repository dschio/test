/*
 * L2Comm.cpp
 *
 *  Created on: Jul 21, 2018
 *      Author: ds
 */




//============================================================================
// Name        : sckComm.cpp
// Author      : dschio
// Version     :
// Date		   : 07/21/18
// Copyright   : 2018
// Description : classes / functions for dealing with the VC210 interface
//============================================================================
#include <unistd.h>
#include "L2Comm.h"


static void debugPrint( uint8_t * recvdPacket, uint8_t haveRead )
{
	printf( "dbg: (read %d)  ", haveRead );
	for( int i = 0;i < haveRead;i++ )
		printf("%02x ", recvdPacket[i]);
	printf("\n");
}

void SCKCalculate16bitCRC( uint16_t octet, uint16_t * crcValue )
{
	uint16_t crcLow;

	crcLow = (*crcValue & 0xff) ^ (octet & 0xff);
	*crcValue = (*crcValue >> 8) ^ (crcLow << 8) ^ (crcLow << 3) ^ (crcLow << 12) ^ (crcLow >> 4) ^ (crcLow & 0x0f) ^ ((crcLow & 0x0f) << 7);
}

void L2Comm::recvCommRecieveL2Decode(char * inCharBuff, int count)
{
	uint8_t b;
	static RECV_PACKET rcvPkt;
	static L2_PACKET l2_pkt;
	static uint8_t * dataPtr;


	if( rcvPkt.m_haveRead >= MAX_BUFF_SIZE )
	{
		rcvPkt.m_haveRead = rcvPkt.m_toRead = 0;
		rcvPkt.m_recieveState = RX_STATE_IDLE;
	}

	for( int i = 0;i < count;i++ )
	{

		b = inCharBuff[i];

		//printf("(%d) %d: %02x \n", count, rcvPkt.m_recieveState, b);

		switch( rcvPkt.m_recieveState )
		{
		case RX_STATE_IDLE:
		case RX_STATE_SUCCESS:
			rcvPkt.m_haveRead = rcvPkt.m_toRead = 0;

			//
			// Already have a parsed packet, can't start parsing again until
			// somebody resets us.
			if( b != NEW_SCK_HEADER_BYTE )
			{
				rcvPkt.m_recieveState = RX_STATE_IDLE;
				break;
			}
			// fall thru if this is a header byte
		case RX_STATE_READ_HEADER:
			if( b == NEW_SCK_HEADER_BYTE )
			{
				rcvPkt.m_recieveState = RX_STATE_READ_CS;
			}
			else
			{
				rcvPkt.m_recieveState = RX_STATE_IDLE;
			}
			break;

		case RX_STATE_READ_CS:
			if( b == 'C' )
			{
				l2_pkt.SetL2CS( 'C' );
			}
			else if( b == 'S' )
			{
				l2_pkt.SetL2CS( 'S' );
			}
			else
			{
				rcvPkt.m_recieveState = RX_STATE_IDLE;
				break;
			}

			rcvPkt.m_recieveState = RX_STATE_READ_LENGTH_L;
			rcvPkt.m_crcValue = 0xffff;
			SCKCalculate16bitCRC(b, &rcvPkt.m_crcValue);
			break;

		case RX_STATE_READ_LENGTH_L:
			rcvPkt.m_recieveState = RX_STATE_READ_LENGTH_H;
			rcvPkt.m_dataLength = b;
			SCKCalculate16bitCRC(b, &rcvPkt.m_crcValue);
			break;

		case RX_STATE_READ_LENGTH_H:
			rcvPkt.m_dataLength += (unsigned int) b << 8;
			rcvPkt.m_toRead = (uint16_t) rcvPkt.m_dataLength;
			rcvPkt.m_haveRead = 0;
			rcvPkt.m_dataLength = 0;
			rcvPkt.m_recieveState = RX_STATE_READ_COMMAND_L;
			SCKCalculate16bitCRC(b, &rcvPkt.m_crcValue);
			break;

		case RX_STATE_READ_COMMAND_L:
			SCKCalculate16bitCRC(b, &rcvPkt.m_crcValue);
			rcvPkt.m_haveRead++;
			l2_pkt.SetL2Cmd( (uint16_t) b );
			if( --rcvPkt.m_toRead == 0 )
			{
				rcvPkt.m_recieveState = RX_STATE_IDLE;
			}
			else
			{
				rcvPkt.m_recieveState = RX_STATE_READ_COMMAND_H;
			}
			break;

		case RX_STATE_READ_COMMAND_H:
			SCKCalculate16bitCRC(b, &rcvPkt.m_crcValue);
			rcvPkt.m_haveRead++;
			l2_pkt.SetL2Cmd( l2_pkt.GetL2Cmd() | ((uint16_t) b << 8) );
			if( --rcvPkt.m_toRead == 0 )
			{
				rcvPkt.m_recieveState = RX_STATE_READ_CRC_L_BYTE;
			}
			else
			{
				dataPtr = l2_pkt.GetPayloadPtr();
				rcvPkt.m_recieveState = RX_STATE_READ_PAYLOAD;
			}
			break;

		case RX_STATE_READ_PAYLOAD:
			SCKCalculate16bitCRC(b, &rcvPkt.m_crcValue);
			*(dataPtr++) = b;
			rcvPkt.m_haveRead++;
			if( --rcvPkt.m_toRead == 0 )
			{
				rcvPkt.m_recieveState = RX_STATE_READ_CRC_L_BYTE;
			}
			break;

		case RX_STATE_READ_CRC_L_BYTE:
			rcvPkt.m_recieveState = RX_STATE_READ_CRC_H_BYTE;
			rcvPkt.m_crc = b;
			break;

		case RX_STATE_READ_CRC_H_BYTE:
			rcvPkt.m_crc += ((unsigned int) b << 8);
			if( rcvPkt.m_crc == rcvPkt.m_crcValue )
				// ok
				rcvPkt.m_recieveState = RX_STATE_READ_TRAILER;
			else
			{
				rcvPkt.m_recieveState = RX_STATE_IDLE;
				printf("RECIEVE PACKET FAIL CRC!\n");
			}
			break;

		case RX_STATE_READ_TRAILER:
		{
			if( b == 0x03 )
			{
				// good
				rcvPkt.m_recieveState = RX_STATE_SUCCESS;

				l2_pkt.SetL2Length( rcvPkt.m_haveRead );

				if( m_rcvMsg )
				{
					//debugPrint( (uint8_t *)l2_pkt.GetL2Msg(),l2_pkt.GetL2TotalLength());

					// this sends up the entire packet between the STX and the CRC (not including those)
					int snd = m_rcvMsg->publish( (char *)MSG_L3_From_L2, l2_pkt.GetL2TotalLength(), (const char *)l2_pkt.GetL2Msg(),  QOS_0) ;
					if( snd != 0 )
						printf("mqtt_send error=%i\n", snd);
				}
			}
			else
			{
				// bad
				rcvPkt.m_recieveState = RX_STATE_IDLE;
				printf("RECIEVE PACKET FAIL TRAIL!\n");
			}
		}
			break;

		default:
			rcvPkt.m_recieveState = RX_STATE_IDLE;
			printf("RECIEVE PACKET ERROR!\n");
			break; // should not happen.
		}
	}

}

/////
void L2CommMessage::processPacket( L2_PACKET * msg )
{
	// the length of the message to send including the C/S and Length bytes
	int len = msg->GetL2TotalLength();

	// a pointer to the message, starting with the C/S byte
	uint8_t * tmpMsg = msg->GetL2Msg();


	//uint8_t * tmpMsg = (uint8_t *)msg->GetL2Data();
	// a buffer to build the outgoing packet in.  this size now includes the STX/CRC/ETX bytes
	uint8_t * buff = new uint8_t [len + 4];

	uint16_t crc;
	crc = 0xffff;

	int offset = 0;
	buff[offset++] = 0x02;		// stx

	for( int i=0; i< len; i++ )
	{
		uint8_t b = * tmpMsg++;
		SCKCalculate16bitCRC( (uint16_t) b, & crc );
		buff[offset++] = b;
	}

	buff[offset++] = crc & 0xff;
	buff[offset++] = (crc >> 8) & 0xff;
	buff[offset++] = 0x03;		// etx

	// transmit it
	m_comPort->write( (char *)buff, offset );

	delete [] buff;
}

// this (static) function is started in it's own thread.
// it's purpose is to read / write to the port in an asynchronous manner.
// upon receiving one or more characters from the comm port, it calls into a
// class specific L2 decode function to deal with it.
void * L2Comm::VC210CommsThread( void * t )
{
	L2Comm * th = (L2Comm *) t;
	int commFd = th->m_comPort->GetFd();
	static char recvBuffer[MAX_BUFF_SIZE];

	fd_set rfds, afds;
	struct timeval tv;
	int retval;

	FD_ZERO(&rfds);
	FD_SET(commFd, &rfds);

	while( true )
	{
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		afds = rfds;

		retval = select(commFd + 1, &afds, NULL, NULL, &tv);

		if( retval == -1 )
			perror("select()");

		else if( retval )
		{
			if( FD_ISSET(commFd, &afds) )
			{
				retval = read(commFd, recvBuffer, MAX_BUFF_SIZE);
				th->recvCommRecieveL2Decode(recvBuffer, retval);
				//printf("got %d chars\n", retval);
			}
		}

		else
			printf("No data within five seconds.\n");

	}

	return NULL;
}






