/*
 * jsonInterface.h
 *
 *  Created on: Sep 2, 2018
 *      Author: ds
 */

#ifndef JSONINTERFACE_H_
#define JSONINTERFACE_H_

//#include <cstdint>
#include <iostream>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/pointer.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#define	USE_PRETTY	true


using namespace rapidjson;
using namespace std;

class JsonInterface
{
public:
	JsonInterface()
	{
		m_readBuffer = NULL;
	}
	~JsonInterface()
	{
		if( m_readBuffer )
			delete[] m_readBuffer;
	}

	// parse from a file
	bool Parse( FILE * fp );

	// parse from a char string
	bool Parse( const char * c );
	void Process();

	// create fields and values. intended for generating JSON data
	void Set(const char * field, const char * val ) 	{ Pointer(field).Set(m_doc, (char *)val); }
	void Set(const char * field, int val ) 				{ Pointer(field).Set(m_doc, (int)val); }
	void Set(const char * field, unsigned int val ) 	{ Pointer(field).Set(m_doc, (unsigned int)val); }
	void Set(const char * field, int64_t val ) 			{ Pointer(field).Set(m_doc, (int64_t)val); }
	void Set(const char * field, uint64_t val ) 		{ Pointer(field).Set(m_doc, (uint64_t)val); }
	void Set(const char * field, bool val ) 			{ Pointer(field).Set(m_doc, (bool)val); }
	void Set(const char * field, double val ) 			{ Pointer(field).Set(m_doc, (double)val); }

	Document * GetDoc()									{ return & m_doc; }

	// retrieve a JSON formatted string
	string RetrieveJsonFormatMessage( int * size = NULL, bool pretty = false );

	virtual void processJsonMessage() = 0;



	// debug stuff
	void PrintNode( const rapidjson::Value &node, size_t indent = 0, unsigned int level = 0, const std::string& nodeName = "" );
	void PrintObject( const rapidjson::Value &node, string hdr, size_t indent = 0, unsigned int level = 0 );
	void PrintArray( const rapidjson::Value& node, string hdr, size_t indent = 0, unsigned int level = 0 );


protected:
	Document m_doc;
	char * m_readBuffer;

};



#endif /* JSONINTERFACE_H_ */
