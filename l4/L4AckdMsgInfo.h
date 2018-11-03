/*
 * L4AckdMsgInfo.h
 *
 *  Created on: Nov 2, 2018
 *      Author: ds
 */

#ifndef L4_L4ACKDMSGINFO_H_
#define L4_L4ACKDMSGINFO_H_

#include <unistd.h>

#pragma pack(1)

class L4AckdMsgInfo
{
public:
	L4AckdMsgInfo( JSON_MESSAGE_BASE * msg, string & topic, int qos, bool save )
	{
		m_guid = msg->GetGuid();
		m_jsonMsg = msg->GetOutput();
		m_mqttTopic = topic;
		m_qos = qos;

		m_retryCount = 0;
		m_retryStartTimer = current_timestamp();

		m_saveFlag = save;
	}

	L4AckdMsgInfo( string * msg, string * topic, int qos, bool save )
	{
		//m_guid = msg->GetGuid();
		m_jsonMsg = *msg;
		m_mqttTopic = *topic;
		m_qos = qos;

		m_retryCount = 0;
		m_retryStartTimer = current_timestamp();

		m_saveFlag = save;
	}

	L4AckdMsgInfo()
	{
		m_jsonMsg.erase();
		m_mqttTopic.erase();
	}

	~L4AckdMsgInfo()
	{

	}

	uint64_t GetGuid()
	{
		return m_guid;
	}
	string * GetMsgPtr()
	{
		return &m_jsonMsg;
	}
	string * GetTopicPtr()
	{
		return &m_mqttTopic;
	}
	int GetQos()
	{
		return m_qos;
	}
	int GetRetry()
	{
		return m_retryCount;
	}
	uint64_t GetRetryTime()
	{
		return m_retryStartTimer;
	}
	void NextRetry()
	{
		m_retryCount++;
		m_retryStartTimer = current_timestamp();
	}
	bool IsTimeout()
	{
		return (current_timestamp() > (m_retryStartTimer + CLOUD_RETRY_TIME));
	}

	bool SaveIt()
	{
		return m_saveFlag;
	}

	char * Serialize( int * totalLength )
	{
		// this is a fairly brute force serializer.  Not real elegant.
		// it saves the standard types (i.e. int, uint, etc) as is, but
		// saves the strings as a length / character string combination.
		// it puts the whole mess into a buffer.  the pointer to the buffer is returned.
		// IT MUST BE FREED AFTER USE.
		char * buf;
		char * tmp;

		// what is the total size of the buffer we need for the stuff we are going to serialize:
		//		guid
		//		qos
		//		jsonMsg
		//		topic
		//	we'll add in the other fields, but no need to use them
		//		retryCount
		//		retryStartTimer
		int totalSize;
		totalSize = sizeof(m_guid);
		totalSize += sizeof(m_qos);
		totalSize += sizeof(m_retryCount);
		totalSize += sizeof(m_retryStartTimer);

		// the string sizes including the pre-pended length field
		totalSize += m_jsonMsg.size() + sizeof(int);
		totalSize += m_mqttTopic.size() + sizeof(int);

		// allocate a buffer big enough for all of it
		tmp = buf = new char[totalSize];

		// start saving the stuff

		// guid
		memcpy(tmp, &m_guid, sizeof(m_guid));
		tmp += sizeof(m_guid);

		// qos
		memcpy(tmp, &m_qos, sizeof(m_qos));
		tmp += sizeof(m_qos);

		// retryCount
		memcpy(tmp, &m_retryCount, sizeof(m_retryCount));
		tmp += sizeof(m_retryCount);

		// retryStartTimer
		memcpy(tmp, &m_retryStartTimer, sizeof(m_retryStartTimer));
		tmp += sizeof(m_retryStartTimer);

		// now the strings...

		// jsonMsg
		int size = m_jsonMsg.size();
		memcpy(tmp, &size, sizeof(int));
		tmp += sizeof(int);
		memcpy(tmp, m_jsonMsg.data(), size);
		tmp += size;

		// topic
		size = m_mqttTopic.size();
		memcpy(tmp, &size, sizeof(int));
		tmp += sizeof(int);
		memcpy(tmp, m_mqttTopic.data(), size);
		tmp += size;

		if( totalLength )
		{
			*totalLength = totalSize;
		}

		return buf;
	}

	void Deserialize( char * buf )
	{
		char * tmp = buf;

		// the opposite of the above
		// guid
		m_guid = (uint64_t) *(uint64_t *) tmp;
		tmp += sizeof(m_guid);

		// m_qos
		m_qos = (int) *(int *) tmp;
		tmp += sizeof(m_qos);

		// m_retryCount
		m_retryCount = (int) *(int *) tmp;
		tmp += sizeof(m_retryCount);

		// m_retryStartTimer
		m_retryStartTimer = (uint64_t) *(uint64_t*) tmp;
		tmp += sizeof(m_retryStartTimer);

		// m_jsonMsg
		m_jsonMsg.erase();
		int size = *(int *) tmp;
		tmp += sizeof(int);
		m_jsonMsg.append(tmp, size);
		tmp += size;

		// m_mqttTopic
		m_mqttTopic.erase();
		size = *(int *) tmp;
		tmp += sizeof(int);
		m_mqttTopic.append(tmp, size);
	}

private:
	uint64_t m_guid;
	string m_jsonMsg;
	string m_mqttTopic;
	int m_qos;
	int m_retryCount;
	uint64_t m_retryStartTimer;
	bool m_saveFlag;
};

#pragma pack()

#endif /* L4_L4ACKDMSGINFO_H_ */
