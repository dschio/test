/*
 * L3CommEvents.cpp
 *
 *  Created on: Aug 19, 2018
 *      Author: ds
 */

#include "L3Comm.h"

class EVENT_TABLE g_EventTable;
class RESPONSE_TABLE g_ResponseTable;
class VAT_STATE_TABLE g_VatStateTable;


EVENT_TABLE::eventMap EVENT_TABLE::m_em =
{
	// oid (from definition enum)      string to use in the message "type" field
	{ COOK_START            ,     "COOK START" },
	{ COOK_COMPLETE         ,     "COOK COMPLETE" },
	{ COOK_COMPLETE_ACK_    ,     "COOK COMPLETE ACK " },
	{ COOK_CANCEL           ,     "COOK CANCEL" },
	{ COOK_PREALARM         ,     "COOK PREALARM" },
	{ HOLD_START            ,     "HOLD START" },
	{ HOLD_CANCEL           ,     "HOLD CANCEL" },
	{ HOLD_COMPLETE         ,     "HOLD COMPLETE" },
	{ HOLD_COMPLETE_ACK_    ,     "HOLD COMPLETE ACK " },
	{ VAT_STATE             ,     "VAT STATE" },
	{ RIGHT_VAT_STATE       ,     "RIGHT VAT STATE" },
	{ TEMPERATURE           ,     "Temperature" },
	{ RIGHT_TEMPERATURE     ,     "Right Temperature" },
	{ SET_TEMPERATURE       ,     "Set Temperature" },
	{ SET_RIGHT_TEMPERATURE ,     "Set Right Temperature " },
	{ VAT_STATE_MISC        ,     "VAT STATE MISC" },
	{ RIGHT_VAT_STATE_MISC  ,     "RIGHT VAT STATE MISC" },
	{ POLISH_FINISHED       ,     "POLISH FINISHED" },
	{ POLISH_START          ,     "POLISH START" },
	{ POLISH_FORCED_START   ,     "POLISH FORCED START" },
	{ POLISH_FORCED_CANCEL  ,     "POLISH FORCED CANCEL" },
	{ COOK_QUANTITY         ,     "COOK QUANTITY" },
	{ VAT_STATE_MISC_2      ,     "VAT STATE MISC 2" },
	{ VAT_STATE_RIGHT_MISC_2,     "VAT STATE RIGHT MISC 2" },
	{ VAT_ERROR             ,     "VAT ERROR" },
	{ VAT_ERROR_RIGHT       ,     "VAT ERROR_RIGHT" },
};

RESPONSE_TABLE::responseMap RESPONSE_TABLE::m_rm =
{
	// oid (from definition enum)      string to use in the message "type" field
	{ FLASH_NUMBER,					"FlashNumberResponse" },
	{ SOFTWARE_NUMBER, 				"SoftwareNumberResponse" },
	{ DOWNLOAD_NUMBER, 				"DownloadNumberResponse" },
};

VAT_STATE_TABLE::vatStateMap VAT_STATE_TABLE::m_statMap =
{
	{ STATE_OFF 			,	"STATE OFF"            },
	{ STATE_MELT            ,	"STATE MELT"           },
	{ STATE_IDLE            ,	"STATE IDLE"           },
	{ STATE_COOK            ,	"STATE COOK"           },
	{ STATE_BOIL            ,	"STATE BOIL"           },
	{ STATE_HOLD            ,	"STATE HOLD"           },
	{ STATE_FILTER          ,	"STATE FILTER"         },
	{ STATE_COOL            ,	"STATE COOL"           },
	{ STATE_SETBACK         ,	"STATE SETBACK"        },
	{ STATE_FAULT           ,	"STATE FAULT"          },
	{ STATE_10              ,	"STATE 10"             },
	{ STATE_POLISH          ,	"STATE POLISH"         },
	{ STATE_12              ,	"STATE 12"             },
	{ STATE_13              ,	"STATE 13"             },
	{ STATE_14              ,	"STATE 14"             },
	{ STATE_DISPOSAL        ,	"STATE DISPOSAL"       },
	{ STATE_16              ,	"STATE 16"             },
	{ STATE_FILL            ,	"STATE FILL"           },
	{ STATE_AUTO_COOL       ,	"STATE AUTO_COOL"      },
	{ STATE_19              ,	"STATE 19"             },
	{ STATE_20              ,	"STATE 20"             },
	{ STATE_21              ,	"STATE 21"             },
	{ STATE_22              ,	"STATE 22"             },
	{ STATE_CLEAN           ,	"STATE CLEAN"          },
	{ STATE_PROGRAM         ,	"STATE PROGRAM"        },
	{ STATE_STANDBY         ,	"STATE STANDBY"        },
	{ STATE_ON              ,	"STATE ON"             },
	{ STATE_DIAG            ,	"STATE DIAG"           },
	{ STATE_LOCK_OUT_BY_SPM ,	"STATE LOCK_OUT_BY_SPM"},
};

VAT_STATE_TABLE::vatStateMiscMap VAT_STATE_TABLE::m_stateMiscMap =
{
	{ STATE_MISC_NORMAL    		,	"STATE MISC NORMAL" },
	{ STATE_MISC_TEMP_LOW       ,	"STATE MISC TEMP LOW" },
	{ STATE_MISC_TEMP_HIGH      ,	"STATE MISC TEMP HIGH" },
	{ STATE_MISC_COOK_ALARM     ,	"STATE MISC COOK ALARM" },
	{ STATE_MISC_COOK_DONE      ,	"STATE MISC COOK DONE" },
	{ STATE_MISC_HOLD_DONE      ,	"STATE MISC HOLD DONE" },
	{ STATE_MISC_HOLD_ALARM     ,	"STATE MISC HOLD ALARM" },
	{ STATE_MISC_NONE           ,	"STATE MISC NONE" },
};

VAT_STATE_TABLE::vatStateMisc2Map VAT_STATE_TABLE::m_stateMisc2Map =
{
	{ STATE_MISC_2_NORMAL        	,	"STATE MISC 2 NORMAL" },
	{ STATE_MISC_2_DOOR_OPEN_HALF   ,	"STATE MISC 2 TEMP LOW" },
	{ STATE_MISC_2_DOOR_OPEN_FULL   ,	"STATE MISC 2 TEMP HIGH" },
	{ STATE_MISC_2_BLOWER_MOTOR	    ,	"STATE MISC 2 COOK ALARM" },
	{ STATE_MISC_2_NONE             ,	"STATE MISC 2 COOK DONE" },
};

VAT_STATE_TABLE::vatStateErrorMap VAT_STATE_TABLE::m_stateErrorMap =
{
	{ STATE_ERROR_NORMAL                 	,	"STATE ERROR NORMAL" },
	{ STATE_ERROR_WATER_DETECT           	,	"STATE ERROR WATER_DETECT" },
	{ STATE_ERROR_NO_HEAT                	,	"STATE ERROR NO_HEAT" },
	{ STATE_ERROR_PROBE_ERROR            	,	"STATE ERROR PROBE ERROR" },
	{ STATE_ERROR_GAS_ERROR              	,	"STATE ERROR GAS ERROR" },
	{ STATE_ERROR_HIGH_TEMP_ERROR        	,	"STATE ERROR HIGH TEMP ERROR" },
	{ STATE_ERROR_AIRSWITCH_ERROR        	,	"STATE ERROR AIRSWITCH ERROR" },
	{ STATE_ERROR_IGNITION_ERROR         	,	"STATE ERROR IGNITION ERROR" },
	{ STATE_ERROR_DRAIN_OPEN             	,	"STATE ERROR DRAIN OPEN" },
	{ STATE_ERROR_RECOVERY_ERROR         	,	"STATE ERROR RECOVERY ERROR" },
	{ STATE_ERROR_MOTOR_ERROR            	,	"STATE ERROR MOTOR_ERROR" },
	{ STATE_ERROR_IR_BURNER_ERROR        	,	"STATE ERROR IR_BURNER ERROR" },
	{ STATE_ERROR_OTHER                  	,	"STATE ERROR OTHER" },
	{ STATE_ERROR_NONE                    	,	"STATE ERROR NONE" },
};


void L3Message::SCKEventCallbackFunction( L2_PACKET * packet )
{
	COMMS_EVENT * RxEventP = (COMMS_EVENT *) packet->GetPayloadPtr();

	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++FRAMETYPE_EVENT_REPORT %0d %0x\n", RxEventP->GetOID(), RxEventP->GetData() );

	SCKEventHandleEvent(RxEventP);

	// always ack it
	SimpleAck();
}

bool L3Message::SCKEventCheckIncomingEventIsSupported( EVENT_TABLE::EVENT_OIDS oid )
{

	if( g_EventTable.FindEvent( oid ) != g_EventTable.NotInEvents() )
	{
		return true;
	}

	return false;
}

// These will all eventually lead to a message being sent up to the cloud.
void L3Message::SCKEventHandleEvent( COMMS_EVENT * RxEvent )
{
	printf("SCKEventHandleEvent %d\n", RxEvent->GetOID());

	// if the event is supported, then pass it on up to the next layer
	if( SCKEventCheckIncomingEventIsSupported(RxEvent->GetOID()) )
	{
		//printf("----->> COMMS_EVENT %d, %d : is supported\n", RxEvent->GetOID(), RxEvent->GetData());
		Write(RxEvent, MSG_L4_From_L3);
	}
	else
	{
		printf("----->> COMMS_EVENT %d, %d : is NOT supported\n", RxEvent->GetOID(), RxEvent->GetData());
	}
}

