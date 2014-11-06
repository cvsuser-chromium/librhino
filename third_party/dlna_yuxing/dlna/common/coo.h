/*-----------------------------------------------------------------------------------------------*/
/*
* Yuxing Software CONFIDENTIAL
* Copyright (c) 2003, 2011 Yuxing Corporation.  All rights reserved.
* 
* The computer program contained herein contains proprietary information
* which is the property of Yuxing Software Co., Ltd.  The program may be used
* and/or copied only with the written permission of Yuxing Software Co., Ltd.
* or in accordance with the terms and conditions stipulated in the
* agreement/contract under which the programs have been supplied.
*
*    filename:			coo.h
*    author(s):			yanyongmeng@gmail.com
*    version:			1.0
*    date:				2012/12/7
* History
*/
/*-----------------------------------------------------------------------------------------------*/
#ifndef _COO_LIB_H_
#define _COO_LIB_H_

#define CONST_STR_LEN(str)		(sizeof(str)-1)
#define COO_OBJECT_NEW(type)	(type *)coo_malloc_bzero(sizeof(type))
extern char *coo_malloc_bzero(int size);
extern void coo_str_rid_tail(char *str, char tail);
extern int coo_str_equal (char *s1, char *s2);
extern int coo_str_are_spaces (char *s1);

extern void coo_str_rm_ext(const char *str);
extern int coo_str_GetXmlPatternChar(char *str, int *num);
extern void coo_str_to_xml(char *str, char *buf, int *buf_len);
extern int coo_str_from_xml(char *xml, char *str);

extern char* coo_strtrim_ends(char *str);
struct json_object* dlna_json_tokener_parse(const char *str);



typedef struct _coo_array_t_ COO_ARRAY;
struct _coo_array_t_ {
	void	**array;
	int		max_count;
	int		real_count;
	int		percent;
};
COO_ARRAY *coo_array_create (int init_count, int percent);
void coo_array_insert(COO_ARRAY *ca, int index, void *item);/* index is zero based, must >= 0 */
void coo_array_append (COO_ARRAY *ca, void *item);
void coo_array_delete(COO_ARRAY *ca, int index);/* index is zero based, must >= 0 */
void coo_array_switch(COO_ARRAY *ca, int x, int y);/* x, y is zero based, must >= 0 */
int coo_array_real_count (COO_ARRAY *ca);
void *coo_array_get (COO_ARRAY *ca, int index);
void coo_array_free (COO_ARRAY *ca);

int coo_util_get_mac (char *lan_interface, char **mac_address);

#endif /* _COO_LIB_H_ */

