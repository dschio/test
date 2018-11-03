/*
 * L3CommReadWriteCmd.cpp
 *
 *  Created on: Aug 19, 2018
 *      Author: ds
 */

#include "L3Comm.h"


// at the moment, the only read / write supported is "are you there"
void L3Message::SCKProcessReadWriteCommand( L2_PACKET * packet )
{
	L2_PACKET l2_response;

	printf("SCKProcessReadWriteCommand\n");
	switch( packet->GetL2Cmd() )
	{
	case FRAMETYPE_ARE_YOU_THERE:
		printf("FRAMETYPE_ARE_YOU_THERE\n");
		l2_response.SetL2CS( 'S' );
		l2_response.SetL2Length( 4 );
		l2_response.SetL2Cmd( STATUS_AWAKE );
		l2_response.SetPayload( 0, 0x01 );
		l2_response.SetPayload( 1, 0x00 );
		break;

	default:
		printf("Unknown R_W Command %d\n", packet->GetL2Cmd() );
		l2_response.SetL2CS( 'S' );
		l2_response.SetL2Length( 2 );
		l2_response.SetL2Cmd( STATUS_UNKNOW_CMD );
		break;
	}

	Write( & l2_response, MSG_L2_From_L3 );
}

