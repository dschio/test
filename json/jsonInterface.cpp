/*
 * jsonInterface.cpp
 *
 *  Created on: Sep 2, 2018
 *      Author: ds
 */

#include "jsonInterface.h"





bool JsonInterface::Parse( FILE * fp )
{
	m_readBuffer = new char[65536];
	FileReadStream json(fp, m_readBuffer, sizeof(m_readBuffer));
	m_doc.ParseStream(json);

	if( m_doc.HasParseError() )
	{
		printf("\nError(offset %u): %d\n", (unsigned) m_doc.GetErrorOffset(), m_doc.GetParseError());
	}
	return (!m_doc.HasParseError());
}

// parse from a char string
bool JsonInterface::Parse( const char * c )
{
	m_doc.Parse(c);
	if( m_doc.HasParseError() )
	{
		printf("\nError(offset %u): %d\n", (unsigned) m_doc.GetErrorOffset(), m_doc.GetParseError());
	}
	return( !m_doc.HasParseError() );
}

void JsonInterface::Process()
{
	processJsonMessage();
}

string JsonInterface::RetrieveJsonFormatMessage( int * size, bool pretty )
{
	StringBuffer buffer;
	buffer.Clear();

	if( pretty )
	{
		PrettyWriter<StringBuffer> writer(buffer);
		m_doc.Accept(writer);
	}
	else
	{
		Writer<StringBuffer> writer(buffer);
		m_doc.Accept(writer);
	}

	if( size )
		*size = buffer.GetLength();
	return( string( buffer.GetString()) );
}


// debug
void JsonInterface::PrintNode( const rapidjson::Value &node, size_t indent, unsigned int level, const std::string& nodeName )
{
	//std::cout << GetIndentString(indent, level);

	if( !nodeName.empty() )
	{
		//std::cout << nodeName << ": ";
		printf(">>>>> %s  ", nodeName.c_str());
	}

	if( node.IsBool() )
		std::cout << node.GetBool();

	else if( node.IsInt() )
		std::cout << node.GetInt();

	else if( node.IsUint() )
		std::cout << node.GetUint();

	else if( node.IsInt64() )
		std::cout << node.GetInt64();

	else if( node.IsUint64() )
		std::cout << node.GetUint64();

	else if( node.IsDouble() )
		std::cout << node.GetDouble();

	else if( node.IsString() )
		std::cout << node.GetString();

	else if( node.IsArray() )
	{
		if( !nodeName.empty() )
		{
			//std::cout << "\n" << GetIndentString(indent, level);
		}

		PrintArray(node, nodeName, indent, level);
	}

	else if( node.IsObject() )
	{
		if( !nodeName.empty() )
		{
			//std::cout << "\n" << GetIndentString(indent, level);
		}
		PrintObject(node, nodeName, indent, level);
	}

	std::cout << "\n";
}

void JsonInterface::PrintObject( const rapidjson::Value &node, string hdr, size_t indent, unsigned int level )
{
	for( rapidjson::Value::ConstMemberIterator childNode = node.MemberBegin();childNode != node.MemberEnd();++childNode )
	{
		PrintNode(childNode->value, indent, level + 1, hdr + childNode->name.GetString() + ":");
	}
}

void JsonInterface::PrintArray( const rapidjson::Value& node, string hdr, size_t indent, unsigned int level )
{
	for( rapidjson::SizeType i = 0;i < node.Size();++i )
	{
		PrintNode(node[i], indent, level + 1, hdr);
	}
}
