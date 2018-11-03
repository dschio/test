/*
 * L3Comm.cpp
 *
 *  Created on: Jul 21, 2018
 *      Author: ds
 */

#include "L3Comm.h"

uint16_t FASTLAN_COMMAND_MSG::nextTid = 1;


void L3Message::SimpleAck()
{
	L2_PACKET outBuf;

	// build the response
	outBuf.SetL2CS( 'S' );
	outBuf.SetL2Cmd( STATUS_ACK );
	outBuf.SetL2Length( STATUS_LEN );

	Write( &outBuf, MSG_L2_From_L3 );
}

void L3Message::Write( L2_PACKET * msg, const char * topic, int qos )
{
	publish( topic, msg->GetL2TotalLength(), (const char *) msg, qos);
}

void L3Message::Write( COMMS_EVENT * msg, const char * topic, int qos )
{
	publish( topic,sizeof(COMMS_EVENT), (const char *) msg, qos);
}

void L3Message::Write( FASTLAN_RESPONSE_MSG * msg, int length, const char * topic, int qos )
{
	publish( topic,length, (const char *) msg, qos);
}


// process a message from upper layer (originating from the "cloud")
void L3Message::processL4Packet( FASTLAN_COMMAND_MSG * packet )
{
	printf( "originating from the cloud %d, %d, %d, %d\n",
			packet->GetOID(),
			packet->GetType(),
			packet->GetOffset(),
			packet->GetCount()
	);

	// build an L2 message for the VC 210 from the fastlan message we get
	L2_PACKET msg;
	msg.SetL2CS( 'C' );
	msg.SetL2Cmd( 0xFF55 );
	msg.SetL2Length( 12 );
	memcpy( msg.GetPayloadPtr(), (char *)packet, 12 );
	Write( & msg, MSG_L2_From_L3 );

}

// process a message from lower layer (originating from the VC210)
void L3Message::processL2Packet( L2_PACKET * packet )
{
	uint16_t ret;

	printf( "processL2Packet\n");
	// the packet is the un-wrapped level 2 packet (everything but the STX / CRC / ETC).

	// process based on whether a command or response
	//
	// commands from the VC210
	if( packet->GetL2CS() == 'C'  )
	{
		// command
		switch( ret = packet->GetL2Cmd() )
		{
		case FRAMETYPE_EVENT_REPORT:
			printf("FRAMETYPE_EVENT_REPORT\n");
			SCKEventCallbackFunction(packet);
			break;

		case FRAMETYPE_ARE_YOU_THERE:
			printf("FRAMETYPE_ARE_YOU_THERE\n");
			SCKProcessReadWriteCommand(packet);
			break;

		case FRAMETYPE_FASTLAN:
			printf("FRAMETYPE_FASTLAN\n");
			// NOT USED AT THIS TIME
			//SCKReadWriteFastlanCallbackFunction(packet);
			break;

		default:
		case FRAMETYPE_READ_CONFIG_IO:
		case FRAMETYPE_WRITE_CONFIG_IO:
			printf("FRAMETYPE_??? - NOT HANDLED\n");
			break;
		}
	}

	// responses to a command to the VC210
	else
	{
		switch( ret = packet->GetL2Cmd() )
		{
		case STATUS_SETPOINT_READ_SUCCESS:
		case STATUS_SETPOINT_WRITE_SUCCESS:
			SCKProcessSetpointStatus(packet);
			break;

		case FRAMETYPE_FASTLAN:
			SCKProcessFastlan(packet);
			break;

		default:
			break;
		}
	}

}

