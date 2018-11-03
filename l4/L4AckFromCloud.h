/*
 * L4AckFromCloud.h
 *
 *  Created on: Nov 2, 2018
 *      Author: ds
 */

#ifndef L4_L4ACKFROMCLOUD_H_
#define L4_L4ACKFROMCLOUD_H_

#include <thread>
#include <unistd.h>


class L4Comm;

#pragma pack(1)

class L4AckFromCloud
{
public:
	L4AckFromCloud( L4Comm * ptr )
	{
		m_parentPtr = ptr;
	}

	~L4AckFromCloud()
	{
		std::lock_guard < std::mutex > lck(m_ackMtx);
		for( m_ackItr = m_ack.begin();m_ackItr != m_ack.end();m_ackItr++ )
		{
			// remove it from the map
			// first free up the structure
			if( (*m_ackItr) != nullptr )
				delete *m_ackItr;
			m_ackItr = m_ack.erase(m_ackItr);
			if( m_ackItr == m_ack.end() )
				break;
		}
	}

	void Init()
	{
		thread ackThread(&L4AckFromCloud::AckThread, this);
		ackThread.detach();
	}

	void Publish( string * topic, string * msg, int qos );
	void AddAckedMessage( JSON_MESSAGE_BASE * msg, string & topic, int qos, bool save );
	void AddAckedMessage( string * msg, string * topic, int qos, bool save );
	void IncomingAckMessage( string & corrId, uint16_t errorCode );
	void RemoveMessage( JSON_MESSAGE_BASE * msg );

	int GetMessageCount()
	{
		return m_ack.size();
	}

	void SaveMessage( L4AckdMsgInfo * msg );

private:
	void * AckThread();

private:
	typedef std::vector<L4AckdMsgInfo *> AckList;
	typedef AckList::iterator AckListItr;

	AckList m_ack;
	AckListItr m_ackItr;
	mutex m_ackMtx;

	L4Comm * m_parentPtr;
};

#pragma pack()

#endif /* L4_L4ACKFROMCLOUD_H_ */
