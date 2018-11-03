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
#include <linux/if.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>


#define SOFTWARE_VERSION (char *)"XXX-YYYYY-01 ECL-A"		// damned if I know what number to use

BezelConfig g_bezelConfig;
uint64_t g_macAddress = 0;


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
			printf(" %02x", c );
			mac += (((uint64_t) c) << (uint64_t)(i * 8));
		}
		g_macAddress = mac;

		printf("\n  %" PRIX64 "\n", g_macAddress);
		return;
	}
	return;
}

static void initVC210CommThread(  )
{
	// grab the MAC address for later use
	saveMac();

	// start up the rs485 / fastlan processing
	L2Comm * vcL2Comms = new L2Comm( COM_PORT, VC210_BAUD );
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


int main( void )
{
	printf("!!!Hello There.  Welcome to the Wi-Fi Bezel Version: %s!!!\n", SOFTWARE_VERSION );

	// do some basic inits

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




