/*
 * config.cpp
 *
 *  Created on: Nov 2, 2018
 *      Author: ds
 */

#include "config.h"


bool BezelConfig::CreateInitJson( FILE * fp )
{
	Config d3;

	d3.Set("/brokerIp", "192.168.2.104");
	d3.Set("/2000", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2001", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2002", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2003", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2005", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2006", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2007", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2008", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2009", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2027", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2028", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2032", MSG_OUTBOUND_ENVIRONMENT);
	d3.Set("/2033", MSG_OUTBOUND_ENVIRONMENT);
	d3.Set("/2037", MSG_OUTBOUND_ENVIRONMENT);
	d3.Set("/2038", MSG_OUTBOUND_ENVIRONMENT);
	d3.Set("/2042", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2043", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2050", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2051", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2052", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2053", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2056", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2308", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2309", MSG_OUTBOUND_WORKFLOW);
	d3.Set("/2313", MSG_OUTBOUND_ERROR);
	d3.Set("/2314", MSG_OUTBOUND_ERROR);
	d3.Set("/589",  MSG_OUTBOUND_RESPONSE);
	d3.Set("/410",  MSG_OUTBOUND_RESPONSE);
	d3.Set("/285",  MSG_OUTBOUND_RESPONSE);

//	d3.Set("/2000/1", "COOK START");
//	d3.Set("/2001/1", "COOK COMPLETE");
//	d3.Set("/2002/1", "COOK COMPLETE ACK");
//	d3.Set("/2003/1", "COOK CANCEL");
//	d3.Set("/2005/1", "COOK PREALARM");
//	d3.Set("/2006/1", "HOLD START");
//	d3.Set("/2007/1", "HOLD CANCEL");
//	d3.Set("/2008/1", "HOLD COMPLETE");
//	d3.Set("/2009/1", "HOLD COMPLETE ACK");
//	d3.Set("/2027/1", "VAT STATE");
//	d3.Set("/2028/1", "RIGHT VAT STATE");
//	d3.Set("/2032/1", "Temperature");
//	d3.Set("/2033/1", "Right Temperature");
//	d3.Set("/2037/1", "Set Temperature");
//	d3.Set("/2038/1", "Set Right Temperature");
//	d3.Set("/2042/1", "VAT STATE MISC");
//	d3.Set("/2043/1", "RIGHT VAT STATE MISC");
//	d3.Set("/2050/1", "POLISH FINISHED");
//	d3.Set("/2051/1", "POLISH START");
//	d3.Set("/2052/1", "POLISH FORCED START");
//	d3.Set("/2053/1", "POLISH FORCED CANCEL");
//	d3.Set("/2056/1", "COOK QUANTITY");
//	d3.Set("/2308/1", "VAT STATE MISC 2");
//	d3.Set("/2309/1", "VAT STATE RIGHT MISC 2");
//	d3.Set("/2313/1", "VAT ERROR");
//	d3.Set("/2314/1", "VAT ERROR RIGHT");
//
//	d3.Set("/589/1",  "FLASH NUMBER RESPONSE" );
//	d3.Set("/410/1",  "SOFTWARE NUMBER RESPONSE" );
//	d3.Set("/285/1",  "DOWNLOAD NUMBER RESPONSE" );

	string output = d3.RetrieveJsonFormatMessage(NULL, USE_PRETTY);
	fprintf(fp, "%s\n", output.c_str());

	return true;

}
