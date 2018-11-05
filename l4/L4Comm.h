/*
 * L4Comm.h
 *
 *  Created on: Aug 25, 2018
 *      Author: ds
 */

#ifndef L4COMM_H_
#define L4COMM_H_

#include "L3Comm.h"
#include "L4JsonInterface.h"
#include "msg24hrStorage.h"

#include "L4AckdMsgInfo.h"
#include "L4AckFromCloud.h"
#include "L4CommMessageFromCloud.h"
#include "L4CommMessageFromL3ToL4.h"

#include <thread>
#include <unistd.h>
#include <vector>



#pragma pack(1)



class L4Comm
{
public:
	L4Comm()
	{
	}

	virtual ~L4Comm()
	{
		if( m_ack )
			delete m_ack;
		if( m_rcvMsgL4_From_L3_To_Cloud )
			delete m_rcvMsgL4_From_L3_To_Cloud;
		if( m_rcvMsgL4_FromCloud )
			delete m_rcvMsgL4_FromCloud;
	}

	void startL4Comms()
	{
		// start the thread to handle the ack / retry stuff
		m_ack = new L4AckFromCloud(this);
		m_ack->Init();

		// start up the mqtt messaging loop ...

		// the mqtt recevier that subscribes to messages from the lower layer (and publishes back to the lower layer)
		m_rcvMsgL4_From_L3_To_Cloud = new L4CommMessageFromL3ToL4((char *) "L4CommL3", this);
		m_rcvMsgL4_From_L3_To_Cloud->Init();

		// the mqtt recevier that subscribes to messages from the cloud (and publishes back to the cloud)
		m_rcvMsgL4_FromCloud = new L4CommMessageFromCloud((char *) "L4CommCloud", this);
		m_rcvMsgL4_FromCloud->Init();


		// also start up a garbage collector to clear out old 24hr storage files if need be
		{
		    timer_t timerid;
		    struct itimerspec ts;
		    struct sigevent sev;

		    // Specify a repeating timer that runs the 24 hour storage garbage collector
		    ts.it_value.tv_sec = (60 * 10);		// first pass on garbage collection in 10 minutes
		    ts.it_value.tv_nsec = 0;
		    ts.it_interval.tv_sec = (60 * 60);	// then every hour after the first
		    ts.it_interval.tv_nsec = 0;


		    sev.sigev_notify = SIGEV_THREAD;
		    sev.sigev_notify_function = Msg24hrStorage::GarbageCollector;
		    sev.sigev_notify_attributes = NULL;

		    timer_create(CLOCK_REALTIME, &sev, &timerid);

		    timer_settime(timerid, 0, &ts, 0);
		}
	}

	void PublishToCloud( char * topic, int size, char * msg, int qos = QOS_0 )
	{
		m_rcvMsgL4_FromCloud->publish(topic, size, msg, qos);
	}

	void PublishToL3( char * topic, int size, char * msg, int qos = QOS_0 )
	{
		m_rcvMsgL4_From_L3_To_Cloud->publish(topic, size, msg, qos);
	}

	void AddAckedMessage( JSON_MESSAGE_BASE * msg, string & topic, int qos, bool save )
	{
		m_ack->AddAckedMessage( msg, topic, qos, save );
	}

	void IncomingAckMessage( string & corrId, uint16_t errorCode )
	{
		m_ack->IncomingAckMessage( corrId, errorCode );
	}

	void dump24hourMessages();




private:
	L4CommMessageFromL3ToL4 * m_rcvMsgL4_From_L3_To_Cloud;
	L4CommMessageFromCloud * m_rcvMsgL4_FromCloud;
	L4AckFromCloud * m_ack;
};

#pragma pack()
#endif /* L4COMM_H_ */
