/*
 * L3CommFastlan.cpp
 *
 *  Created on: Aug 19, 2018
 *      Author: ds
 */
#include "L3Comm.h"

#ifdef ADD_FASTLAN_COMMANDS
SCK_UNSIGNED_8BIT vc210L3Message::SCKIncomingRequestReadEnableDisableSecurity( SCK_UNSIGNED_16BIT offset )
{
	printf("SCKIncomingRequestReadEnableDisableSecurity\n");
	return 0;
}

SCK_UNSIGNED_8BIT vc210L3Message::SCKIncomingRequestWriteEnableDisableSecurity( SCK_UNSIGNED_16BIT offset )
{
	printf("SCKIncomingRequestWriteEnableDisableSecurity\n");
	return 0;
}

void vc210L3Message::SCKReadWriteFastlanCallbackFunction( L2_PACKET * inputBuffer )
{
	printf("SCKReadWriteFastlanCallbackFunction\n");

	// NOT USED AT THIS TIME.  We expect no FASTLAN reads from the VC210 in this app.
	//FASTLAN_READ_CMD_MSG tmp( (void *) inputBuffer->GetPayloadPtr() );
}


void vc210L3Message::SCKGenerateFastlanReadResponse( FASTLAN_READ_CMD_MSG & messageBufferPtr )
{
	// NOT USED AT THIS TIME !!!

	int length;
	int count;
	int result;
	int tid;
	int offset;
	int type;
	int oid;

	SCK_ERRORCODES errorCode = ERRORCODE_NO_ERROR;
	FASTLAN_READ_REPLY_MSG replyMsg;

	printf("SCKGenerateFastlanReadResponse\n");

	count = messageBufferPtr.GetCount();
	tid = messageBufferPtr.GetTID();
	offset = messageBufferPtr.GetOffset();
	type = messageBufferPtr.GetType();
	oid = messageBufferPtr.GetOID();

	switch( messageBufferPtr.GetType( ))
	{
		case DATATYPE_STRING:
		case DATATYPE_UBYTE:
		case DATATYPE_BYTE:
			length = count;
			printf( "-----> SCKGenerateFastlanReadResponse: type DATATYPE_BYTE\n" );
			break;

		case DATATYPE_UINTEGER:
		case DATATYPE_INTEGER:
			length = (2 * count);
			printf( "-----> SCKGenerateFastlanReadResponse: type DATATYPE_BYTE\n" );
			break;

		case DATATYPE_LONG:
		case DATATYPE_ULONG:
		case DATATYPE_FLOAT:
		case DATATYPE_SFLOAT:
			length = (4 * count);
			printf( "-----> SCKGenerateFastlanReadResponse: type DATATYPE_BYTE\n" );
			break;

		default:
			errorCode = ERRORCODE_BAD_DATATYPE;
			printf( "-----> SCKGenerateFastlanReadResponse: type ERROR\n" );
			break;
	}

	switch( messageBufferPtr.GetOID() )
	{
	case 3401:
		printf( "-------> SCKIncomingRequestReadEnableDisableSecurity\n" );
		result = SCKIncomingRequestReadEnableDisableSecurity( messageBufferPtr.GetOffset() );
		break;

	case 555:
	case 3400:
	case 3402:
	case 3403:
	case 3404:
	case 3405:
	case 3406:
	case 5125:
	case 589:
	case 410:
	case 285:
		// handle not handled
		break;
	}

	// build a response
	replyMsg.SetTID( tid );
	replyMsg.SetRW( COMMAND_READ_REPLY );
	replyMsg.SetOID( oid );
	replyMsg.SetType( type );
	replyMsg.SetOffset( offset );
	replyMsg.SetCount( count );
//	for( int i=0; i<length; )
//	{
//
//	}
	// hard code a response for now.
	replyMsg.SetData( 0, (uint8_t) 0 );


	// now build the output buffer
	L2_PACKET outBuf;

	// build the response
	outBuf.SetL2CS( 'S' );
	outBuf.SetL2Cmd( COMMAND_FASTLAN );
	outBuf.SetL2Length( 12 );

	uint8_t * r = (uint8_t *)&replyMsg;

	for( int i=0; i< (int)sizeof(FASTLAN_READ_CMD_MSG) + length; i++ )
		outBuf.SetPayload( i, r[i] );

	Write( &outBuf, MSG_L2_From_L3 );


}



void vc210L3CommSubMessage::SCKGenerateFastlanWriteResponse( SCK_MESSAGE_BUFFER & messageBufferPtr )
{
	printf("SCKGenerateFastlanWriteResponse\n");
}
#endif


