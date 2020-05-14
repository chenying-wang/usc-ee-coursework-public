/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, January 24, 2020
 **/
#ifndef __COMMON
#define __COMMON

#define SUCCESS 0
#define ERROR 1

#define PXL_FMT_GRAY "gray"
#define PXL_FMT_RGB24 "rgb24"

#define NUM_OF_CHANNELS_GRAY 1
#define CHANNEL_GRAY 0

#define NUM_OF_CHANNELS_RGB 3
#define CHANNEL_R 0
#define CHANNEL_G 1
#define CHANNEL_B 2

#define DEFAULT_PIXEL_VALUE 0x5a

#define DEFAULT_PIXEL_TYPE uint8_t
#ifndef PIXEL_TYPE
#define PIXEL_TYPE DEFAULT_PIXEL_TYPE
#endif
#define MAX_PIXEL_VALUE ((PIXEL_TYPE) ((1 << (sizeof(PIXEL_TYPE) << 3)) - 1))

#endif
