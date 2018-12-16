/*
 * Head file for tea5767 chip
 *
 */


#ifndef _TEA5767_H
#define _TEA5767_H

#include "types.h"
#include "config.h"

#define REGISTER_SIZE	5

#define INTERMEDIATE_FREQUENCY	225

#define TEA5767_SEARCH_DOWN	0
#define TEA5767_SEARCH_UP		1

/* This is in linux compute method */
#define LINUX_IF	231250

#define MIN_PLL	0x299D	/* frequency in 87.5 MHz */
#define MAX_PLL	0x3364	/* frequncy in 108.00 MHz */

#define JAPANESE_BAND	0x1
#define EUROPEAN_BAND	0x3

#define MIN_EURO_FREQ		87500
#define MAX_EURO_FREQ	     108000

#define MIN_JAPAN_FREQ		76000
#define MAX_JAPAN_FREQ		91000

#define MIN_EURO_FREQ_MHZ		87.5
#define MAX_EURO_FREQ_MHZ	108.0

#define MIN_JAPAN_FREQ_MHZ	76.0
#define MAX_JAPAN_FREQ_MHZ	91.0


#define FREQ_STEP	100
#define PLL_STEP		0xC


//#define TEA5767_LOW_LO_32768 

//#ifdef TEA5767_LOW_LO_32768
/* The frequency is KHz */
/* Use low to high inject */
#define LTOH_PLL_TO_FREQ(p)	(((long)(p) << 15) / 4000 + INTERMEDIATE_FREQUENCY)
#define LTOH_FREQ_TO_PLL(f)		((((f) - INTERMEDIATE_FREQUENCY ) * 4000) >> 15)


/* Use high to low inject */
#define HTOL_PLL_TO_FREQ(p)	(((long)(p) << 15) / 4000 - INTERMEDIATE_FREQUENCY)
#define HTOL_FREQ_TO_PLL(f)		((((f) + INTERMEDIATE_FREQUENCY ) * 4000) >> 15)

//#endif

/* Chip address for iic */
#define TEA5767_WR_BASE	0xC0
#define TEA5767_RD_BASE	0xC1


/* Write mode byte 1 */
#define TEA5767_MUTE	0x80
#define TEA5767_AUTO_SEARCH_MODE	0x40

/* Write mode byte 3 */
#define TEA5767_SEARCH_DIRECT_UP	0x80
#define TEA5767_SEARCH_HIGH_LEVEL	0x60
#define TEA5767_SEARCH_MID_LEVEL	0x40
#define TEA5767_SEARCH_LOW_LEVEL	0x20
#define TEA5767_HIGH_LO_INJECT_MODE	0x10
#define TEA5767_DISABLE_STEREO	0x08
#define TEA5767_SEARCH_INDEX	0x01

/* Write mode byte 4 */
#define TEA5767_STANDBY_MODE	0x40
#define TEA5767_JAPAN_BAND	0x20
#define TEA5767_XTAL_32768K	0x10
#define TEA5767_SOFT_MUTE	0x08
#define TEA5767_STEREO_NOISE_REDUCE	0x02


/* Read mode byte 1 */
#define TEA5767_RADIO_READY_MASK	0x80
#define TEA5767_BAND_LIMIT_MASK	0x40

/* Read mode byte 2 */
#define TEA5767_STEREO_MASK	0x80


/* Read mode byte 3 */
#define TEA5767_IF_CNTR_MASK	0x7F

/* IF counter in XTAL 32768Hz */
#define TEA5767_MIN_IF	0x31
#define TEA5767_MAX_IF	0x3E

#define VALID_IF(i)	((i) >= TEA5767_MIN_IF && (i) <= TEA5767_MAX_IF)

/* Fourth register */
#define TEA5767_ADC_LEVEL_MASK	0xF0

void tea5767Init(void);

char sleepRadio(void);

char wakupRadio(char bandSwitch, short pll);

char setFreq(char bandSwitch, short freq);

char setPLL(char bandSwitch, short pll);

char autoScan(ChannelList* chl, char direct, char bandSwitch);

char manualScan(char direct, short freq);


#endif

