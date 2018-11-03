/*
 * L3CommEvents.h
 *
 *  Created on: Sep 8, 2018
 *      Author: ds
 */

#ifndef L3COMMEVENTS_H_
#define L3COMMEVENTS_H_

#include <string>
#include <cstring>
#include <map>

// VC210 event table
class EVENT_TABLE
{

public:
	// oid definitions
	enum EVENT_OIDS : uint16_t
	{
		COOK_START=                          2000,
		COOK_COMPLETE=                       2001,
		COOK_COMPLETE_ACK_=                  2002,
		COOK_CANCEL=                         2003,
		COOK_PREALARM=                       2005,
		HOLD_START=                          2006,
		HOLD_CANCEL=                         2007,
		HOLD_COMPLETE=                       2008,
		HOLD_COMPLETE_ACK_=                  2009,
		VAT_STATE=                           2027,
		RIGHT_VAT_STATE=                     2028,
		TEMPERATURE=                         2032,
		RIGHT_TEMPERATURE=                   2033,
		SET_TEMPERATURE=                     2037,
		SET_RIGHT_TEMPERATURE =              2038,
		VAT_STATE_MISC=                      2042,
		RIGHT_VAT_STATE_MISC=                2043,
		POLISH_FINISHED=                     2050,
		POLISH_START=                        2051,
		POLISH_FORCED_START=                 2052,
		POLISH_FORCED_CANCEL=                2053,
		COOK_QUANTITY=                       2056,
		VAT_STATE_MISC_2=                    2308,
		VAT_STATE_RIGHT_MISC_2=              2309,
		VAT_ERROR=                           2313,
		VAT_ERROR_RIGHT=                     2314,
	};

	typedef std::map< EVENT_OIDS, std::string > eventMap;
	typedef eventMap::iterator eventItr;

	eventMap * GetEventMap() 						{ return & m_em; }
	eventItr FindEvent( EVENT_OIDS oid )			{ return GetEventMap()->find( oid );	}
	eventItr NotInEvents()							{ return GetEventMap()->end(); }

	std::string GetEventString( EVENT_OIDS oid )	{ return FindEvent(oid)->second; }

	static  eventMap m_em;
};

// VC210 response table
class RESPONSE_TABLE
{

public:
	// oid definitions
	enum RESPONSE_OIDS : uint16_t
	{
		FLASH_NUMBER=                     589,
		SOFTWARE_NUMBER=                  410,
		DOWNLOAD_NUMBER=                  285,
		//PROTOCOL_VERSION=                 ???,
	};

	typedef std::map< RESPONSE_OIDS, std::string > responseMap;
	typedef responseMap::iterator responseItr;

	responseMap * GetResponseMap()						{ return & m_rm; }
	responseItr FindResponse( RESPONSE_OIDS oid )		{ return GetResponseMap()->find( oid );	}
	responseItr NotInResponse()							{ return GetResponseMap()->end(); }

	std::string GetResponseString( RESPONSE_OIDS oid )	{ return FindResponse(oid)->second; }

	static  responseMap m_rm;
};

class VAT_STATE_TABLE
{

public:
	// states definitions
	enum VAT_STATES : uint16_t
	{
		STATE_OFF = 0,
		STATE_MELT,
		STATE_IDLE,
		STATE_COOK ,
		STATE_BOIL,
		STATE_HOLD,
		STATE_FILTER,
		STATE_COOL,
		STATE_SETBACK,
		STATE_FAULT,
		STATE_10,
		STATE_POLISH,
		STATE_12,
		STATE_13,
		STATE_14,
		STATE_DISPOSAL,
		STATE_16,
		STATE_FILL,
		STATE_AUTO_COOL,
		STATE_19,
		STATE_20,
		STATE_21,
		STATE_22,
		STATE_CLEAN,
		STATE_PROGRAM,
		STATE_STANDBY,
		STATE_ON ,
		STATE_DIAG,
		STATE_LOCK_OUT_BY_SPM=100
	};

	enum VAT_STATES_MISC : uint16_t
	{
		STATE_MISC_NORMAL = 0,
		STATE_MISC_TEMP_LOW,
		STATE_MISC_TEMP_HIGH,
		STATE_MISC_COOK_ALARM,
		STATE_MISC_COOK_DONE,
		STATE_MISC_HOLD_DONE,
		STATE_MISC_HOLD_ALARM,
		STATE_MISC_NONE = 0xFF,
	};

	enum VAT_STATES_MISC_2 : uint16_t
	{
		STATE_MISC_2_NORMAL = 0,
		STATE_MISC_2_DOOR_OPEN_HALF,
		STATE_MISC_2_DOOR_OPEN_FULL,
		STATE_MISC_2_BLOWER_MOTOR,
		STATE_MISC_2_NONE = 0xFF
	};

	enum VAT_STATES_ERROR : uint16_t
	{
		STATE_ERROR_NORMAL = 0,
		STATE_ERROR_WATER_DETECT,
		STATE_ERROR_NO_HEAT,
		STATE_ERROR_PROBE_ERROR,
		STATE_ERROR_GAS_ERROR,
		STATE_ERROR_HIGH_TEMP_ERROR,
		STATE_ERROR_AIRSWITCH_ERROR,
		STATE_ERROR_IGNITION_ERROR,
		STATE_ERROR_DRAIN_OPEN,
		STATE_ERROR_RECOVERY_ERROR,
		STATE_ERROR_MOTOR_ERROR,
		STATE_ERROR_IR_BURNER_ERROR,
		STATE_ERROR_OTHER = 254,
		STATE_ERROR_NONE = 0xFF
	};

	// I really should do all of this with templates.

	typedef std::map< VAT_STATES, std::string > vatStateMap;
	typedef vatStateMap::iterator vatStateMapItr;

	typedef std::map< VAT_STATES_MISC, std::string > vatStateMiscMap;
	typedef vatStateMiscMap::iterator vatStateMiscMapItr;

	typedef std::map< VAT_STATES_MISC_2, std::string > vatStateMisc2Map;
	typedef vatStateMisc2Map::iterator vatStateMisc2MapItr;

	typedef std::map< VAT_STATES_ERROR, std::string > vatStateErrorMap;
	typedef vatStateErrorMap::iterator vatStateErrorMapItr;

	vatStateMap * GetStateMap()							{ return & m_statMap; }
	vatStateMapItr FindState( VAT_STATES state )		{ return GetStateMap()->find( state );	}
	vatStateMapItr NotInState()							{ return GetStateMap()->end(); }
	std::string GetStateString( VAT_STATES state )		{ return FindState(state)->second; }

	vatStateMiscMap * GetStateMiscMap()						{ return & m_stateMiscMap; }
	vatStateMiscMapItr FindState( VAT_STATES_MISC state )	{ return GetStateMiscMap()->find( state );	}
	vatStateMiscMapItr NotInMiscState()						{ return GetStateMiscMap()->end(); }
	std::string GetStateString( VAT_STATES_MISC state )		{ return FindState(state)->second; }

	vatStateMisc2Map * GetStateMisc2Map()						{ return & m_stateMisc2Map; }
	vatStateMisc2MapItr FindState( VAT_STATES_MISC_2 state )	{ return GetStateMisc2Map()->find( state );	}
	vatStateMisc2MapItr NotInMisc2State()						{ return GetStateMisc2Map()->end(); }
	std::string GetStateString( VAT_STATES_MISC_2 state )		{ return FindState(state)->second; }

	vatStateErrorMap * GetStateErrorMap()						{ return & m_stateErrorMap; }
	vatStateErrorMapItr FindState( VAT_STATES_ERROR state )		{ return GetStateErrorMap()->find( state );	}
	vatStateErrorMapItr NotInErrorState()						{ return GetStateErrorMap()->end(); }
	std::string GetStateString( VAT_STATES_ERROR state )		{ return FindState(state)->second; }

	static  vatStateMap m_statMap;
	static  vatStateMiscMap m_stateMiscMap;
	static  vatStateMisc2Map m_stateMisc2Map;
	static  vatStateErrorMap m_stateErrorMap;
};


#endif /* L3COMMEVENTS_H_ */
