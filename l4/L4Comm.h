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
	}

	void dump24hourMessages()
	{
		vector<uint8_t> msgBuffer;
		Msg24hrStorage z;

		for( int i = 24;i >= 0;i-- )
		{
			z.LockMessageFile( true );
			// walk the files
			if( z.OpenMessageStorageFile(i) )
			{
				printf("dump24hourMessages:: got file at %d hours back\n", i);

				while( z.GetRecordFromStorage(msgBuffer) )
				{
					// we got the message here!  do what you will.

					// cast the message part to point to the original storage level format
					Msg24hrStorage::Msg24hrStorageRecord * gotMsg = (Msg24hrStorage::Msg24hrStorageRecord *) &msgBuffer[0];

					// the deserialize it into the actual desired message format
					L4AckdMsgInfo mmm;
					mmm.Deserialize((char *) &gotMsg->m_record[0]);

					// send it out as a new message
					m_ack->AddAckedMessage( mmm.GetMsgPtr(), mmm.GetTopicPtr(), mmm.GetQos(), false );
				}

				z.CloseMessageStorageFile();

				// at this point, all the messages from this file have been queued for transmission
				// so we can delete the file.
				z.DeleteMessageStorageFile(i);

			}
			else
			{
				printf("no file for %d\n", i);
			}
			z.LockMessageFile( false );
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

	L4CommMessageFromL3ToL4 * m_rcvMsgL4_From_L3_To_Cloud;
	L4CommMessageFromCloud * m_rcvMsgL4_FromCloud;
	L4AckFromCloud * m_ack;
};

#pragma pack()
#endif /* L4COMM_H_ */
