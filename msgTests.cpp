/*
 * msgTests.cpp
 *
 *  Created on: Jul 15, 2018
 *      Author: ds
 */
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <fcntl.h>    /* For O_RDWR */
#include <termios.h>
#include <string.h>
#include <pthread.h>

//#include <stdint.h>

#include "mqtt_messaging.h"

//#include </home/ds/workspace/apps/zeromq-4.2.3/include/zmq.h>

using namespace std;


//#define TEST_BROKER_IP	"54.91.111.24"

///////////////////////////////////////////////////////////////////  MQTT
/////////////////////////////////// SUBSCRIBE

//// subscribe thread
//void * msgTestSub( void * unused )
//{
//	mqtt_message * vc;
//
//	vc = new mqtt_message(NULL, (char *) TEST_BROKER_IP, 1883);
//	vc->setTopic((char *) "/testtopic");
//
//	printf("starting loop\n");
//	vc->loop_forever();
//
//	mosqpp::lib_cleanup();
//
//	return 0;
//}

void * msgTestSub( void * unused )
{
	mqtt_message * vc;
	vc = new mqtt_message(NULL);
	vc->connect((char *) "127.0.0.1" );
	vc->setTopic("dummy");


	int mosq_fd = vc->socket();


	fd_set rfds, afds;
	struct timeval tv;

	FD_ZERO(&rfds);
	FD_SET(mosq_fd, &rfds);

	while( 1 )
	{
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		afds = rfds;


		int rc = select( mosq_fd + 1, &afds, NULL, NULL, &tv);

		if( rc < 0 )
		{
			perror("select");
			exit (EXIT_FAILURE);
		}
		else if( rc == 0 )
		{
			printf("timeout\n");
			continue;
		}
		else if( FD_ISSET(mosq_fd, &afds) )
		{
			printf( "got it \n" );
			// so the loop_read calls the on_message?
			int ret = vc->loop_read(1);
			if( ret == MOSQ_ERR_CONN_LOST )
			{
				/* We've been disconnected from the server */
				printf("Reconnect...\n");
				vc->reconnect();
			}
			if( vc->want_write() )
			{
				vc->loop_write(1);
			}

			vc->loop_misc();
			mosq_fd = vc->socket();

		}
		else
		{
			printf("FD wasn't set!\n");
		}
	}
}
//
//// publish thread
//void * msgTestPub( void * unused )
//{
//	mqtt_message * vc;
//
//	vc = new mqtt_message(NULL, (char *) TEST_BROKER_IP, 1883);
//
//	vc->loop_start();
//
//	printf("here we go...\n");
//	char *buf = (char *) malloc(64);
//	memset(buf, 0, 64);
//	for( int i = 0;i < TEST_COUNT;i++ )
//	{
//		*(int *) &buf[0] = i;
//		int snd = vc->publish((char *)VC210_RCVD_PKT, sizeof(int), buf, QOS_0);
//		if( snd != 0 )
//			printf("mqtt_send error=%i\n", snd);
//
//		//usleep(2);
//	}
//
//	mosquitto_lib_cleanup();
//
//	return 0;
//}

#ifdef adfadfadf
///////////////////// 0MQ stuff below
///////////////////////////////////
//
void * _0msgTestSub( void * context )
{
	//void *context = zmq_ctx_new();
	void *responder = zmq_socket(context, ZMQ_PAIR);
	//int rc = zmq_bind(responder, "tcp://*:5555");
	int rc = zmq_bind(responder, "inproc://mytest");

	printf("Receiver: Started %d\n", rc);

	char buffer[128];

	int count = 0;

	while( true )
	{
		int num = zmq_recv(responder, buffer, sizeof(int), 0);

		if( num < 0 )
		break;

		//if (num > 0)
		{
			//buffer[num] = '\0';
			//printf("Receiver: Received (%x) %d\n", buffer[0], num);
		}
		count++;
	}

	printf("done %d, %d\n", count, *(int *) &buffer[0]);

	return NULL;
}

void * _0msgTestPub( void * context )
{
	char buffer[32];

//    void *context = zmq_ctx_new();
	void *requester = zmq_socket(context, ZMQ_PAIR);
	//int rc = zmq_connect(requester, "tcp://localhost:5555");
	int rc = zmq_connect(requester, "inproc://mytest");

	printf("Sender: Started %d\n", rc);

	for( int i = 0;i < TEST_COUNT;++i )
	{
		//usleep(1);
		//sprintf(buffer, "Message %d\0", i + 1);
		//printf("Sender: Sending (%s)\n", buffer);
		//int rc = zmq_send(requester, buffer, strlen(buffer), 0);

		*(int *) &buffer[0] = i;
		zmq_send(requester, buffer, sizeof(int), 0);
	}

	zmq_close(requester);
	zmq_ctx_destroy(context);

	return NULL;
}
#endif

