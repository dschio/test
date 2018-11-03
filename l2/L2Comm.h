/*
 * L2Comm.h
 *
 *  Created on: Jul 21, 2018
 *      Author: ds
 */

#ifndef L2COMM_H_
#define L2COMM_H_

#include "serial_kb.h"
#include "mqtt_messaging.h"


#define COM_PORT		"/dev/ttymxc1"
#define VC210_BAUD		B57600


#define NEW_SCK_HEADER_BYTE			0x02
#define SIZE_OF_NEW_INPUT_BUFFER    384
#define DATA_SIZE					SIZE_OF_NEW_INPUT_BUFFER

#pragma pack(1)

typedef enum
{
	RX_STATE_IDLE = 0,                      		         // initial recive state
	RX_STATE_READ_HEADER,
	RX_STATE_READ_CS,
	RX_STATE_READ_LENGTH_L,
	RX_STATE_READ_LENGTH_H,
	RX_STATE_READ_COMMAND_L,
	RX_STATE_READ_COMMAND_H,
	RX_STATE_READ_PAYLOAD,
	RX_STATE_READ_CRC_L_BYTE,
	RX_STATE_READ_CRC_H_BYTE,
	RX_STATE_READ_TRAILER,
	RX_STATE_SUCCESS,
	RX_STATE_FAIL,
} SCK_RECIEVE_STATES;

typedef union
{
	uint16_t integerData;
	uint8_t byteData[2];
} SCK_SPLIT_INT;

void SCKCalculate16bitCRC( uint16_t octet, uint16_t * crcValue );


// definition for packet: this is the packet format after the header, crc, and trailer are stripped
//
//		C/S
//		Length			- the length of the rest of this packet (CMD/STATUS bytes + the payload)
//		CMD / STATUS	- as is seems
//		PAYLOAD[]		- data that is attached to the above CMD / STATUS message
//
class L2_PACKET
{
public:
	void SetL2CS( uint8_t cs ) 					{ m_L2Msg.m_cs = cs; }
	uint8_t GetL2CS()							{ return m_L2Msg.m_cs;}

	void SetL2Data( int indx, uint8_t d ) 		{ m_L2Msg.m_paylod[indx] = d; }
	void SetL2Length( int l ) 					{ m_L2Msg.m_length = l; }

	uint8_t * GetL2Msg() 						{ return  (uint8_t *)&m_L2Msg; }
	uint16_t GetL2TotalLength()					{ return m_L2Msg.m_length + sizeof(L2_MESSAGE::m_length) + sizeof(L2_MESSAGE::m_cs); }

	uint16_t GetL2Cmd()							{ return m_L2Msg.m_cmd; }
	void SetL2Cmd( uint16_t cmd )				{ m_L2Msg.m_cmd = cmd; }
	uint8_t * GetPayloadPtr()					{ return m_L2Msg.m_paylod; }

	void SetPayload(int index, uint8_t data )	{ m_L2Msg.m_paylod[index] = data; }



private:
	// the first part of the packet.
	typedef struct _L2_HDR_
	{
		uint8_t  m_cs;
		uint16_t m_length;
		uint16_t m_cmd;
		uint8_t m_paylod[DATA_SIZE];
	} L2_MESSAGE;

	L2_MESSAGE			 	m_L2Msg;
};

// this class is used to receive and decode the data on the wire.
class RECV_PACKET
{
public:
	RECV_PACKET() {}


	SCK_RECIEVE_STATES m_recieveState;
	uint16_t m_toRead;
	uint16_t m_haveRead;
	uint16_t m_crc;
	uint16_t m_crcValue;
	uint16_t m_dataLength;
};


// this is the mqtt interface for publishing the L3 packets to the L2 and comm port
class L2CommMessage : public mqtt_message
{
public:
	L2CommMessage( char *id, SerialComm * comPort ) : mqtt_message( id )
	{
		m_comPort = comPort;
		setTopic((char *) MSG_L2_TOPIC);
		connect((char *) LOCAL_BROKER_IP, 1883 );
	}

	virtual void on_message( const struct mosquitto_message *message )
	{
		L2_PACKET * rcvPkt = (L2_PACKET *) message->payload;
		processPacket( rcvPkt );
	};

private:
	void processPacket( L2_PACKET * packet );
	SerialComm * m_comPort;
};

// this class wraps the serial port running FASTLAN,
// and the messaging to / from the next layer up over MQTT
class L2Comm
{
public:
	L2Comm( const char * port, int baud )
	{
		m_comPort = new SerialComm( port );
		if( m_comPort )
			m_comPort->initPort( baud );
	}

	virtual ~L2Comm()
	{
	}

	void startVC210Comms()
	{
		// start the thread to receive data from the comm port
		pthread_create(&VC210CommsThread_id, NULL, &VC210CommsThread, (void *) this);

		// the mqtt recevier that subscribes to messages from the upper layer (i.e. the L3 messages)
		m_rcvMsg = new L2CommMessage( (char *)"vc210L2Comm", m_comPort );

		// in this case, we will handle the receipt of memssages from the L3 directly in the message processing loop (on_message from the subscription)
		m_rcvMsg->loop_start();
	}


protected:
	static void * VC210CommsThread( void * t );


private:
	// this function will get every byte that comes in on the comm port.  It is expected
	// that it will do the frame-level decode; i.e. the FASTLAN decode in this case.
	void recvCommRecieveL2Decode(char * inCharBuff, int count);

	pthread_t VC210CommsThread_id;
	SerialComm * m_comPort;

	// mqtt_message recvMsg;
	L2CommMessage * m_rcvMsg;
};

#pragma pack()
#endif /* L2COMM_H_ */
