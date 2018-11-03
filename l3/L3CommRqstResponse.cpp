/*
 * L3CommRqstResponse.cpp
 *
 *  Created on: Aug 20, 2018
 *      Author: ds
 */

#include "L3CommRqstResponse.h"
#include "L3Comm.h"


void L3Message::SCKProcessSetpointStatus( L2_PACKET * packet )
{
	printf("Process Setpoints\n");

	SetpointResponse * p = (SetpointResponse *) packet->GetPayloadPtr();

	printf( "===== status %02x, what %d, which %d\n", p->status, p->bdata.prodHdr.what, p->bdata.prodHdr.which );
	printf("===== name %s  \n", p->bdata.eep2.productName );
}

void L3Message::SCKProcessFastlan( L2_PACKET * packet )
{
	int length;

	//printf("Process FastLan\n");

	FASTLAN_RESPONSE_MSG * m = (FASTLAN_RESPONSE_MSG *)packet->GetPayloadPtr();

	length = sizeof(FASTLAN_RESPONSE_MSG);
	length += m->GetDataLength();

	Write( m, length, MSG_L4_From_L3_FAST );
}


