/*
 * utilities.h
 *
 *  Created on: Sep 13, 2018
 *      Author: ds
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_


#include <string>
#include <time.h>

uint64_t current_timestamp();
typedef unsigned int (*HashFunction)(const std::string&);

uint64_t RSHash(const uint64_t v);
uint64_t RSHash(const uint64_t mac, uint64_t secs );
uint64_t guidHash();
uint64_t RSHash  (const std::string& str);


unsigned int JSHash  (const std::string& str);
unsigned int PJWHash (const std::string& str);
unsigned int ELFHash (const std::string& str);
unsigned int BKDRHash(const std::string& str);
unsigned int SDBMHash(const std::string& str);
unsigned int DJBHash (const std::string& str);
unsigned int DEKHash (const std::string& str);
unsigned int BPHash  (const std::string& str);
unsigned int FNVHash (const std::string& str);
unsigned int APHash  (const std::string& str);

uint64_t getMac();

#endif /* UTILITIES_H_ */
