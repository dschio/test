/*
 * config.h
 *
 *  Created on: Nov 2, 2018
 *      Author: ds
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "L4JsonInterface.h"

#define CONFIG_FILE_NAME	"Config.json"

class Config: public JsonInterface
{
public:
	Config() {}
	~Config() {}

	virtual void processJsonMessage() {}
};

class BezelConfig
{
public:
	BezelConfig()
	{
		// we will eventually be sending messages up to the cloud over mqtt.  at this point, a lot of that is still
		// up in the air, so let's use a JSON file to set up a few things.  But if that file doesn't exist,
		// create a dummy one based on what we know today.
		FILE * fp = fopen(CONFIG_FILE_NAME, "r");
		if( fp == NULL )
		{
			// if the file doesn't exist, make it.
			fp = fopen(CONFIG_FILE_NAME, "w");
			if( fp == NULL )
			{
				// houston, we have a problem
				printf("error:  can't open config file\n");
				exit(0);
			}
			if( !CreateInitJson(fp) )
			{
				// houston, we have a problem
				printf("error:  can't create config file\n");
				exit(0);
			}
			fflush(fp);
			fseek(fp, 0, SEEK_SET);
			freopen(CONFIG_FILE_NAME, "r", fp);
		}

		if( !m_config.Parse(fp) )
		{
			printf("error:  can't parse config file\n");
			exit(0);
		}

		fclose(fp);
	}

	~BezelConfig() {}

	bool CreateInitJson( FILE * fp );

	string GetTopicForOID( uint16_t oid )
	{
		string retStr;
		Document * doc = GetConfig();

		// get the topic to send this message to, based on the OID
		//	first convert the OID to a string...
		char oidStr[10];
		snprintf( oidStr, sizeof(oidStr), "%d", oid );

		// the try and find the oid in the config json file.  If we find it,
		// then we can grab the topic to post it to from there.
		if( doc->HasMember(oidStr) )
		{
			if( ((*doc)[oidStr]).IsString() )
			{
				retStr = ((*doc)[oidStr]).GetString();
			}
		}

		return retStr;
	}

	Document * GetConfig()	{ return m_config.GetDoc(); }

private:
	Config m_config;
};


#endif /* CONFIG_H_ */
