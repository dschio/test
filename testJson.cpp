/*
 * testJson.cpp
 *
 *  Created on: Sep 1, 2018
 *      Author: ds
 */

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>   /* For open(), creat() */
#include <termios.h>
#include <string.h>
#include <iostream>
#include <string>

#include <pthread.h>

//#include <stdint.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/pointer.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "L4JsonInterface.h"

using namespace rapidjson;
using namespace std;

FILE * fp;

string fullName = "";

void testJson()
{
//	L4JsonInterface d;
//	fp = fopen("test.json", "r");
//
//	// char buffer method
//	char bufferx[50000];
//	fread(bufferx, 50000, 1, fp);
//
//	if( !d.Parse(bufferx) )
//	{
//		printf("error \n");
//	}
//	d.processJsonMessage();
//
//	if( fp )
//		fclose(fp);
//
//	// file method
//	fp = fopen("test.json", "r");
//	if( !d.Parse(fp) )
//	{
//		printf("error \n");
//	}
//	d.processJsonMessage();
//	if( fp )
//		fclose(fp);
//
//
//
//	printf("==== %s\n", d.RetrieveJsonFormatMessage(NULL, USE_PRETTY).c_str() );
//
//
//	L4JsonInterface d2;
//
//	// test a create
//	d2.Set("/quid", "abc123");
//	d2.Set("/sourceId", "0a:0b:0c:01:02:03");
//	d2.Set("/correlationId", "abc123");
//	d2.Set("/type", "COOK START");
//	d2.Set("/recipeInstanceId", 0);
//	d2.Set("/recipeName", "OR Chicken");
//	d2.Set("/qty", -1);
//	d2.Set("/date", "09/02/2018");
//	d2.Set("/time", "15:00:12");
//	d2.Set("/LQI", "50");
//
//	d2.Set("/test/really", 9.123);
//	d2.Set("/test/bigdeal", "456123");
//	d2.Set("/testa/0", "entr1");
//	d2.Set("/testa/1", "entr2");
//	d2.Set("/testa/2", "entr3");
//
//	d2.Set("/testb/0/basic", "11");
//	d2.Set("/testb/0/next", 111);
//
//	d2.Set("/testb/1/basic", "22");
//	d2.Set("/testb/2/crazy", 33);
//
//	string output = d2.RetrieveJsonFormatMessage(NULL, USE_PRETTY);		// call with the USE_PRETTY option if you want lf
//
//
//	FILE* fpx = fopen("output.json", "wb");
//	fprintf(fpx, "%s\n", output.c_str());
//
//	printf("done\n %s\n", output.c_str());
//
//	fclose(fpx);


	L4JsonInterface d3;

	d3.Set("/brokerIp","192.168.2.104" );
	d3.Set("/2000/0", "/us/qpm/server/workflow");
	d3.Set("/2001/0", "/us/qpm/server/workflow");
	d3.Set("/2002/0", "/us/qpm/server/workflow");
	d3.Set("/2003/0", "/us/qpm/server/workflow");
	d3.Set("/2005/0", "/us/qpm/server/workflow");
	d3.Set("/2006/0", "/us/qpm/server/workflow");
	d3.Set("/2007/0", "/us/qpm/server/workflow");
	d3.Set("/2008/0", "/us/qpm/server/workflow");
	d3.Set("/2009/0", "/us/qpm/server/workflow");
	d3.Set("/2027/0", "/us/qpm/server/workflow");
	d3.Set("/2028/0", "/us/qpm/server/workflow");
	d3.Set("/2032/0", "/us/qpm/server/environmental");
	d3.Set("/2033/0", "/us/qpm/server/environmental");
	d3.Set("/2037/0", "/us/qpm/server/environmental");
	d3.Set("/2038/0", "/us/qpm/server/environmental");
	d3.Set("/2042/0", "/us/qpm/server/workflow");
	d3.Set("/2043/0", "/us/qpm/server/workflow");
	d3.Set("/2050/0", "/us/qpm/server/workflow");
	d3.Set("/2051/0", "/us/qpm/server/workflow");
	d3.Set("/2052/0", "/us/qpm/server/workflow");
	d3.Set("/2053/0", "/us/qpm/server/workflow");
	d3.Set("/2056/0", "/us/qpm/server/workflow");
	d3.Set("/2308/0", "/us/qpm/server/workflow");
	d3.Set("/2309/0", "/us/qpm/server/workflow");
	d3.Set("/2313/0", "/us/qpm/server/error");
	d3.Set("/2314/0", "/us/qpm/server/error");

	d3.Set("/2000/1", "COOK START");
	d3.Set("/2001/1", "COOK COMPLETE");
	d3.Set("/2002/1", "COOK COMPLETE ACK");
	d3.Set("/2003/1", "COOK CANCEL");
	d3.Set("/2005/1", "COOK PREALARM");
	d3.Set("/2006/1", "HOLD START");
	d3.Set("/2007/1", "HOLD CANCEL");
	d3.Set("/2008/1", "HOLD COMPLETE");
	d3.Set("/2009/1", "HOLD COMPLETE ACK");
	d3.Set("/2027/1", "VAT STATE");
	d3.Set("/2028/1", "RIGHT VAT STATE");
	d3.Set("/2032/1", "Temperature");
	d3.Set("/2033/1", "Right Temperature");
	d3.Set("/2037/1", "Set Temperature");
	d3.Set("/2038/1", "Set Right Temperature");
	d3.Set("/2042/1", "VAT STATE MISC");
	d3.Set("/2043/1", "RIGHT VAT STATE MISC");
	d3.Set("/2050/1", "POLISH FINISHED");
	d3.Set("/2051/1", "POLISH START");
	d3.Set("/2052/1", "POLISH FORCED START");
	d3.Set("/2053/1", "POLISH FORCED CANCEL");
	d3.Set("/2056/1", "COOK QUANTITY");
	d3.Set("/2308/1", "VAT STATE MISC 2");
	d3.Set("/2309/1", "VAT STATE RIGHT MISC 2");
	d3.Set("/2313/1", "VAT ERROR");
	d3.Set("/2314/1", "VAT ERROR RIGHT");


	string output = d3.RetrieveJsonFormatMessage(NULL, USE_PRETTY);

	FILE* fpx2 = fopen("mqttConfig.json", "wb");
	fprintf(fpx2, "%s\n", output.c_str());

	printf("done\n %s\n", output.c_str());

	fclose(fpx2);


	L4JsonInterface d4;
	fpx2 = fopen("mqttConfig.json", "r");
	if( !d4.Parse(fpx2) )
	{
		printf("error \n");
	}


	Document * doc = d4.GetDoc();
	string sss = ((*doc)["brokerIp"]).GetString();

	printf( "%s\n", sss.c_str() );

	sss = ((*doc)["2314"][0]).GetString();
	printf( "%s\n", sss.c_str() );

	sss = ((*doc)["2314"][1]).GetString();
	printf( "%s\n", sss.c_str() );
	if( fpx2 )
		fclose(fpx2);


	for( ;; )
		;

}
