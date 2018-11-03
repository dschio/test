/*
 * L4CommMessageFromL3ToL4.h
 *
 *  Created on: Nov 2, 2018
 *      Author: ds
 */

#ifndef L4_L4COMMMESSAGEFROML3TOL4_H_
#define L4_L4COMMMESSAGEFROML3TOL4_H_

#pragma pack(1)

class L4CommMessageFromL3ToL4: public mqtt_message
{
public:
	L4CommMessageFromL3ToL4( char *id, L4Comm * ptr ) :
			mqtt_message(id)
	{
		setTopic((char *) MSG_L4_TOPIC);
		connect((char *) LOCAL_BROKER_IP, 1883);

		// save a pointer back to the object that created me
		m_parentPtr = ptr;
	}

	~L4CommMessageFromL3ToL4()
	{
		loop_stop();
		disconnect();
	}

	void Init()
	{
		// in this case, we will handle the receipt of messages from the Layer directly in the message processing loop (on_message from the subscription)
		loop_start();
	}

	// process the message received from the MQTT service
	virtual void on_message( const struct mosquitto_message *message )
	{
		if( message == NULL )
			return;

		if( message->payload == NULL )
			return;

		// process depending on the topic
		if( strncmp(message->topic, MSG_L4_From_L3, 32) == 0 )
		{
			COMMS_EVENT * rcvPkt = (COMMS_EVENT *) message->payload;
			printf("message from topic %s, %d\n", message->topic, message->mid);
			processPacket(rcvPkt);
			return;
		}

		if( strncmp(message->topic, MSG_L4_From_L3_FAST, 32) == 0 )
		{
			FASTLAN_RESPONSE_MSG * rcvPkt = (FASTLAN_RESPONSE_MSG *) message->payload;
			processPacket(rcvPkt);
			return;
		}

		printf("vc210L4CommMessage WRONG MESSAGE FROM on_message: %s\n", message->topic);
	}

	L4JsonInterface * GetJsonConfig();

private:
	// the processPacket funcion will eventually call one or more of these, depending on the message received.
	//
	//	called upon the receipt of an event message from the VC210

	// this sends a message down the stack to send out on the wire
	//void Write( FASTLAN_COMMAND_MSG * msg, const char * topic, int qos = QOS_0 );

	// the starting point for receiving data packets from the lower level
	void processPacket( COMMS_EVENT * packet );
	void processPacket( FASTLAN_RESPONSE_MSG * packet );

	L4Comm * m_parentPtr;
};

#pragma pack()

#endif /* L4_L4COMMMESSAGEFROML3TOL4_H_ */
