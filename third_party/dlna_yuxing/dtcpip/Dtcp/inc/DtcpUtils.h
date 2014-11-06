//#############################################################################
//## Copyright (c) 2004 Intel Corporation All Rights Reserved. 
//## 
//## The source code contained or described herein and all documents related to
//## the source code ("Material") are owned by Intel Corporation or its 
//## suppliers or licensors. Title to the Material remains with Intel 
//## Corporation or its suppliers and licensors. The Material contains trade 
//## secrets and proprietary and confidential information of Intel or its
//## suppliers and licensors. The Material is protected by worldwide copyright
//## and trade secret laws and treaty provisions. No part of the Material may 
//## be used, copied, reproduced, modified, published, uploaded, posted, 
//## transmitted, distributed, or disclosed in any way without Intel's prior 
//## express written permission.
//## 
//## No license under any patent, copyright, trade secret or other 
//## intellectual property right is granted to or conferred upon you by 
//## disclosure or delivery of the Materials, either expressly, by 
//## implication, inducement, estoppel or otherwise. Any license under such 
//## intellectual property rights must be express and approved by Intel in 
//## writing.
//#############################################################################

#ifndef __DTCP_UTILS_H__
#define __DTCP_UTILS_H__

/// \file
/// \brief Defines macros used by the DTCP-IP SDK

/// \brief Macro to swap endian format in place
#define SwapInPlace(pData,length) \
	{\
	int i;\
	unsigned char *pBuffer=(unsigned char *)pData;\
	unsigned char a;\
	for(i=0;i<length/2;i++)\
	{ \
	a=pBuffer[i];\
	pBuffer[i]=pBuffer[length-i -1];\
	pBuffer[length-i-1]=a;\
	}\
	}

/// \brief Macro to calculate the max of two values
#define MAX(val1,val2)  ((val1 >= val2) ? val1 : val2)
#endif // __DTCP_UTILS_H__
