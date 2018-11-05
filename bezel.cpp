//============================================================================
// Name        : bezel.cpp
// Author      : dschio
// Version     :
// Date		   : 07/21/18
// Copyright   : 2018
// Description : Entry point for the wi-fi bezel
//============================================================================

#include "config.h"
#include "L3Comm.h"
#include "L4Comm.h"
#include "config.h"
#include "msg24hrStorage.h"
#include <linux/if.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <list>

#define SOFTWARE_VERSION (char *)"XXX-YYYYY-01 ECL-A"		// damned if I know what number to use

void makeDirs()
{
	// make sure an default directories are here
	//
	// directory for the 24 hour storage files...
	struct stat st;
	if( stat( MSG_STORAGE_FILE_DIR, &st) == 0 )
	{
		if( (st.st_mode & S_IFDIR) != 0 )
		{
			printf("%s is present\n", MSG_STORAGE_FILE_DIR );
			return;
		}
	}

	if( mkdir( MSG_STORAGE_FILE_DIR,
			S_IRUSR |
			S_IWUSR |
			S_IRGRP |
			S_IWGRP |
			S_IROTH |
			S_IWOTH ) == -1 )
		printf( "storage dir is fubar\n" );

	return;
}

static void saveMac()
{
	struct ifreq s;
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	uint64_t mac = 0;

	strcpy(s.ifr_name, "wlan0");
	if( 0 == ioctl(fd, SIOCGIFHWADDR, &s) )
	{
		int i;
		for( i = 0;i < 6;++i )
		{
			uint8_t c = (uint8_t) s.ifr_addr.sa_data[i];
			printf(" %02x", c);
			mac += (((uint64_t) c) << (uint64_t) (i * 8));
		}
		g_macAddress = mac;

		printf("\n  %" PRIX64 "\n", g_macAddress);
		return;
	}
	return;
}

static void initVC210CommThread()
{
	// grab the MAC address for later use
	saveMac();

	// start up the rs485 / fastlan processing
	char * port = (g_VC210CommPort == "") ? DEFAULT_COM_PORT : (char *) g_VC210CommPort.c_str();
	g_VC210CommPort = port;
	L2Comm * vcL2Comms = new L2Comm(port, VC210_BAUD);
	if( vcL2Comms )
	{
		vcL2Comms->startVC210Comms();
	}

	// start up the fastlan decoder
	L3Comm * vcL3Comms = new L3Comm();
	if( vcL3Comms )
	{
		vcL3Comms->startL3Comms();
	}

	// start up the "cloud" interface
	L4Comm * vcL4Comms = new L4Comm();
	if( vcL4Comms )
	{
		vcL4Comms->startL4Comms();
	}
}

int main( int argc, char **argv )
{
	printf("!!!Hello There.  Welcome to the Wi-Fi Bezel Version: %s!!!\n", SOFTWARE_VERSION);

	int c = 0;

	while( (c = getopt(argc, argv, "p:w:")) != -1 )
	{
		switch( c )
		{
		case 'p':
			g_VC210CommPort = optarg;
			printf("got p: %s\n", g_VC210CommPort.c_str());
			break;
		case 'w':
			g_wifiAdapter = optarg;
			printf("got w: %s\n", g_wifiAdapter.c_str());
			break;
		}
	}

	// do some basic inits
	makeDirs();

	// start the mosquitto MQTT stuff
	mosquitto_lib_init();

	// start up the thread to process data in / out to the VC210
	initVC210CommThread();

	while( true )
	{
		sleep(5);
	}

	mosquitto_lib_cleanup();

	return EXIT_SUCCESS;
}

