/*
 * utilities.cpp
 *
 *  Created on: Sep 13, 2018
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
#include <string>

#include <utilities.h>
#include <sys/time.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <linux/wireless.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include <sys/ioctl.h>

using namespace std;

extern uint64_t g_macAddress;

int getLQI()
{
	iwreq req;
	iw_range *range;
	iw_statistics *quality;

	int maxQual;
	int presQual;
	int sigStrength = 0;

	char buf[sizeof(struct iw_range) * 2];

	bzero(buf, sizeof(buf));
	req.u.data.pointer = buf;
	req.u.data.length = sizeof(buf);
	req.u.data.flags = 0;
	strcpy(req.ifr_name, "wlan0");

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	// get the ranges
	if( ioctl(sockfd, SIOCGIWRANGE, &req) >= 0 )
	{
		range = (struct iw_range *) buf;
		maxQual = range->max_qual.qual;
	}

	//this will gather the signal strength
	bzero(buf, sizeof(buf));
	req.u.data.pointer = buf;
	req.u.data.length = sizeof(buf);
	req.u.data.flags = 0;
	strcpy(req.ifr_name, "wlan0");

	if( ioctl(sockfd, SIOCGIWSTATS, &req) == -1 )
	{
		//die with error, invalid interface
		fprintf(stderr, "Invalid interface.\n");
		return (-1);
	}
	else if( ((iw_statistics *) req.u.data.pointer)->qual.updated & IW_QUAL_DBM )
	{
		quality = (struct iw_statistics *) buf;
		presQual = quality->qual.qual;
	}

	if( maxQual > 0 )
	{
		sigStrength = (presQual * 100) / maxQual;
	}

	printf("SIGNAL STRENGTH %d\n", sigStrength);

	return sigStrength;
}

uint64_t current_timestamp()
{
    struct timeval te;
    gettimeofday(&te, NULL);
    uint64_t milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}

uint64_t getMac()
{
	return g_macAddress;
}

// simple 64 bit hash of the mac address and the time (seconds since start)
uint64_t guidHash()
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return RSHash( g_macAddress, (uint64_t)((uint64_t)ts.tv_sec * 1000000000) + (uint64_t)ts.tv_nsec );
}

uint64_t RSHash( const uint64_t mac, uint64_t nsecs )
{
	uint64_t b = 378551;
	uint64_t a = 63689;
	uint64_t hash = 0;

	union HASH
	{
		struct HASH_IN
		{
			uint64_t mac;
			uint64_t nsecs;
		} hashIn;
		char a[sizeof(struct HASH_IN)];
	} u;
	char * hashInPtr;

	u.hashIn.mac = mac;
	u.hashIn.nsecs = nsecs;

	hashInPtr = u.a;

	int sizeOfHash = sizeof(HASH::HASH_IN);

	for( int i = 0;i < sizeOfHash;i++ )
	{
		hash = hash * a + (char) hashInPtr[i];
		a = a * b;
	}

	return hash;
}

uint64_t RSHash(const uint64_t v)
{
   uint64_t b    = 378551;
   uint64_t a    = 63689;
   uint64_t hash = 0;

   for(size_t i = 0; i < sizeof(uint64_t); i++)
   {
      hash = hash * a + (v>>(i*8));
      a    = a * b;
   }

   return hash;
}

uint64_t RSHash(const string& str)
{
   uint64_t b    = 378551;
   uint64_t a    = 63689;
   uint64_t hash = 0;

   for(size_t i = 0; i < str.length(); i++)
   {
      hash = hash * a + str[i];
      a    = a * b;
   }

   return hash;
}
/* End Of RS Hash Function */


unsigned int JSHash(const string& str)
{
   unsigned int hash = 1315423911;

   for(size_t i = 0; i < str.length(); i++)
   {
      hash ^= ((hash << 5) + str[i] + (hash >> 2));
   }

   return hash;
}
/* End Of JS Hash Function */


unsigned int PJWHash(const string& str)
{
   unsigned int BitsInUnsignedInt = (unsigned int)(sizeof(unsigned int) * 8);
   unsigned int ThreeQuarters     = (unsigned int)((BitsInUnsignedInt  * 3) / 4);
   unsigned int OneEighth         = (unsigned int)(BitsInUnsignedInt / 8);
   unsigned int HighBits          = (unsigned int)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth);
   unsigned int hash              = 0;
   unsigned int test              = 0;

   for(size_t i = 0; i < str.length(); i++)
   {
      hash = (hash << OneEighth) + str[i];

      if((test = hash & HighBits)  != 0)
      {
         hash = (( hash ^ (test >> ThreeQuarters)) & (~HighBits));
      }
   }

   return hash;
}
/* End Of  P. J. Weinberger Hash Function */


unsigned int ELFHash(const string& str)
{
   unsigned int hash = 0;
   unsigned int x    = 0;

   for(size_t i = 0; i < str.length(); i++)
   {
      hash = (hash << 4) + str[i];
      if((x = hash & 0xF0000000L) != 0)
      {
         hash ^= (x >> 24);
      }
      hash &= ~x;
   }

   return hash;
}
/* End Of ELF Hash Function */


unsigned int BKDRHash(const string& str)
{
   unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
   unsigned int hash = 0;

   for(size_t i = 0; i < str.length(); i++)
   {
      hash = (hash * seed) + str[i];
   }

   return hash;
}
/* End Of BKDR Hash Function */


unsigned int SDBMHash(const string& str)
{
   unsigned int hash = 0;

   for(size_t i = 0; i < str.length(); i++)
   {
      hash = str[i] + (hash << 6) + (hash << 16) - hash;
   }

   return hash;
}
/* End Of SDBM Hash Function */


unsigned int DJBHash(const string& str)
{
   unsigned int hash = 5381;

   for(size_t i = 0; i < str.length(); i++)
   {
      hash = ((hash << 5) + hash) + str[i];
   }

   return hash;
}
/* End Of DJB Hash Function */


unsigned int DEKHash(const string& str)
{
   unsigned int hash = static_cast<unsigned int>(str.length());

   for(size_t i = 0; i < str.length(); i++)
   {
      hash = ((hash << 5) ^ (hash >> 27)) ^ str[i];
   }

   return hash;
}
/* End Of DEK Hash Function */


unsigned int BPHash(const string& str)
{
   unsigned int hash = 0;
   for(size_t i = 0; i < str.length(); i++)
   {
      hash = hash << 7 ^ str[i];
   }

   return hash;
}
/* End Of BP Hash Function */


unsigned int FNVHash(const string& str)
{
   const unsigned int fnv_prime = 0x811C9DC5;
   unsigned int hash = 0;
   for(size_t i = 0; i < str.length(); i++)
   {
      hash *= fnv_prime;
      hash ^= str[i];
   }

   return hash;
}
/* End Of FNV Hash Function */


unsigned int APHash(const string& str)
{
   unsigned int hash = 0xAAAAAAAA;

   for(size_t i = 0; i < str.length(); i++)
   {
      hash ^= ((i & 1) == 0) ? (  (hash <<  7) ^ str[i] * (hash >> 3)) :
                               (~((hash << 11) + (str[i] ^ (hash >> 5))));
   }

   return hash;
}

