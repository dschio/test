/*
 * 24hrStorage.cpp
 *
 *  Created on: Oct 21, 2018
 *      Author: ds
 */

#include "msg24hrStorage.h"
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <cstring>


using namespace std;

std::recursive_mutex Msg24hrStorage::s_storageFileMtx;

// add the record to the file.
//	the file is identified by the hour that the message is added.
//	if the file doesn't exist, create it
//	if the file does exist, open it
//	then add the record to the end of the file
//	then close the file
void Msg24hrStorage::AddRecord( void * recPtr, int recLen )
{
	ofstream msgStrFp;

	// do at least a minor sanity check on it
	if( recPtr == nullptr ||recLen > MAX_STORED_MSG_SIZE )
		return;

	// get time now
	system_clock::time_point recTime = system_clock::now();
	time_t tt = system_clock::to_time_t(recTime);
	struct tm * now = gmtime(&tt);

	// build up a file name string based on this day and time
	std::stringstream ss;

	ss << MSG_STORAGE_FILE << (now->tm_year + 1900) << '-' << (now->tm_mon + 1) << '-' << now->tm_mday << '-' << now->tm_hour;

	cout << "writing to storage file " << ss.str() << '\n';

	msgStrFp.open( ss.str().c_str(), ios_base::app | ios_base::binary  );

	//if( msgfp )
	if( msgStrFp.good() )
	{
		// build a record to save
		// first determine the size
		int sizeOfThisRec = sizeof(Msg24hrStorageRecord) + recLen;

		// allocate a chunk of memory to hold it
		// cast it to the record format
		Msg24hrStorage::Msg24hrStorageRecord * mr = (Msg24hrStorage::Msg24hrStorageRecord *) new uint8_t[sizeOfThisRec];

		if( mr )
		{
			mr->m_recordSize = sizeOfThisRec;
			mr->m_recordTime = tt;

			memcpy(mr->m_record, recPtr, recLen);

			// write to the file
			std::lock_guard < std::recursive_mutex > lck(s_storageFileMtx);

			msgStrFp.write( (const char *) mr, sizeOfThisRec );
			if( ! msgStrFp.good() )
				cout << "cant write storage file\n";
			msgStrFp.flush();
			msgStrFp.close();

			// clean up
			delete  mr;
		}
	}
	else
		cout << "cant open storage file\n";

}

bool Msg24hrStorage::GetRecordFromStorage( vector<uint8_t> & msgVec, bool reset )
{
	int tmpOffset;

	if( m_streamReadFp.good() && !m_streamReadFp.eof() )
	{
		std::lock_guard < std::recursive_mutex > lck(s_storageFileMtx);

		if( reset )
		{
			m_streamReadFp.seekg( 0, ios_base::beg );
		}

		// where are we in the file
		tmpOffset = m_streamReadFp.tellg();

		// first get the header
		Msg24hrStorage::Msg24hrStorageRecord mr;
		m_streamReadFp.read( (char *)&mr.m_recordSize, sizeof(uint16_t) );

		// do at least a minor sanity check on it
		if( mr.m_recordSize < 0 || mr.m_recordSize > MAX_STORED_MSG_SIZE )
			return false;

		// if not at the end of the file
		if( !m_streamReadFp.eof() )
		{
			// then read the data into the vector
			msgVec.resize( mr.m_recordSize );

			m_streamReadFp.seekg( tmpOffset, ios_base::beg );
			m_streamReadFp.read(  (char *) &msgVec[0], mr.m_recordSize );

			// print it - or use if if you want
//			system_clock::time_point recTime = system_clock::from_time_t( mPtr->m_recordTime );
//			time_t tt = system_clock::to_time_t(recTime);

			return true;
		}
	}
	return false;
}

bool Msg24hrStorage::OpenMessageStorageFile( int hoursBackFromNow )
{
	// get time now
	system_clock::time_point recTime = system_clock::now();

	// and x hours ago
	chrono::duration<int, std::ratio<60 * 60> > one_hour(hoursBackFromNow);
	system_clock::time_point xHoursAgo = recTime - one_hour;

	// try to open the file from then
	time_t tt = system_clock::to_time_t(xHoursAgo);
	struct tm * now = gmtime(&tt);

	// build up a file name string based on this day and time
	std::stringstream ss;

	ss << MSG_STORAGE_FILE << (now->tm_year + 1900) << '-' << (now->tm_mon + 1) << '-' << now->tm_mday << '-' << now->tm_hour;

//	cout << "reading from " << ss.str() << '\n';

	if( m_streamReadFp.is_open() )
		m_streamReadFp.close();

	m_streamReadFp.open( ss.str(),ios_base::in );
	return( m_streamReadFp.good() );

}

void Msg24hrStorage::DeleteMessageStorageFile( int hoursBackFromNow )
{
	// get time now
	system_clock::time_point recTime = system_clock::now();

	// and x hours ago
	chrono::duration<int, std::ratio<60 * 60> > one_hour(hoursBackFromNow);
	system_clock::time_point xHoursAgo = recTime - one_hour;

	// try to open the file from then
	time_t tt = system_clock::to_time_t(xHoursAgo);
	struct tm * now = gmtime(&tt);

	// build up a file name string based on this day and time
	std::stringstream ss;

	ss << MSG_STORAGE_FILE << (now->tm_year + 1900) << '-' << (now->tm_mon + 1) << '-' << now->tm_mday << '-' << now->tm_hour;

//	cout << "reading from " << ss.str() << '\n';

	if( m_streamReadFp.is_open() )
		m_streamReadFp.close();

	remove( ss.str() .c_str() );
	return;

}


void Msg24hrStorage::CloseMessageStorageFile()
{
	m_streamReadFp.close();
}

void Msg24hrStorage::GarbageCollector(sigval_t)
{
	printf("in garbage collector\n");

	typedef list<string> fnameVec;
	typedef fnameVec::iterator fnameVecItr;

	fnameVec v;
	fnameVecItr vItr;

	// get a list of all the files in this directory that have the message store prefix
	DIR* dirp = opendir(".");
	struct dirent * dp;
	while( (dp = readdir(dirp)) != NULL )
	{
		char * s = strstr( dp->d_name,  MSG_STORAGE_FILE );

		if( s != nullptr && (s == dp->d_name) )
			v.push_back(dp->d_name);
	}
	closedir(dirp);

	// get time now.  we're going to delete any files that pre-date this time by more than 24 hours
	system_clock::time_point nowTime = system_clock::now();


//	time_t tt = system_clock::to_time_t(nowTime);
//	struct tm * now = gmtime(&tt);
//	cout << "it is now " << (now->tm_year + 1900) << '-' << (now->tm_mon + 1) << '-' << now->tm_mday << '-' << now->tm_hour << "\n";


	// look through the list...
	// we are going to wind up with a list of files that are in this directory that are prior to the last 24 hours, if any.
	for( int i=0; i<24; i++ )
	{
		// for a file from "i" hours ago
		chrono::duration<int, std::ratio<60 * 60> > one_hour(i);
		system_clock::time_point xHoursAgo = nowTime - one_hour;

		time_t tt = system_clock::to_time_t(xHoursAgo);
		struct tm * check = gmtime(&tt);

		// build up a file name string based on that day and time
		std::stringstream ss;

		ss << MSG_STORAGE_FILE << (check->tm_year + 1900) << '-' << (check->tm_mon + 1) << '-' << check->tm_mday << '-' << check->tm_hour;

		// now look through the list to see if the file is in there.
		// if it is, remove it from the list.
		vItr = v.begin();
		do
		{
			if( (*vItr) == ss. str() )
			{
				if( (vItr = v.erase(vItr)) == v.end() )
					continue;
			}
			vItr++;
		} while( vItr != v.end() );
	}

	// once we are here, the list only contains the names of files that are beyond the 24 hour limit.
	//cout << "remaining to be removed: " << "\n";
	for( vItr = v.begin(); vItr != v.end(); ++vItr )
	{
		// so remove those files
		//cout << "removing..." << (*vItr).c_str() << "\n";
		remove( (*vItr).c_str() );
	}


}
