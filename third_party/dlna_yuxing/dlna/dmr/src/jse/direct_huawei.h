#ifndef __AVTRANSPORT_H__
#define __AVTRANSPORT_H__

#include "dlna_type.h"
extern int Raw_Dmp_HuaweiJse( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult, int readonly );
extern void Raw_Dmp_SetCallback(t_DLNA_EVENT EventCallback);

#endif

