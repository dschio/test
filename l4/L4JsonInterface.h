/*
 * L4JsonInterface.h
 *
 *  Created on: Sep 2, 2018
 *      Author: ds
 */

#ifndef L4JSONINTERFACE_H_
#define L4JSONINTERFACE_H_

//#define USE_STRING_CORRID_INSTEAD

#include "jsonInterface.h"
#include "L3Comm.h"
#include "utilities.h"
#include <atomic>
#include <mutex>
#include <sstream>
#include <inttypes.h>

extern int getLQI();

using namespace std;

#define MAX_RECIPES			20
#define CLOUD_RETRY_TIME	3000//0	// msec
#define CLOUD_RETRY_COUNT	2		// original + 2 retries

extern class EVENT_TABLE g_EventTable;
extern class RESPONSE_TABLE g_ResponseTable;
extern class VAT_STATE_TABLE g_VatStateTable;

static array<atomic<uint64_t>,MAX_RECIPES> s_currentCookCorrelation;
static array<atomic<uint64_t>,MAX_RECIPES> s_currentHoldCorrelation;
static array<atomic<uint64_t>,MAX_RECIPES> s_currentPolishCorrelation;

class STATUS_CORRELATION_ID
{
public:
	STATUS_CORRELATION_ID()
	{
	}

	STATUS_CORRELATION_ID( string s )
	{
		SetCorrId( s );
	}

#ifdef USE_STRING_CORRID_INSTEAD
	string GetCorrId()
	{
		std::lock_guard < std::mutex > lck(g_currentStatusCorrelationMtx);
		return s_currentStatusCorrelation;
	}
	string GetCorrIdStr()
	{
		return GetCorrId();
	}
	void SetCorrId( string s )
	{
		std::lock_guard < std::mutex > lck(g_currentStatusCorrelationMtx);
		s_currentStatusCorrelation = s;
	}
#else
	uint64_t GetCorrId()
	{
		return s_currentStatusCorrelation;
	}

	string GetCorrIdStr()
	{
		stringstream ss;
		ss << hex << s_currentStatusCorrelation;
		return ss.str();
	}

	void SetCorrId( string s )
	{
		s_currentStatusCorrelation = stoul(s,nullptr,16);
	}
#endif

private:
#ifdef USE_STRING_CORRID_INSTEAD
	static string s_currentStatusCorrelation;
	static mutex g_currentStatusCorrelationMtx;
#else
	static atomic<uint64_t> s_currentStatusCorrelation;
#endif
};

class L4JsonInterface: public JsonInterface
{
public:
	L4JsonInterface()
	{
	}
	~L4JsonInterface()
	{
	}

	virtual void processJsonMessage() {}
private:
};

// the base class that all messages derive from.
// sets the basic parameters and builds the basic JSON message.
class JSON_MESSAGE_BASE
{
public:
	JSON_MESSAGE_BASE()
	{
	}
	JSON_MESSAGE_BASE( string t )
	{
		type = t;
	}
	virtual ~JSON_MESSAGE_BASE()
	{
	}

	void ChangeType( string t )
	{
		type = t;
		L4Json.Set("/type", type.c_str());
	}

	virtual void buildMessage()
	{
		time_t rawtime;
		struct tm * timeinfo;
		char buffer[80];

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(buffer, 80, "%FT%T%z", timeinfo);
		dateTime = buffer;

		sprintf(buffer, "%" PRIx64, guidHash64 = guidHash());
		guid = buffer;

		uint64_t mac = getMac();
		sprintf(buffer, "%02x:%02x:%02x:%02x:%02x:%02x",
				(uint16_t) ((mac >> 0) & 0xff),
				(uint16_t) ((mac >> 8) & 0xff),
				(uint16_t) ((mac >> 16) & 0xff),
				(uint16_t) ((mac >> 24) & 0xff),
				(uint16_t) ((mac >> 32) & 0xff),
				(uint16_t) ((mac >> 40) & 0xff)
				);
		source = buffer;

		mac = 0;
		sprintf(buffer, "%02x:%02x:%02x:%02x:%02x:%02x",
				(uint16_t) ((mac >> 0) & 0xff),
				(uint16_t) ((mac >> 8) & 0xff),
				(uint16_t) ((mac >> 16) & 0xff),
				(uint16_t) ((mac >> 24) & 0xff),
				(uint16_t) ((mac >> 32) & 0xff),
				(uint16_t) ((mac >> 40) & 0xff)
				);
		destination = buffer;

		lqi = getLQI();

		// start building the json message with the common fields
		L4Json.Set("/guid", guid.c_str());
		L4Json.Set("/sourceId", source.c_str());
		L4Json.Set("/destinationId", destination.c_str());
		L4Json.Set("/dateTime", dateTime.c_str());
		L4Json.Set("/LQI", lqi);
		L4Json.Set("/type", type.c_str());

	}

	string GetOutput( int * size = NULL )
	{
		return L4Json.RetrieveJsonFormatMessage(size, USE_PRETTY);
	}

	void Print()
	{
		cout << GetOutput() << endl;
	}

	uint64_t GetGuid()
	{
		return guidHash64;
	}


	bool Parse( const char * cc )
	{
		return L4Json.Parse( cc );
	}

protected:
	string guid;
	string source;
	string destination;
	string dateTime;
	string type;
	int lqi;

	uint64_t guidHash64;

	L4JsonInterface L4Json;
};

/////////////////////////////////////////////////////
// Cook class messages
class JSON_MESSAGE_COOK: public JSON_MESSAGE_BASE
{
public:
	JSON_MESSAGE_COOK( string t ) :
			JSON_MESSAGE_BASE(t)
	{
		//cout << "doing the cook class" << endl;
	}

	JSON_MESSAGE_COOK( string t, int recipe ) :
			JSON_MESSAGE_BASE(t)
	{
		//cout << "doing the cook class" << endl;

		buildMessage( recipe );
	}
	~JSON_MESSAGE_COOK()
	{
	}

	void buildMessage( int recipe )
	{
		JSON_MESSAGE_BASE::buildMessage();

		if( recipe >= MAX_RECIPES )
			return;

		recipeInstanceId = recipe;
		recipeName = "whoKnows";

		L4Json.Set("/recipeInstanceId", recipeInstanceId);
		L4Json.Set("/recipeName", recipeName.c_str());
	}


	void SetCorrelationId( bool newId = false )
	{
		char buffer[80];

		if( recipeInstanceId >= MAX_RECIPES )
			return;

		uint64_t corrId;
		if( newId )
		{
			s_currentCookCorrelation[recipeInstanceId] = corrId = GetGuid();
		}
		else
		{
			corrId = s_currentCookCorrelation[recipeInstanceId];
		}

		sprintf(buffer, "%" PRIx64, corrId);
		correlationId = buffer;

		L4Json.Set("/correlationId", correlationId.c_str());
	}

private:
	string correlationId;
	int recipeInstanceId;
	string recipeName;
};

class JSON_MESSAGE_COOK_START: public JSON_MESSAGE_COOK
{
public:
	JSON_MESSAGE_COOK_START( int recipe ) :
			JSON_MESSAGE_COOK(g_EventTable.GetEventString(EVENT_TABLE::EVENT_OIDS::COOK_START), recipe )
	{
		// creating this message creates a new guid hash and therefore a new correlationId for this recipe.
		// All subsequent cook events on this recipe will correlate back to this.
		//
		SetCorrelationId(true);
	}
	~JSON_MESSAGE_COOK_START()
	{
	}
};

class JSON_MESSAGE_COOK_COMPLETE: public JSON_MESSAGE_COOK
{
public:
	JSON_MESSAGE_COOK_COMPLETE( int recipe ) :
			JSON_MESSAGE_COOK(g_EventTable.GetEventString(EVENT_TABLE::EVENT_OIDS::COOK_COMPLETE), recipe)
	{
		SetCorrelationId();
	}
	~JSON_MESSAGE_COOK_COMPLETE()
	{
	}
};

class JSON_MESSAGE_COOK_CANCEL: public JSON_MESSAGE_COOK
{
public:
	JSON_MESSAGE_COOK_CANCEL( int recipe ) :
			JSON_MESSAGE_COOK(g_EventTable.GetEventString(EVENT_TABLE::EVENT_OIDS::COOK_CANCEL), recipe)
	{
		SetCorrelationId();
	}
	~JSON_MESSAGE_COOK_CANCEL()
	{
	}
};

class JSON_MESSAGE_COOK_PREALARM: public JSON_MESSAGE_COOK
{
public:
	JSON_MESSAGE_COOK_PREALARM( int recipe ) :
			JSON_MESSAGE_COOK(g_EventTable.GetEventString(EVENT_TABLE::EVENT_OIDS::COOK_PREALARM), recipe)
	{
		SetCorrelationId();
	}
	~JSON_MESSAGE_COOK_PREALARM()
	{
	}
};

class JSON_MESSAGE_COOK_QUANTITY: public JSON_MESSAGE_COOK
{
public:
	JSON_MESSAGE_COOK_QUANTITY( int recipeAndQuantity ) :
			JSON_MESSAGE_COOK(g_EventTable.GetEventString(EVENT_TABLE::EVENT_OIDS::COOK_QUANTITY) )
	{
		int recipe = (recipeAndQuantity >> 0) & 0xff;
		JSON_MESSAGE_COOK::buildMessage(recipe);

		quantity = (recipeAndQuantity >> 8) & 0xff;
		IncludeQuantity(quantity);
		SetCorrelationId();
	}
	~JSON_MESSAGE_COOK_QUANTITY()
	{
	}

private:
	void IncludeQuantity( int quantity )
	{
		L4Json.Set("/quantity", quantity);
	};

private:
	int quantity;
};

class JSON_MESSAGE_COOK_COMPLETE_ACK: public JSON_MESSAGE_COOK
{
public:
	JSON_MESSAGE_COOK_COMPLETE_ACK( int recipe ) :
			JSON_MESSAGE_COOK(g_EventTable.GetEventString(EVENT_TABLE::EVENT_OIDS::COOK_COMPLETE_ACK_), recipe)
	{
		SetCorrelationId();
	}
	~JSON_MESSAGE_COOK_COMPLETE_ACK()
	{
	}
};
// End cook class messages
/////////////////////////////////////////////////////

// Hold class messages
class JSON_MESSAGE_HOLD: public JSON_MESSAGE_BASE
{
public:
	JSON_MESSAGE_HOLD( string t ) :
			JSON_MESSAGE_BASE(t)
	{
		//cout << "doing the hold class" << endl;
		vatNumber =  0;
	}

	JSON_MESSAGE_HOLD( string t, int recipe ) :
			JSON_MESSAGE_BASE(t)
	{
		//cout << "doing the hold class" << endl;
		vatNumber = 0;
		buildMessage( recipe );
	}
	~JSON_MESSAGE_HOLD()
	{
	}

	void buildMessage( int recipe )
	{
		JSON_MESSAGE_BASE::buildMessage();

		if( recipe >= MAX_RECIPES )
			return;

		holdInstanceId = recipe;
		recipeName = "whoKnows";

		L4Json.Set("/holdInstanceId", holdInstanceId);
		L4Json.Set("/recipeName", recipeName.c_str());
		L4Json.Set("/vatNum", vatNumber );
	}

	void SetCorrelationId( bool newId = false )
	{
		char buffer[80];

		if( holdInstanceId >= MAX_RECIPES )
			return;

		uint64_t corrId;
		if( newId )
		{
			s_currentHoldCorrelation[holdInstanceId] = corrId = GetGuid();
		}
		else
		{
			corrId = s_currentHoldCorrelation[holdInstanceId];
		}

		sprintf(buffer, "%" PRIx64, corrId);
		correlationId = buffer;

		L4Json.Set("/correlationId", correlationId.c_str());
	}

private:
	string correlationId;
	int holdInstanceId;
	string recipeName;
	int vatNumber;

};

class JSON_MESSAGE_HOLD_START: public JSON_MESSAGE_HOLD
{
public:
	JSON_MESSAGE_HOLD_START( int recipe ) :
			JSON_MESSAGE_HOLD(g_EventTable.GetEventString(EVENT_TABLE::EVENT_OIDS::HOLD_START), recipe )
	{
		// creating this message creates a new guid hash and therefore a new correlationId for this recipe.
		// All subsequent cook events on this recipe will correlate back to this.
		//
		SetCorrelationId(true);

		holdTime = 0;
		displayTime = 0;
	}
	~JSON_MESSAGE_HOLD_START()
	{
	}

	void buildMessage( int recipe )
	{
		JSON_MESSAGE_HOLD::buildMessage( recipe );

		if( recipe >= MAX_RECIPES )
			return;

		L4Json.Set( "/holdTime", holdTime );
		L4Json.Set( "/displayTime", displayTime );
	}


private:
	int holdTime;
	int displayTime;

};

class JSON_MESSAGE_HOLD_COMPLETE: public JSON_MESSAGE_HOLD
{
public:
	JSON_MESSAGE_HOLD_COMPLETE( int recipe ) :
		JSON_MESSAGE_HOLD(g_EventTable.GetEventString(EVENT_TABLE::EVENT_OIDS::HOLD_COMPLETE), recipe)
	{
		SetCorrelationId();
	}
	~JSON_MESSAGE_HOLD_COMPLETE()
	{
	}
};

class JSON_MESSAGE_HOLD_CANCEL: public JSON_MESSAGE_HOLD
{
public:
	JSON_MESSAGE_HOLD_CANCEL( int recipe ) :
		JSON_MESSAGE_HOLD(g_EventTable.GetEventString(EVENT_TABLE::EVENT_OIDS::HOLD_CANCEL), recipe)
	{
		SetCorrelationId();
		buildMessage( recipe );
	}
	~JSON_MESSAGE_HOLD_CANCEL()
	{
	}

	void buildMessage( int recipe )
	{
		JSON_MESSAGE_HOLD::buildMessage( recipe );

		if( recipe >= MAX_RECIPES )
			return;

		L4Json.Set( "/holdTime", holdTime );
		L4Json.Set( "/remainingTime", remainingHoldTime );
	}

private:
	int remainingHoldTime;
	int holdTime;
};

class JSON_MESSAGE_HOLD_COMPLETE_ACK: public JSON_MESSAGE_HOLD
{
public:
	JSON_MESSAGE_HOLD_COMPLETE_ACK( int recipe ) :
		JSON_MESSAGE_HOLD(g_EventTable.GetEventString(EVENT_TABLE::EVENT_OIDS::HOLD_COMPLETE_ACK_), recipe)
	{
		SetCorrelationId();
		buildMessage( recipe );
	}
	~JSON_MESSAGE_HOLD_COMPLETE_ACK()
	{
	}
};


// End hold class messages
/////////////////////////////////////////////////////

// polish class messages
class JSON_MESSAGE_POLISH: public JSON_MESSAGE_BASE
{
public:
	JSON_MESSAGE_POLISH( string t ) :
			JSON_MESSAGE_BASE(t)
	{
		//cout << "doing the hold class" << endl;
	}

	JSON_MESSAGE_POLISH( string t, int recipe ) :
			JSON_MESSAGE_BASE(t)
	{
		//cout << "doing the hold class" << endl;

		buildMessage( recipe );
	}
	~JSON_MESSAGE_POLISH()
	{
	}

	void buildMessage( int recipe )
	{
		JSON_MESSAGE_BASE::buildMessage();

		if( recipe >= MAX_RECIPES )
			return;

		recipeInstanceId = recipe;
		recipeName = "whoKnows";

		L4Json.Set("/recipeInstanceId", recipeInstanceId);
		L4Json.Set("/recipeName", recipeName.c_str());
	}

	void SetCorrelationId( bool newId = false )
	{
		char buffer[80];

		if( recipeInstanceId >= MAX_RECIPES )
			return;

		uint64_t corrId;
		if( newId )
		{
			s_currentPolishCorrelation[recipeInstanceId] = corrId = GetGuid();
		}
		else
		{
			corrId = s_currentPolishCorrelation[recipeInstanceId];
		}

		sprintf(buffer, "%" PRIx64, corrId);
		correlationId = buffer;

		L4Json.Set("/correlationId", correlationId.c_str());
	}

private:
	string correlationId;
	int recipeInstanceId;
	string recipeName;
};

class JSON_MESSAGE_POLISH_START: public JSON_MESSAGE_POLISH
{
public:
	JSON_MESSAGE_POLISH_START( int recipe ) :
		JSON_MESSAGE_POLISH(g_EventTable.GetEventString(EVENT_TABLE::EVENT_OIDS::POLISH_START), recipe )
	{
		// creating this message creates a new guid hash and therefore a new correlationId for this recipe.
		// All subsequent cook events on this recipe will correlate back to this.
		//
		SetCorrelationId(true);
	}
	~JSON_MESSAGE_POLISH_START()
	{
	}
};

class JSON_MESSAGE_POLISH_FINISHED: public JSON_MESSAGE_POLISH
{
public:
	JSON_MESSAGE_POLISH_FINISHED( int recipe ) :
		JSON_MESSAGE_POLISH(g_EventTable.GetEventString(EVENT_TABLE::EVENT_OIDS::POLISH_FINISHED), recipe)
	{
		SetCorrelationId();
	}
	~JSON_MESSAGE_POLISH_FINISHED()
	{
	}
};

class JSON_MESSAGE_FORCED_START: public JSON_MESSAGE_POLISH
{
public:
	JSON_MESSAGE_FORCED_START( int recipe ) :
		JSON_MESSAGE_POLISH(g_EventTable.GetEventString(EVENT_TABLE::EVENT_OIDS::POLISH_FORCED_START), recipe)
	{
		SetCorrelationId();
	}
	~JSON_MESSAGE_FORCED_START()
	{
	}
};

class JSON_MESSAGE_FORCED_CANCEL: public JSON_MESSAGE_POLISH
{
public:
	JSON_MESSAGE_FORCED_CANCEL( int recipe ) :
		JSON_MESSAGE_POLISH(g_EventTable.GetEventString(EVENT_TABLE::EVENT_OIDS::POLISH_FORCED_CANCEL), recipe)
	{
		SetCorrelationId();
	}
	~JSON_MESSAGE_FORCED_CANCEL()
	{
	}
};
// End polish class messages
/////////////////////////////////////////////////////

// vat state class messages
class JSON_MESSAGE_VAT_STATE: public JSON_MESSAGE_BASE
{
public:
	JSON_MESSAGE_VAT_STATE( int state ) :
		JSON_MESSAGE_BASE( "Vat State" )
	{
		m_state = state;
		buildMessage();
	}

	JSON_MESSAGE_VAT_STATE( int state, string type ) :
		JSON_MESSAGE_BASE( type )
	{
		m_state = state;
		buildMessage();
	}

	~JSON_MESSAGE_VAT_STATE() {}


	void buildMessage()
	{
		JSON_MESSAGE_BASE::buildMessage();

		VAT_STATE_TABLE::vatStateMapItr  itr;

		if( (itr = g_VatStateTable.FindState( (VAT_STATE_TABLE::VAT_STATES)m_state) ) == g_VatStateTable.NotInState() )
		{
			L4Json.Set("/state", "UNKNOWN" );
			return;
		}

		L4Json.Set("/state",  itr->second.c_str() );
	}
private:
	int m_state;
};

class JSON_MESSAGE_RIGHT_VAT_STATE: public JSON_MESSAGE_VAT_STATE
{
public:
	JSON_MESSAGE_RIGHT_VAT_STATE( int state ) :
		JSON_MESSAGE_VAT_STATE( state, "Right Vat State" )
	{
	}

	~JSON_MESSAGE_RIGHT_VAT_STATE() {}

};

class JSON_MESSAGE_VAT_STATE_MISC: public JSON_MESSAGE_BASE
{
public:
	JSON_MESSAGE_VAT_STATE_MISC( int state ) :
		JSON_MESSAGE_BASE( "Vat Misc State" )
	{
		m_state = state;
		buildMessage();
	}

	JSON_MESSAGE_VAT_STATE_MISC( int state, string type ) :
		JSON_MESSAGE_BASE( type )
	{
		m_state = state;
		buildMessage();
	}

	~JSON_MESSAGE_VAT_STATE_MISC() {}


	void buildMessage()
	{
		JSON_MESSAGE_BASE::buildMessage();

		VAT_STATE_TABLE::vatStateMiscMapItr  itr;

		if( (itr = g_VatStateTable.FindState( (VAT_STATE_TABLE::VAT_STATES_MISC)m_state) ) == g_VatStateTable.NotInMiscState() )
		{
			L4Json.Set("/state", "UNKNOWN" );
			return;
		}

		L4Json.Set("/state",  itr->second.c_str() );
	}
private:
	int m_state;
};

class JSON_MESSAGE_RIGHT_VAT_STATE_MISC: public JSON_MESSAGE_VAT_STATE_MISC
{
public:
	JSON_MESSAGE_RIGHT_VAT_STATE_MISC( int state ) :
		JSON_MESSAGE_VAT_STATE_MISC( state, "Right Vat Misc State" )
	{
	}

	~JSON_MESSAGE_RIGHT_VAT_STATE_MISC() {}

};

class JSON_MESSAGE_VAT_STATE_MISC_2: public JSON_MESSAGE_BASE
{
public:
	JSON_MESSAGE_VAT_STATE_MISC_2( int state ) :
		JSON_MESSAGE_BASE( "Vat Misc State 2" )
	{
		m_state = state;
		buildMessage();
	}

	JSON_MESSAGE_VAT_STATE_MISC_2( int state, string type ) :
		JSON_MESSAGE_BASE( type )
	{
		m_state = state;
		buildMessage();
	}

	~JSON_MESSAGE_VAT_STATE_MISC_2() {}


	void buildMessage()
	{
		JSON_MESSAGE_BASE::buildMessage();

		VAT_STATE_TABLE::vatStateMisc2MapItr  itr;

		if( (itr = g_VatStateTable.FindState( (VAT_STATE_TABLE::VAT_STATES_MISC_2)m_state) ) == g_VatStateTable.NotInMisc2State() )
		{
			L4Json.Set("/state", "UNKNOWN" );
			return;
		}

		L4Json.Set("/state",  itr->second.c_str() );
	}
private:
	int m_state;
};

class JSON_MESSAGE_RIGHT_VAT_STATE_MISC_2: public JSON_MESSAGE_VAT_STATE_MISC
{
public:
	JSON_MESSAGE_RIGHT_VAT_STATE_MISC_2( int state ) :
		JSON_MESSAGE_VAT_STATE_MISC( state, "Right Vat Misc State 2" )
	{
	}

	~JSON_MESSAGE_RIGHT_VAT_STATE_MISC_2() {}

};


class JSON_MESSAGE_VAT_ERROR: public JSON_MESSAGE_BASE
{
public:
	JSON_MESSAGE_VAT_ERROR( int state ) :
		JSON_MESSAGE_BASE( "Vat Error" )
	{
		m_state = state;
		buildMessage();
	}

	JSON_MESSAGE_VAT_ERROR( int state, string type ) :
		JSON_MESSAGE_BASE( type )
	{
		m_state = state;
		buildMessage();
	}

	~JSON_MESSAGE_VAT_ERROR() {}


	void buildMessage()
	{
		JSON_MESSAGE_BASE::buildMessage();

		VAT_STATE_TABLE::vatStateErrorMapItr  itr;

		if( (itr = g_VatStateTable.FindState( (VAT_STATE_TABLE::VAT_STATES_ERROR)m_state) ) == g_VatStateTable.NotInErrorState() )
		{
			L4Json.Set("/error", "UNKNOWN" );
			return;
		}

		L4Json.Set("/error",  itr->second.c_str() );
	}
private:
	int m_state;
};

class JSON_MESSAGE_RIGHT_VAT_ERROR: public JSON_MESSAGE_VAT_STATE_MISC
{
public:
	JSON_MESSAGE_RIGHT_VAT_ERROR( int state ) :
		JSON_MESSAGE_VAT_STATE_MISC( state, "Right Vat Error" )
	{
	}

	~JSON_MESSAGE_RIGHT_VAT_ERROR() {}
};
/////////////////////////////////////////////////////

// temperature class messages
class JSON_MESSAGE_TEMPERATURE: public JSON_MESSAGE_BASE
{
public:
	JSON_MESSAGE_TEMPERATURE( int temperature ) :
		JSON_MESSAGE_BASE( "Temperature" )
	{
		m_temp = temperature;
		buildMessage();
	}

	~JSON_MESSAGE_TEMPERATURE() {}

	void buildMessage()
	{
		JSON_MESSAGE_BASE::buildMessage();
		L4Json.Set("/temperature", m_temp );
	}

	uint16_t m_temp;
};

class JSON_MESSAGE_RIGHT_TEMPERATURE: public JSON_MESSAGE_BASE
{
public:
	JSON_MESSAGE_RIGHT_TEMPERATURE( int temperature ) :
		JSON_MESSAGE_BASE( "Right Temperature" )
	{
		m_temp = temperature;
		buildMessage();
	}

	~JSON_MESSAGE_RIGHT_TEMPERATURE() {}

	void buildMessage()
	{
		JSON_MESSAGE_BASE::buildMessage();
		L4Json.Set("/temperature", m_temp );
	}

	uint16_t m_temp;
};

class JSON_MESSAGE_SET_TEMPERATURE: public JSON_MESSAGE_BASE
{
public:
	JSON_MESSAGE_SET_TEMPERATURE( int temperature ) :
		JSON_MESSAGE_BASE( "Set Temperature" )
	{
		m_temp = temperature;
		buildMessage();
	}

	~JSON_MESSAGE_SET_TEMPERATURE() {}

	void buildMessage()
	{
		JSON_MESSAGE_BASE::buildMessage();
		L4Json.Set("/temperature", m_temp );
	}

	uint16_t m_temp;
};

class JSON_MESSAGE_RIGHT_SET_TEMPERATURE: public JSON_MESSAGE_BASE
{
public:
	JSON_MESSAGE_RIGHT_SET_TEMPERATURE( int temperature ) :
		JSON_MESSAGE_BASE( "Right Set Temperature" )
	{
		m_temp = temperature;
		buildMessage();
	}

	~JSON_MESSAGE_RIGHT_SET_TEMPERATURE() {}

	void buildMessage()
	{
		JSON_MESSAGE_BASE::buildMessage();
		L4Json.Set("/temperature", m_temp );
	}

	uint16_t m_temp;
};

/////////////////////////
// responses
////////////////////////
class JSON_MESSAGE_CONFIG_RESPONSE: public JSON_MESSAGE_BASE
{
public:
	JSON_MESSAGE_CONFIG_RESPONSE( string t ) :
			JSON_MESSAGE_BASE(t)
	{
		//cout << "doing the config response class" << endl;
		JSON_MESSAGE_BASE::buildMessage();
	}

	~JSON_MESSAGE_CONFIG_RESPONSE()
	{
	}

	void SetCorrelationId( bool newId = false )
	{
		STATUS_CORRELATION_ID sId;
		L4Json.Set("/correlationId", sId.GetCorrIdStr().c_str());
	}

	void buildMessage( FASTLAN_RESPONSE_MSG * msg )
	{
		// we have to build up the json message from the disjointed data that we get from the vc210.
		// I have no choice at the moment but to blatantly hard code this.
		//
		// there are 5 unsigned ints in the message
		if( msg->GetCount() == 5 )
		{
			uint16_t * m = (uint16_t *) msg->GetData();

			char buffer[80];
			sprintf( buffer, "%d-%d%d-%d ECL-%c",
					htons(m[4]),
					htons(m[3]),
					htons(m[0]),
					htons(m[1]),
					htons(m[2])
					);
			L4Json.Set("/response", buffer);
		}
	}

private:
	string correlationId;
};

class JSON_MESSAGE_CONFIG_FLASH_NUMBER: public JSON_MESSAGE_CONFIG_RESPONSE
{
public:
	JSON_MESSAGE_CONFIG_FLASH_NUMBER( FASTLAN_RESPONSE_MSG * msg ) :
		JSON_MESSAGE_CONFIG_RESPONSE(g_ResponseTable.GetResponseString(RESPONSE_TABLE::RESPONSE_OIDS::FLASH_NUMBER) )
	{
		SetCorrelationId();
		buildMessage( msg );
	}
	~JSON_MESSAGE_CONFIG_FLASH_NUMBER()
	{
	}

};

class JSON_MESSAGE_CONFIG_SOFTWARE_NUMBER: public JSON_MESSAGE_CONFIG_RESPONSE
{
public:
	JSON_MESSAGE_CONFIG_SOFTWARE_NUMBER( FASTLAN_RESPONSE_MSG * msg ) :
		JSON_MESSAGE_CONFIG_RESPONSE(g_ResponseTable.GetResponseString(RESPONSE_TABLE::RESPONSE_OIDS::SOFTWARE_NUMBER) )
	{
		SetCorrelationId();
		buildMessage( msg );
	}
	~JSON_MESSAGE_CONFIG_SOFTWARE_NUMBER()
	{
	}

};

class JSON_MESSAGE_CONFIG_DOWNLOAD_NUMBER: public JSON_MESSAGE_CONFIG_RESPONSE
{
public:
	JSON_MESSAGE_CONFIG_DOWNLOAD_NUMBER( FASTLAN_RESPONSE_MSG * msg ) :
		JSON_MESSAGE_CONFIG_RESPONSE(g_ResponseTable.GetResponseString(RESPONSE_TABLE::RESPONSE_OIDS::DOWNLOAD_NUMBER) )
	{
		SetCorrelationId();
		buildMessage( msg );
	}
	~JSON_MESSAGE_CONFIG_DOWNLOAD_NUMBER()
	{
	}
};

#endif /* L4JSONINTERFACE_H_ */
