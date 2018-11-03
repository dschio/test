/*
 * L4CommMessageFromCloud.h
 *
 *  Created on: Nov 2, 2018
 *      Author: ds
 */

#ifndef L4_L4COMMMESSAGEFROMCLOUD_H_
#define L4_L4COMMMESSAGEFROMCLOUD_H_

#pragma pack(1)

class L4CommMessageFromCloud: public mqtt_message
{
public:
	L4CommMessageFromCloud( char *id, L4Comm * ptr );
	~L4CommMessageFromCloud()
	{
		loop_stop();
		disconnect();
	}

	// process the message received from the MQTT service
	virtual void on_message( const struct mosquitto_message *message )
	{
		if( message == NULL )
			return;

		if( message->payload == NULL )
			return;

		void * rcvPkt = (void *) message->payload;
		printf("message from topic %s, %d\n", message->topic, message->mid);

		// process depending on the topic
		if( strncmp(message->topic, MSG_INBOUND_SYSTEM, 32) == 0 )
		{
			processPacket(rcvPkt, message->payloadlen);
			return;
		}

		// note:  same processing block for now.  Processing will depend on JSON fields
		if( strncmp(message->topic, MSG_INBOUND_ACK, 32) == 0 )
		{
			processPacket(rcvPkt, message->payloadlen);
			return;
		}

		// note:  same processing block for now.  Processing will depend on JSON fields
		if( strncmp(message->topic, MSG_INBOUND_CONFIG, 32) == 0 )
		{
			processPacket(rcvPkt, message->payloadlen);
			return;
		}

		printf("vc210L4CommMessageCloud WRONG MESSAGE FROM on_message: %s\n", message->topic);
	}

	void Init()
	{
		loop_start();
	}

private:
	// the processPacket funcion will eventually call one or more of these, depending on the message received.
	//
	//	called upon the receipt of an event message from the VC210

	// the starting point for receiving data packets from the cloud
	void processPacket( void * packet, int len );

	//L4JsonInterface * GetJsonConfig();

private:
	L4Comm * m_parentPtr;
};

#pragma pack()

#endif /* L4_L4COMMMESSAGEFROMCLOUD_H_ */
