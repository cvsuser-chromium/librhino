#ifndef __PRECOMP_H__
#define __PRECOMP_H__

#if defined(__ICC)
  #define __cdecl

  #if defined (_A6) || defined (_W7)
    #include <xmmintrin.h>
  #else
    #if defined (_M6)
      #include <mmintrin.h>
    #endif
  #endif
#endif

#include "owncp.h"
#include "ippcp.h"

#endif /* __PRECOMP_H__ */
