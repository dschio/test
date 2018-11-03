/*
 * messaging.h
 *
 *  Created on: Jul 16, 2018
 *      Author: ds
 */

#ifndef MQTT_MESSAGING_H_
#define MQTT_MESSAGING_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>


#include "mosquitto.h"
#include "cpp/mosquittopp.h"

#define LOCAL_BROKER_IP	"localhost"
#define WORLD_IP		"192.168.2.104"		// obviously needs to be set programatically

//							vc210/layer that handles it/specific message
#define MSG_L2_From_L3			(const char *)"/vc210/L2/FromL3"
#define MSG_L3_From_L2			(const char *)"/vc210/L3/FromL2"
#define MSG_L3_From_L4			(const char *)"/vc210/L3/FromL4"
#define MSG_L4_From_L3			(const char *)"/vc210/L4/FromL3"
#define MSG_L4_From_L3_FAST		(const char *)"/vc210/L4/FromL3_Fast"

#define MSG_L2_TOPIC			(const char *)"/vc210/L2/#"
#define MSG_L3_TOPIC			(const char *)"/vc210/L3/#"
#define MSG_L4_TOPIC			(const char *)"/vc210/L4/#"


// topic definitions for messages originating in "cloud" (i.e. server) and heading to the bezel (i.e. client)
#define MSG_INBOUND_CONFIG		(const char *)"/us/qpm/client/config"
#define MSG_INBOUND_SYSTEM		(const char *)"/us/qpm/client/system"
#define MSG_INBOUND_ACK			(const char *)"/us/qpm/client/ack"
#define MSG_INBOUND_ALL			(const char *)"/us/qpm/client/#"

// topic definitions for messages originating in bezel (i.e. client) and heading to the "cloud" (i.e. server)
#define MSG_OUTBOUND_RESPONSE		(const char *)"/us/qpm/server/response"
#define MSG_OUTBOUND_WORKFLOW		(const char *)"/us/qpm/server/workflow"
#define MSG_OUTBOUND_ENVIRONMENT 	(const char *)"/us/qpm/server/environmental"
#define MSG_OUTBOUND_ERROR			(const char *)"/us/qpm/server/error"

#define QOS_0	0
#define QOS_1	1
#define QOS_2	2

#define TEST_COUNT	1000000



class mqtt_message : public mosqpp::mosquittopp
{
public:
	mqtt_message( const char * mid ) : mosqpp::mosquittopp( mid )
	{
		m_connected = -1;
		m_topic = "";
	};

	~mqtt_message()
	{
		m_topic.clear();
	}

	void setTopic( char * topic )
	{
		m_topic = topic;
	}

	const char * getTopic( )
	{
		return m_topic.c_str();
	}

	int publish( const char * topic, int bufSize, const char * buf, int qos = QOS_0 )
	{
		return mosquittopp::publish( NULL, topic, bufSize, buf, qos );
	}

	int connect( const char *host, int port=1883 )
	{
		return mosquittopp::connect( host, port );
	}

	int disconnect()
	{
		return mosquittopp::disconnect();
	}

	virtual void on_message( const struct mosquitto_message *message )
	{
		// dschio : the example(s) show a check here, but not necessary unless using wildcards and we want some distinction
		//if( !strcmp(message->topic, VC210_RCVD_PKT) )
		{
				// here is where we process the message  !!!!!!!!
		}
	}

	virtual void on_connect( int rc )
	{
		printf("Connected with code %d to topic: %s\n", rc, m_topic.c_str());
		if( rc == 0 )
		{
			/* Only attempt to subscribe on a successful connect. */
			if( ! m_topic.empty() )
				subscribe(NULL, m_topic.c_str());
		}
	}

	virtual void on_subscribe( int mid, int qos_count, const int *granted_qos )
	{
		printf("Subscription succeeded.\n");
		m_connected = true;
	}

	virtual void on_disconnect( int rc )
	{
		printf("disConnected with code %d to topic: %s\n", rc, m_topic.c_str());
	}

	bool m_connected;
private:
	std::string m_topic;

};





#endif /* MQTT_MESSAGING_H_ */
