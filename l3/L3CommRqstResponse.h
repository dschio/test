/*
 * L4CommRqstResponse.h
 *
 *  Created on: Sep 3, 2018
 *      Author: ds
 */

#ifndef L3COMMRQSTRESPONSE_H_
#define L3COMMRQSTRESPONSE_H_


#define NUM_OF_PROFILES						9
#define LENGTH_NAME							16
#define NUM_OF_PRODUCTS     				20

#include <cstdint>

#pragma pack(1)
typedef struct
{
	uint8_t what;
	uint8_t which;
	uint16_t length;
} EEP_PRODUCT_HEADER;

typedef struct
{
	int32_t time[NUM_OF_PROFILES];       // = 9
	int16_t temp[NUM_OF_PROFILES];
	uint8_t flag[NUM_OF_PROFILES];       // bit 1: mode, bit 0: pressure
	int32_t prealarm[NUM_OF_PROFILES];
	uint8_t prealarmDone[NUM_OF_PROFILES];
	uint8_t prealarmTone[NUM_OF_PROFILES];
	uint8_t filterOrSetback;              // 0 is filter key
	uint8_t key_weight;
	uint16_t hold;
	uint16_t instantOnTime;
	uint8_t timeFormat;
	uint8_t holdType;             // WILL be removed
	uint8_t holdLink;
	uint8_t trackerID;
	uint8_t trackerDisplayID;
} EEP_PRODUCT;             // total = 128 bytes

typedef struct
{
	uint8_t productName[LENGTH_NAME];    // = 16
	uint8_t prealarmName[NUM_OF_PROFILES][LENGTH_NAME];
	uint8_t enable;
} EEP_PRODUCT_2;

typedef struct
{
	EEP_PRODUCT_HEADER prodHdr;
	EEP_PRODUCT eep;
	EEP_PRODUCT_2 eep2;
} BUTTON_DATA;

class SetpointResponse
{
public:
	uint16_t status;
	BUTTON_DATA bdata;
};

#pragma pack()

#endif /* L3COMMRQSTRESPONSE_H_ */
