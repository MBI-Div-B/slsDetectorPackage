#ifndef SERVER_DEFS_H
#define SERVER_DEFS_H

#include "sls_detector_defs.h"

#include <stdint.h> 


// Hardware definitions

#define NCHAN 128
#define NCHIP 10
#define NMAXMODX  1
#define NMAXMODY 1
#define NMAXMOD NMAXMODX*NMAXMODY
#define NDAC 8
#define NADC 5

#define NCHANS NCHAN*NCHIP*NMAXMOD
#define NDACS NDAC*NMAXMOD

#define NTRIMBITS 6
#define NCOUNTBITS 24

//#define TRIM_DR ((2**NTRIMBITS)-1)
//#define COUNT_DR ((2**NCOUNTBITS)-1) 
#define TRIM_DR (((int)pow(2,NTRIMBITS))-1)
#define COUNT_DR (((int)pow(2,NCOUNTBITS))-1)

#define PHASE_SHIFT 120


#define ALLMOD 0xffff
#define ALLFIFO 0xffff

#ifdef VIRTUAL
#define DEBUGOUT
#endif

#define CLK_FREQ 32.1E+6


#define THIS_SOFTWARE_VERSION 0x20100429
#endif
