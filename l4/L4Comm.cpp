/*
 * L4Comm.cpp
 *
 *  Created on: Aug 25, 2018
 *      Author: ds
 */
#include "config.h"
#include "L4Comm.h"
#include <unistd.h>

#ifdef USE_STRING_CORRID_INSTEAD
string STATUS_CORRELATION_ID::s_currentStatusCorrelation = "";
mutex STATUS_CORRELATION_ID::g_currentStatusCorrelationMtx;
#else
atomic<uint64_t> STATUS_CORRELATION_ID::s_currentStatusCorrelation;
#endif


void L4AckFromCloud::Publish( string * topic, string * msg, int qos )
{
	// publish the message again as though it came back in throught the lower level where it first originated
	m_parentPtr->PublishToCloud((char *) topic->c_str(), msg->size(), (char *) msg->c_str(), qos);
}

void L4AckFromCloud::SaveMessage( L4AckdMsgInfo * msg )
{
	// the message was not ack'd, so put into 24 hour storage for possible later retrieval
	int length;
	Msg24hrStorage m;

	// first, serialize the message into a buffer
	char * buf = msg->Serialize(&length);

	if( buf && length )
	{
		m.AddRecord(buf, length);
	}

	if( buf )
		delete buf;
}

void * L4AckFromCloud::AckThread()
{
	for( ;; )
	{
		printf("in AckThread: messages: %d\n", GetMessageCount());

		// scan through the map looking for messages that may have timed out
		{
			std::lock_guard < std::mutex > lck(m_ackMtx);

			for( m_ackItr = m_ack.begin();m_ackItr != m_ack.end();m_ackItr++ )
			{
				printf("retry count %d\n", (*m_ackItr)->GetRetry());

				if( (*m_ackItr)->IsTimeout() )
				{
					if( (*m_ackItr)->GetRetry() < CLOUD_RETRY_COUNT )
					{
						printf("-----timeout on %" PRIX64 " \n", (*m_ackItr)->GetGuid());
						// try again

						// publish it
						m_parentPtr->PublishToCloud((char *) (*m_ackItr)->GetTopicPtr()->c_str(), (*m_ackItr)->GetMsgPtr()->size(), (char *) (*m_ackItr)->GetMsgPtr()->c_str(), (*m_ackItr)->GetQos());

						(*m_ackItr)->NextRetry();
					}
					else
					{
						// remove it from the map, but save it to the file
						if( (*m_ackItr)->SaveIt() )
							SaveMessage(*m_ackItr);

						// first free up the structure
						delete *m_ackItr;
						// then pull it from the ack map
						m_ackItr = m_ack.erase(m_ackItr);
						if( m_ackItr == m_ack.end() )
							break;
					}
				}
			}
		}

		sleep(1);
	}
}

void L4AckFromCloud::AddAckedMessage( JSON_MESSAGE_BASE * msg, string & topic, int qos, bool save )
{
	std::lock_guard < std::mutex > lck(m_ackMtx);

	// here is where we put the message onto the queue and start the retry crap.
	L4AckdMsgInfo * m = new L4AckdMsgInfo(msg, topic, qos, save );

	m_ack.push_back(m);

	int outStrSize;
	string outStr = msg->GetOutput(&outStrSize);

	m_parentPtr->PublishToCloud((char *) topic.c_str(), outStrSize, (char *) outStr.c_str(), qos);
}

void L4AckFromCloud::AddAckedMessage( string * msg, string * topic, int qos, bool save )
{
	std::lock_guard < std::mutex > lck(m_ackMtx);

	// here is where we put the message onto the queue and start the retry crap.
	L4AckdMsgInfo * m = new L4AckdMsgInfo(msg, topic, qos, save );

	m_ack.push_back(m);

	m_parentPtr->PublishToCloud((char *) topic->c_str(), msg->size(), (char *) msg->c_str(), qos);
}

void L4AckFromCloud::IncomingAckMessage( string & corrId, uint16_t errorCode )
{
	uint64_t guidInt = stoull(corrId, NULL, 16);

	std::lock_guard < std::mutex > lck(m_ackMtx);

	// find the message on the queue, then remove it, assuming it is still there.
	//
	// the incoming ack message is assummed:
	//	unique guid
	//	correlaton id is the guid of the message being ack'd
	for( m_ackItr = m_ack.begin();m_ackItr != m_ack.end();m_ackItr++ )
	{
		L4AckdMsgInfo * jMsg = *m_ackItr;

		if( jMsg->GetGuid() == guidInt )
		{
			if( *m_ackItr )
				delete *m_ackItr;

			m_ackItr = m_ack.erase(m_ackItr);
			if( m_ackItr == m_ack.end() )
				break;
		}
	}
}

// the starting point for receiving COMMS_EVENT data packets from the lower level
void L4CommMessageFromL3ToL4::processPacket( COMMS_EVENT * packet )
{
	EVENT_TABLE::eventItr itr;

	if( (itr = g_EventTable.FindEvent(packet->GetOID())) == g_EventTable.NotInEvents() )
	{
		printf("********************************************************************** Not In events  %s, %04x\n", itr->second.c_str(), packet->GetData());
		return;
	}

	// prepare for sending a mqtt message:
	// 	get the json configuration for sending these messages
	string topic = g_bezelConfig.GetTopicForOID( packet->GetOID() );

	JSON_MESSAGE_BASE * msgToSend = NULL;

	// handle each event.  each sends a different json message
	switch( packet->GetOID() )
	{
	case EVENT_TABLE::EVENT_OIDS::COOK_START:
		msgToSend = new JSON_MESSAGE_COOK_START(packet->GetData());
		break;
	case EVENT_TABLE::EVENT_OIDS::COOK_COMPLETE:
		msgToSend = new JSON_MESSAGE_COOK_COMPLETE(packet->GetData());
		break;
	case EVENT_TABLE::EVENT_OIDS::COOK_COMPLETE_ACK_:
		msgToSend = new JSON_MESSAGE_COOK_COMPLETE_ACK(packet->GetData());
		break;
	case EVENT_TABLE::EVENT_OIDS::COOK_CANCEL:
		msgToSend = new JSON_MESSAGE_COOK_CANCEL(packet->GetData());
		break;
	case EVENT_TABLE::EVENT_OIDS::COOK_PREALARM:
		msgToSend = new JSON_MESSAGE_COOK_PREALARM(packet->GetData());
		break;
	case EVENT_TABLE::EVENT_OIDS::COOK_QUANTITY:
		msgToSend = new JSON_MESSAGE_COOK_QUANTITY(packet->GetData());
		break;

	case EVENT_TABLE::EVENT_OIDS::HOLD_START:
		msgToSend = new JSON_MESSAGE_HOLD_START(packet->GetData());
		break;
	case EVENT_TABLE::EVENT_OIDS::HOLD_CANCEL:
		msgToSend = new JSON_MESSAGE_HOLD_CANCEL(packet->GetData());
		break;
	case EVENT_TABLE::EVENT_OIDS::HOLD_COMPLETE:
		msgToSend = new JSON_MESSAGE_HOLD_COMPLETE(packet->GetData());
		break;
	case EVENT_TABLE::EVENT_OIDS::HOLD_COMPLETE_ACK_:
		msgToSend = new JSON_MESSAGE_HOLD_COMPLETE_ACK(packet->GetData());
		break;

	case EVENT_TABLE::EVENT_OIDS::POLISH_START:
		msgToSend = new JSON_MESSAGE_POLISH_START(packet->GetData());
		break;
	case EVENT_TABLE::EVENT_OIDS::POLISH_FINISHED:
		msgToSend = new JSON_MESSAGE_POLISH_FINISHED(packet->GetData());
		break;
	case EVENT_TABLE::EVENT_OIDS::POLISH_FORCED_START:
		msgToSend = new JSON_MESSAGE_FORCED_START(packet->GetData());
		break;
	case EVENT_TABLE::EVENT_OIDS::POLISH_FORCED_CANCEL:
		msgToSend = new JSON_MESSAGE_FORCED_CANCEL(packet->GetData());
		break;

	case EVENT_TABLE::EVENT_OIDS::VAT_STATE:
		msgToSend = new JSON_MESSAGE_VAT_STATE(packet->GetData());
		break;
	case EVENT_TABLE::EVENT_OIDS::RIGHT_VAT_STATE:
		msgToSend = new JSON_MESSAGE_RIGHT_VAT_STATE(packet->GetData());
		break;

	case EVENT_TABLE::EVENT_OIDS::VAT_STATE_MISC:
		msgToSend = new JSON_MESSAGE_VAT_STATE_MISC(packet->GetData());
		break;
	case EVENT_TABLE::EVENT_OIDS::RIGHT_VAT_STATE_MISC:
		msgToSend = new JSON_MESSAGE_RIGHT_VAT_STATE_MISC(packet->GetData());
		break;

	case EVENT_TABLE::EVENT_OIDS::VAT_STATE_MISC_2:
		msgToSend = new JSON_MESSAGE_VAT_STATE_MISC_2(packet->GetData());
		break;
	case EVENT_TABLE::EVENT_OIDS::VAT_STATE_RIGHT_MISC_2:
		msgToSend = new JSON_MESSAGE_RIGHT_VAT_STATE_MISC_2(packet->GetData());
		break;

	case EVENT_TABLE::EVENT_OIDS::VAT_ERROR:
		msgToSend = new JSON_MESSAGE_VAT_ERROR(packet->GetData());
		break;
	case EVENT_TABLE::EVENT_OIDS::VAT_ERROR_RIGHT:
		msgToSend = new JSON_MESSAGE_RIGHT_VAT_ERROR(packet->GetData());
		break;

	case EVENT_TABLE::EVENT_OIDS::TEMPERATURE:
		msgToSend = new JSON_MESSAGE_TEMPERATURE(packet->GetData());
		break;
	case EVENT_TABLE::EVENT_OIDS::RIGHT_TEMPERATURE:
		msgToSend = new JSON_MESSAGE_RIGHT_TEMPERATURE(packet->GetData());
		break;

	case EVENT_TABLE::EVENT_OIDS::SET_TEMPERATURE:
		msgToSend = new JSON_MESSAGE_SET_TEMPERATURE(packet->GetData());
		break;
	case EVENT_TABLE::EVENT_OIDS::SET_RIGHT_TEMPERATURE:
		msgToSend = new JSON_MESSAGE_RIGHT_SET_TEMPERATURE(packet->GetData());
		break;

	default:
		printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++FRAMETYPE_EVENT_REPORT %0d: NOT HANDLED\n", packet->GetOID());
		return;
	}

	if( msgToSend )
	{
		// note: the called function is expected to make a copy of the message, so we can delete the pointer here
		m_parentPtr->m_ack->AddAckedMessage(msgToSend, topic, QOS_2, true);		// TODO:  clean this up
		delete msgToSend;
	}

}

// the starting point for receiving FASTLAN data packets from the lower level
void L4CommMessageFromL3ToL4::processPacket( FASTLAN_RESPONSE_MSG * packet )
{
	RESPONSE_TABLE::responseItr itr;

	if( (itr = g_ResponseTable.FindResponse((RESPONSE_TABLE::RESPONSE_OIDS) packet->GetOID())) == g_ResponseTable.NotInResponse() )
	{
		printf("********************************************************************** NOT IN RESPONSES %s, %04x\n", itr->second.c_str(), packet->GetOID());
		return;
	}

	// prepare for sending a mqtt message:
	// 	get the json configuration for sending these messages
	string topic = g_bezelConfig.GetTopicForOID( packet->GetOID() );

	JSON_MESSAGE_BASE * msgToSend = NULL;

	// handle each event.  each sends a different json message
	switch( packet->GetOID() )
	{
	case RESPONSE_TABLE::RESPONSE_OIDS::FLASH_NUMBER:
		msgToSend = new JSON_MESSAGE_CONFIG_FLASH_NUMBER(packet);
		break;
	case RESPONSE_TABLE::RESPONSE_OIDS::DOWNLOAD_NUMBER:
		msgToSend = new JSON_MESSAGE_CONFIG_DOWNLOAD_NUMBER(packet);
		break;
	case RESPONSE_TABLE::RESPONSE_OIDS::SOFTWARE_NUMBER:
		msgToSend = new JSON_MESSAGE_CONFIG_SOFTWARE_NUMBER(packet);
		break;

	default:
		return;
	}

	if( msgToSend )
	{
		// get the actual formatted json message into a string to publish
		int outStrSize;
		string outStr = msgToSend->GetOutput(&outStrSize);

		// publish it
		m_parentPtr->PublishToCloud((char *) topic.c_str(), outStrSize, (char *) outStr.c_str());

		delete msgToSend;
	}
}

L4CommMessageFromCloud::L4CommMessageFromCloud( char *id, L4Comm * ptr ) :
		mqtt_message(id)
{
	// save a pointer back to the object that created me
	m_parentPtr = ptr;

	Document * doc = g_bezelConfig.GetConfig();
	string worldIp = ((*doc)["brokerIp"]).GetString();

	printf("using broker address %s\n", worldIp.c_str());

	setTopic((char *) MSG_INBOUND_ALL);	// MSG_INBOUND_SYSTEM);
	connect((char *) worldIp.c_str(), 1883);
}


// the starting point for receiving data packets from the cloud
void L4CommMessageFromCloud::processPacket( void * packet, int len )
{
	printf("  >>>>>>>>>> L4 processing packet from cloud\n");

	L4JsonInterface jsonMsg;

	if( !jsonMsg.Parse((const char *) packet) )
	{
		printf("vc210L4CommMessageCloud::processPacket error \n");
		return;
	}

	Document * doc = jsonMsg.GetDoc();
	if( doc->HasMember("type") )
	{
		string sss = ((*doc)["type"]).GetString();
		string corrId;
		string guid;

		if( doc->HasMember("correlationId") )
			corrId = ((*doc)["correlationId"]).GetString();

		if( doc->HasMember("guid") )
			guid = ((*doc)["guid"]).GetString();

		//printf(" this is a command: %s\n", sss.c_str());

		FASTLAN_COMMAND_MSG * msg = nullptr;

		while( true )
		{
			if( sss.compare("GetFlashNumber") == 0 )
			{
				msg = new FASTLAN_COMMAND_MSG_FLASH_NUMBER_MSG;
				break;
			}
			if( sss.compare("GetSoftwareNumber") == 0 )
			{
				msg = new FASTLAN_COMMAND_MSG_SOFTWARE_NUMBER_MSG;
				break;
			}
			if( sss.compare("GetDownloadNumber") == 0 )
			{
				msg = new FASTLAN_COMMAND_MSG_DOWNLOAD_NUMBER_MSG;
				break;
			}
			//		if( sss.compare( "Get Protocol Version" ) == 0 )
			//		{
			//			msg = new FASTLAN_COMMAND_MSG_PROTOCOL_VER_MSG;
			//			break;
			//		}
			if( sss.compare("Send24Hours") == 0 )		// temporary until someone decides
			{
				// we are going to process this one a bit diffently since it stays entirely within this level.
				// just call into the method to queue up the last 24 hours of saved messages.
				m_parentPtr->dump24hourMessages();
				break;
			}

			// if it is an ACK
			if( sss.compare("ACK") == 0 )
			{
				// send this over to whatever is keeping track of the acks and retries
				uint16_t errCode;
				if( doc->HasMember("errorCode") )
					errCode = ((*doc)["errorCode"]).GetUint();
				m_parentPtr->m_ack->IncomingAckMessage(corrId, errCode);		// TODO:  clean this up
				return;
			}

			break;
		}

		if( msg != nullptr )
		{
			STATUS_CORRELATION_ID sId(corrId);
			m_parentPtr->PublishToL3((char *) MSG_L3_From_L4, sizeof(FASTLAN_COMMAND_MSG), (char *) msg);
			delete msg;
		}
	}

	// ...
	else
	{
		printf("nope\n");
	}

//\	printf( "%s\n", msg.RetrieveJsonFormatMessage(NULL, USE_PRETTY).c_str());
}

