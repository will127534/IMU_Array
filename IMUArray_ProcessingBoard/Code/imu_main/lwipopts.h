#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Generally you would define your own explicit list of lwIP options
// (see https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html)
//
// This example uses a common include to avoid repetition
#include "lwipopts_examples_common.h"


//#define TCP_SND_BUF (8 * TCP_MSS) // 2
//#define TCP_WND TCP_SND_BUF // 2
#define TCP_SND_QUEUELEN 200
#define MEMP_NUM_TCP_SEG TCP_SND_QUEUELEN 
//#define 	TCP_SND_QUEUELEN   ((4 * (TCP_SND_BUF) + (TCP_MSS - 1))/(TCP_MSS))
//#define LWIP_WND_SCALE 1
//#define TCP_RCV_SCALE          0
#endif
