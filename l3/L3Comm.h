/*
 * L3Comm.h
 *
 *  Created on: Jul 21, 2018
 *      Author: ds
 */

#ifndef L3COMM_H_
#define L3COMM_H_

#include <netinet/in.h>
#include "mqtt_messaging.h"
#include "L2Comm.h"
#include "L3CommDefs.h"
#include "L3CommEvents.h"



#pragma pack(1)


class FASTLAN_MSG
{
public:
	uint16_t m_tid;
	uint8_t m_rw;
	uint16_t m_oid;
	uint8_t m_dataType;
	uint16_t m_offset;
	uint16_t m_count;
};

class FASTLAN_RESPONSE_MSG
{
public:
	FASTLAN_RESPONSE_MSG() {};
	FASTLAN_RESPONSE_MSG( void * inBuf )
	{
		FASTLAN_RESPONSE_MSG * in = static_cast<FASTLAN_RESPONSE_MSG *>(inBuf);
		m_msg.m_tid = ( in->m_msg.m_tid );
		m_msg.m_rw = in->m_msg.m_rw;
		m_msg.m_oid = ( in->m_msg.m_oid );
		m_msg.m_dataType = in->m_msg.m_dataType;
		m_msg.m_offset = ( in->m_msg.m_offset );
		m_msg.m_count = ( in->m_msg.m_count );
	};
	~FASTLAN_RESPONSE_MSG() {};

	uint16_t GetTID() 		{ return htons( m_msg.m_tid ); }
	uint8_t GetRW() 		{ return m_msg.m_rw; }
	uint16_t GetOID() 		{ return htons( m_msg.m_oid ); }
	uint8_t GetType() 		{ return m_msg.m_dataType; }
	uint16_t GetOffset() 	{ return htons( m_msg.m_offset ); }
	uint16_t GetCount() 	{ return htons( m_msg.m_count ); }
	uint8_t * GetData()		{ return m_data; }

	void SetTID(uint16_t v) 	{ m_msg.m_tid = v; }
	void SetRW(uint8_t v) 		{ m_msg.m_rw = v; }
	void SetOID(uint16_t v) 	{ m_msg.m_oid = v; }
	void SetType(uint8_t v) 	{ m_msg.m_dataType = v; }
	void SetOffset(uint16_t v) 	{ m_msg.m_offset = v; }
	void SetCount(uint16_t v) 	{ m_msg.m_count = v; }

	int GetDataLength()
	{
		int length = 0;
		switch( m_msg.m_dataType )
		{
			case DATATYPE_STRING:
			case DATATYPE_UBYTE:
			case DATATYPE_BYTE:
				length = GetCount();
				break;

			case DATATYPE_UINTEGER:
			case DATATYPE_INTEGER:
				length = (2 * GetCount());
				break;

			case DATATYPE_LONG:
			case DATATYPE_ULONG:
			case DATATYPE_FLOAT:
			case DATATYPE_SFLOAT:
				length = (4 * GetCount());
				break;

			default:
				length = 0;
				break;
		}
		return length;
	}

protected:
	FASTLAN_MSG m_msg;
	uint8_t  m_data[0];

};

class FASTLAN_COMMAND_MSG
{
public:
	FASTLAN_COMMAND_MSG() {}
	FASTLAN_COMMAND_MSG( uint16_t oid, uint16_t offset, uint16_t count, uint8_t type )
	{
		SetOID(oid);
		SetOffset(offset);
		SetCount(count);
		SetType(type);
		SetRW( 0 ); // read
		SetTID( nextTid++ );
	}

	~FASTLAN_COMMAND_MSG() {}

	uint16_t GetTID() 		{ return htons( m_msg.m_tid ); }
	uint8_t GetRW() 		{ return m_msg.m_rw; }
	uint16_t GetOID() 		{ return htons( m_msg.m_oid ); }
	uint8_t GetType() 		{ return m_msg.m_dataType; }
	uint16_t GetOffset() 	{ return htons( m_msg.m_offset ); }
	uint16_t GetCount() 	{ return htons( m_msg.m_count ); }

	void SetTID(uint16_t v) 	{ m_msg.m_tid = htons( v ); }
	void SetRW(uint8_t v) 		{ m_msg.m_rw = v; }
	void SetOID(uint16_t v) 	{ m_msg.m_oid = htons( v ); }
	void SetType(uint8_t v) 	{ m_msg.m_dataType = v; }
	void SetOffset(uint16_t v) 	{ m_msg.m_offset = htons( v ); }
	void SetCount(uint16_t v) 	{ m_msg.m_count = htons( v ); }

	int GetDataLength()
	{
		int length = 0;
		switch( m_msg.m_dataType )
		{
			case DATATYPE_STRING:
			case DATATYPE_UBYTE:
			case DATATYPE_BYTE:
				length = GetCount();
				break;

			case DATATYPE_UINTEGER:
			case DATATYPE_INTEGER:
				length = (2 * GetCount());
				break;

			case DATATYPE_LONG:
			case DATATYPE_ULONG:
			case DATATYPE_FLOAT:
			case DATATYPE_SFLOAT:
				length = (4 * GetCount());
				break;

			default:
				length = 0;
				break;
		}
		return length;
	}

protected:
	FASTLAN_MSG m_msg;
	static uint16_t nextTid;
};

class FASTLAN_COMMAND_MSG_FLASH_NUMBER_MSG : public FASTLAN_COMMAND_MSG
{
public:
	FASTLAN_COMMAND_MSG_FLASH_NUMBER_MSG() :
		FASTLAN_COMMAND_MSG(  RESPONSE_TABLE::FLASH_NUMBER, 0, 5, 1 )
	{
	}
	~FASTLAN_COMMAND_MSG_FLASH_NUMBER_MSG() {}
};

class FASTLAN_COMMAND_MSG_SOFTWARE_NUMBER_MSG : public FASTLAN_COMMAND_MSG
{
public:
	FASTLAN_COMMAND_MSG_SOFTWARE_NUMBER_MSG() :
		FASTLAN_COMMAND_MSG(  RESPONSE_TABLE::SOFTWARE_NUMBER, 0, 5, 1 )
	{
	}
	~FASTLAN_COMMAND_MSG_SOFTWARE_NUMBER_MSG() {}
};

class FASTLAN_COMMAND_MSG_DOWNLOAD_NUMBER_MSG : public FASTLAN_COMMAND_MSG
{
public:
	FASTLAN_COMMAND_MSG_DOWNLOAD_NUMBER_MSG() :
		FASTLAN_COMMAND_MSG(  RESPONSE_TABLE::DOWNLOAD_NUMBER, 0, 5, 1 )
	{
	}
	~FASTLAN_COMMAND_MSG_DOWNLOAD_NUMBER_MSG() {}
};

//class FASTLAN_COMMAND_MSG_PROTOCOL_VER_MSG : public FASTLAN_COMMAND_MSG
//{
//public:
//	FASTLAN_COMMAND_MSG_PROTOCOL_VER_MSG() :
//		FASTLAN_COMMAND_MSG(  RESPONSE_TABLE::FLASH_NUMBER, 0, 5, 1 )
//	{
//	}
//	~FASTLAN_COMMAND_MSG_PROTOCOL_VER_MSG() {}
//};


class L3Message: public mqtt_message
{
public:
	L3Message( char *id ) :
			mqtt_message(id)
	{
		setTopic((char *) MSG_L3_TOPIC);
		connect((char *) LOCAL_BROKER_IP, 1883);
	}

	// process the message received from the MQTT service
	virtual void on_message( const struct mosquitto_message *message )
	{
		if( message == NULL )
			return;

		if( message->payload == NULL )
			return;

		printf("message from topic %s, %d\n", message->topic, message->mid );

		// process depending on the topic
		if( strncmp( message->topic, MSG_L3_From_L2, 32) == 0 )
		{
			L2_PACKET * rcvPkt = (L2_PACKET *) message->payload;
			processL2Packet( rcvPkt );
			return;
		}

		if( strncmp( message->topic, MSG_L3_From_L4, 32) == 0 )
		{
			FASTLAN_COMMAND_MSG * rcvPkt = (FASTLAN_COMMAND_MSG *) message->payload;
			processL4Packet( rcvPkt );
			return;
		}

		printf( "WRONG MESSAGE FROM on_message: %s\n", message->topic );
	}


private:
	// the processPacket funcion will eventually call one or more of these, depending on the message received.
	//
	//	called upon the receipt of an event message from the VC210
	void SCKEventCallbackFunction( L2_PACKET * packet );
	bool SCKEventCheckIncomingEventIsSupported( EVENT_TABLE::EVENT_OIDS oid );
	void SCKEventHandleEvent( COMMS_EVENT * msg );

	// the function to generate a simple ack to a command from the VC210
	void SimpleAck( void );

	// called upon receipt of an M fastlan command packet from the VC210 for a specific OID
#ifdef ADD_FASTLAN_COMMANDS
	void SCKReadWriteFastlanCallbackFunction( L2_PACKET * packet );
	void SCKGenerateFastlanReadResponse( FASTLAN_RESPONSE_MSG & messageBufferPtr );
	SCK_UNSIGNED_8BIT SCKIncomingRequestReadEnableDisableSecurity( SCK_UNSIGNED_16BIT offset );
	SCK_UNSIGNED_8BIT SCKIncomingRequestWriteEnableDisableSecurity( SCK_UNSIGNED_16BIT offset );
#endif

	// called upon receipt of a "new" (non M packet) command packet from the VC210
	void SCKProcessReadWriteCommand( L2_PACKET * packet );

	// called upon receipt of a fastlan response packet from the VC210
	void SCKProcessFastlan( L2_PACKET * packet );
	void SCKProcessSetpointStatus( L2_PACKET * packet );

	// this sends a message down / up the stack to send out on the wire or cloud
	void Write( L2_PACKET * msg, const char * topic, int qos = QOS_0 );
	void Write( COMMS_EVENT * msg, const char * topic, int qos = QOS_0 );
	void Write( FASTLAN_RESPONSE_MSG * msg, int length, const char * topic, int qos = QOS_0 );


	// the starting point for receiving data packets from the lower level
	void processL2Packet( L2_PACKET * packet );

	// the starting point for receiving data packets from the upper level
	// at the moment, I am assuming a fastlan type message to a specific oid
	void processL4Packet( FASTLAN_COMMAND_MSG * packet );


private:

};

// this class wraps the FASTLAN L3 decoder,
// and the messaging to / from the next layer down over MQTT
class L3Comm
{
public:
	L3Comm()
	{

	}

	virtual ~L3Comm()
	{

	}

	void startL3Comms()
	{
		// start up the mqtt messaging loop ...

		// the mqtt recevier that subscribes to messages from the lower layer (i.e. the L2 messages)
		L3Message * m_rcvMsg = new L3Message((char *) "L3Comm");

		// in this case, we will handle the receipt of messages from the L2 and L4 directly in the message processing loop (on_message from the subscription)
		m_rcvMsg->loop_start();
	}

private:
	// this function will get every byte that comes in on the comm port.  It is expected
	// that it will do the frame-level decode; i.e. the FASTLAN decode in this case.
	void recvCommRecieveL3Decode( char * inCharBuff, int count );

	//mqtt_message recvMsg;
	L3Message * m_rcvMsgL3;
};


#pragma pack()

#endif /* L3COMM_H_ */
