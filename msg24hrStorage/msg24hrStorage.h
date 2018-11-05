/*
 * 24hrStorage.h
 *
 *  Created on: Oct 21, 2018
 *      Author: ds
 */

#ifndef _24HRSTORAGE_H_
#define _24HRSTORAGE_H_

#include <inttypes.h>
#include <chrono>
#include <cstdio>
#include <mutex>
#include <fstream>
#include <vector>
#include <signal.h>
#include <list>
#include <dirent.h>


#define MAX_STORED_MSG_SIZE 4096			// arbitrary
#define MSG_STORAGE_FILE_DIR	 	"/MsgStorageDir"
#define MSG_STORAGE_FILE_PREFIX 	"msgStore-"
#define MSG_STORAGE_FILE			MSG_STORAGE_FILE_DIR "/" MSG_STORAGE_FILE_PREFIX

using namespace std;
using namespace std::chrono;

#pragma pack(1)

class Msg24hrStorage
{
public:
	class Msg24hrStorageRecord
	{
	public:
		Msg24hrStorageRecord() {}
		~Msg24hrStorageRecord() {}
		// format of each record saved
		uint16_t						m_recordSize;
		time_t  						m_recordTime;
		uint8_t							m_record[0];
	};

public:
	Msg24hrStorage() {}
	~Msg24hrStorage() {}

	// add the record to the file.
	//	the file is identified by the hour that the message is added.
	//	if the file doesn't exist, create it
	//	if the file does exist, open it
	//	then add the record to the end of the file
	//	then close the file
	void AddRecord( void * recPtr, int recLen );
	bool GetRecordFromStorage( vector<uint8_t> & msg, bool reset = false );
	bool OpenMessageStorageFile( int hoursBackFromNow );
	void CloseMessageStorageFile();
	void DeleteMessageStorageFile( int hoursBackFromNow );

	static void GarbageCollector(sigval_t);

	void LockMessageFile( bool lock )
	{
		if( lock )
			s_storageFileMtx.lock();
		else
			s_storageFileMtx.unlock();
	}


private:
	ifstream m_streamReadFp;


	static std::recursive_mutex s_storageFileMtx;

};

#pragma pack()


#endif /* 24HRSTORAGE_H_ */
