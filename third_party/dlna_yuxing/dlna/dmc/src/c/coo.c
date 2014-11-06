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
 *    filename:			coo.c
 *    author(s):			yanyongmeng@gmail.com
 *    version:			1.0
 *    date:				2012/12/7
 * History
 */
/*-----------------------------------------------------------------------------------------------*/

#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

#include "coo.h"
#include "hitTime.h"

/*-----------------------------------------------------------------------------------------------*/
#define CDS_CHAR_SPACE  	' '

char *coo_malloc_bzero(int size)
{
  char *p = malloc(size);
  if(p)
    memset(p, 0, size);
  return p;
}

void coo_str_rid_tail(char *str, char tail)
{
  int len;
  if( str && (len=strlen(str)) && (len>1) && (str[len-1]==tail) )
    str[len-1]= '\0';
}

int coo_str_equal (char *s1, char *s2)
{
  return ( s1 && s2 && (!strcmp(s1,s2)) )? 1: 0;
}
int coo_str_are_spaces (char *s1)
{
  char *s = coo_strtrim_ends(s1);
  if(s)
    return s[0]? 0 : 1;
  else
    return -1;
}
void coo_str_rm_ext(const char *str)
{
  if(str)
  {
    char *dot = strrchr(str, '.');
    if(dot && (dot != str))
      *dot = 0;
  }
}

char* coo_strtrim_ends(char *str)
{
  char *s;

  if(!str)
    return str;

  while(CDS_CHAR_SPACE == *str)
    str++;
  if(*str == 0)
    return str;

  s = str + strlen(str) - 1;
  while(CDS_CHAR_SPACE == *s)
    s--;
  s++;
  *s = 0;

  return str;
}

char* coo_strtrim_all(char *str)
{
  char *s1, *s2, c;
  int found = 0;

  if(!str)
    return str;

  while(CDS_CHAR_SPACE == *str)
    str++;
  if(*str == 0)
    return str;

  s1 = str;
  s2 = str;
  while(*s1)
  {
    c = *s1;
    if(CDS_CHAR_SPACE != c)
    {
      *s2 = c;
      s2++;
      found = 0;
    }
    else
    {
      if(!found)
      {
        *s2 = c;
        s2++;
        found = 1;
      }
    }			

    s1++;
  }

  if(found)
    *(s2 - 1) = 0;
  else
    *s2 = 0;

  return str;
}


int coo_str_GetXmlPatternChar(char *str, int *num)
{
  int n, slen = 0;
  char c;

  if( !str || !num)
    return -1;

  n = 0;
  c = *str;
  while(c)
  {
    if(c == '<')
    {
      n++;
        slen += CONST_STR_LEN("&lt;");
    }
    else if(c == '>')
    {
      n++;
        slen += CONST_STR_LEN("&gt;");
    }
    else if(c == '&')
    {
      n++;
        slen += CONST_STR_LEN("&amp;");
    }
    else if(c == '\'')
    {
      n++;
        slen += CONST_STR_LEN("&apos;");
    }
    else if(c == '\"')
    {
      n++;
        slen += CONST_STR_LEN("&quot;");
    }
    else
    {
    }

    str++;
    c = *str;
  }

  *num = n;
  return slen;
}

void coo_str_to_xml(char *str, char *buf, int *buf_len)
{
  int n, slen = 0, max_len;
  char c;

  if( !str || !buf || !buf_len)
    return;

  max_len = *buf_len - 1;
  n = 0;
  c = *str;
  while(c)
  {
    if(c == '<')
    {
      n++;
        slen += CONST_STR_LEN("&lt;");

      if(slen > max_len)
        break;
      strcat(buf, "&lt;");
    }
    else if(c == '>')
    {
      n++;
        slen += CONST_STR_LEN("&gt;");

      if(slen > max_len)
        break;
      strcat(buf, "&gt;");
    }
    else if(c == '&')
    {
      n++;
        slen += CONST_STR_LEN("&amp;");

      if(slen > max_len)
        break;
      strcat(buf, "&amp;");
    }
    else if(c == '\'')
    {
      n++;
        slen += CONST_STR_LEN("&apos;");

      if(slen > max_len)
        break;
      strcat(buf, "&apos;");
    }
    else if(c == '\"')
    {
      n++;
        slen += CONST_STR_LEN("&quot;");

      if(slen > max_len)
        break;
      strcat(buf, "&quot;");
    }
    else
    {
      buf[slen] = c;
      slen++;
      buf[slen] = 0;
    }

    str++;
    c = *str;
  }

  *buf_len = slen;
}

int coo_str_from_xml(char *xml, char *str)
{
  int slen = 0;

  if( !xml || !str)
    return -1;

  while(*xml)
  {
    if( 	 !strncasecmp(xml, "&lt;",		CONST_STR_LEN("&lt;")) )
    {
      xml += CONST_STR_LEN("&lt;");
      *str = '<';
    }
    else if( !strncasecmp(xml, "&gt;",		CONST_STR_LEN("&gt;")) )
    {
      xml += CONST_STR_LEN("&lt;");
      *str = '>';
    }
    else if( !strncasecmp(xml, "&amp;",		CONST_STR_LEN("&amp;")) )
    {
      xml += CONST_STR_LEN("&amp;");
      *str = '&';
    }
    else if( !strncasecmp(xml, "&apos;",	CONST_STR_LEN("&apos;")) )
    {
      xml += CONST_STR_LEN("&apos;");
      *str = '\'';
    }
    else if( !strncasecmp(xml, "&quot;",	CONST_STR_LEN("&quot;")) )
    {
      xml += CONST_STR_LEN("&quot;");
      *str = '\"';
    }
    else
    {
      *str = *xml;
      xml++;
    }

    str++;
    slen++;
  }
  *str = 0;

  return slen;
}


/*-----------------------------------------------------------------------------------------------*/
COO_ARRAY *coo_array_create (int init_count, int percent)
{
  COO_ARRAY *ca = COO_OBJECT_NEW(COO_ARRAY);
  if(ca)
  {
    if(percent > 100)
      percent = 100;
    if(percent < 10)
      percent = 10;
    ca->percent = percent/10;

    if(init_count < 0)
      init_count = 0;
    ca->max_count = init_count;

    if(init_count)
      ca->array = (void*)malloc(init_count * sizeof(void *));
  }
  return ca;
}
void coo_array_insert(COO_ARRAY *ca, int index, void *item)/* index is zero based, must >= 0 */
{
  if(index < 0 || index > ca->real_count)
    return;

  int i, n = ca->real_count;
  if(n >= ca->max_count)
  {
    ca->max_count++;
    ca->max_count += (ca->max_count * ca->percent)/10;
    ca->array = (void*)realloc(ca->array, ca->max_count * sizeof(void *));
  }

  if(ca->array)
  {
    if(index != n)
    {
      for(i = n; i > index; i--)
        ca->array[i] = ca->array[i-1];
    }
    ca->array[index] = item;
    ca->real_count++;
  }
  else
  {
    ca->max_count  = 0;
    ca->real_count = 0;
  }
}

void coo_array_append (COO_ARRAY *ca, void *item)
{
#if 1
  coo_array_insert(ca, ca->real_count, item);
#else	
  int n = ca->real_count;
  if(n >= ca->max_count)
  {
    ca->max_count++;
    ca->max_count += (ca->max_count * ca->percent)/10;
    ca->array = (void*)realloc(ca->array, ca->max_count * sizeof(void *));
  }

  if(ca->array)
  {
    ca->array[n] = item;
    ca->real_count++;
  }
  else
  {
    ca->max_count  = 0;
    ca->real_count = 0;
  }
#endif	
}

void coo_array_delete(COO_ARRAY *ca, int index)/* index is zero based, must >= 0 */
{
  int n = ca->real_count;

  if(index < 0 || index >= n)
    return;

  for(; index < n; index++)
    ca->array[index] = ca->array[index+1];

  ca->real_count--;
}

void coo_array_switch(COO_ARRAY *ca, int x, int y)/* x, y is zero based, must >= 0 */
{
  void *item;
  int n = ca->real_count;

  if(x < 0 || x >= n)
    return;
  if(y < 0 || y >= n)
    return;

  item = ca->array[x];
  ca->array[x] = ca->array[y]	;
  ca->array[y] = item;
}

int coo_array_real_count (COO_ARRAY *ca)
{
  return ca->real_count;
}

void *coo_array_get (COO_ARRAY *ca, int index)/* index is zero based, must >= 0 */
{
  if(index < 0 || index >= ca->real_count)
    return NULL;

  return ca->array? ca->array[index] : NULL;
}

void coo_array_free (COO_ARRAY *ca)
{
  if(ca)
  {
    if(ca->array)
      free(ca->array);
    free(ca);
  }
}

/*--------------------------------------------------------------------------------------------------------*/
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>

int coo_util_get_mac (char *lan_interface, char **mac_address)
{
#define MAC_LENGTH	6
  //	int mac_len = 6;
  int sock = -1, ret = -1;
  char *ptr;
  struct ifreq ifr;

  if(!lan_interface || !mac_address)
    goto s_EXIT;

  ret--;
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    goto s_EXIT;

  strcpy (ifr.ifr_name, lan_interface);
  strcpy (ifr.ifr_hwaddr.sa_data, "");

  ret--;
  if (ioctl (sock, SIOCGIFHWADDR, &ifr) < 0)
    goto s_EXIT;

  *mac_address = coo_malloc_bzero(MAC_LENGTH + 2);
  ptr = (char *)ifr.ifr_hwaddr.sa_data;

  memcpy(*mac_address, ptr, MAC_LENGTH);
  (*mac_address)[MAC_LENGTH] = 0;
  ret = 0;

s_EXIT:
  if(sock > -1)
    close(sock);

  return ret;
}




/*--------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------*/
/*
 * $Id: json_tokener.c,v 1.20 2006/07/25 03:24:50 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 *
 * Copyright (c) 2008-2009 Yahoo! Inc.  All rights reserved.
 * The copyrights to the contents of this file are licensed under the MIT License
 * (http://www.opensource.org/licenses/mit-license.php)
 */

//#include "config.h"
#define HAVE_STRNCASECMP 1
#define HAVE_STRNDUP 1

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>

#include "bits.h"
#include "debug.h"
#include "printbuf.h"
#include "arraylist.h"
#include "json_object.h"
#include "json_tokener.h"


#if !HAVE_STRNCASECMP && defined(_MSC_VER)
/* MSC has the version as _strnicmp */
# define strncasecmp _strnicmp
#elif !HAVE_STRNCASECMP
# error You do not have strncasecmp on your system.
#endif /* HAVE_STRNCASECMP */

static const char* json_null_str = "null";
static const char* json_true_str = "true";
static const char* json_false_str = "false";

#if 0
const char* json_tokener_errors[] = {
  "success",
  "continue",
  "nesting to deep",
  "unexpected end of data",
  "unexpected character",
  "null expected",
  "boolean expected",
  "number expected",
  "array value separator ',' expected",
  "quoted object property name expected",
  "object property name separator ':' expected",
  "object value separator ',' expected",
  "invalid string sequence",
  "expected comment",
};


struct json_tokener* json_tokener_new(void)
{
  struct json_tokener *tok;

  tok = (struct json_tokener*)calloc(1, sizeof(struct json_tokener));
  if (!tok) return NULL;
  tok->pb = printbuf_new();
  json_tokener_reset(tok);
  return tok;
}

void json_tokener_free(struct json_tokener *tok)
{
  json_tokener_reset(tok);
  if(tok) printbuf_free(tok->pb);
  free(tok);
}
#endif
static void json_tokener_reset_level(struct json_tokener *tok, int depth)
{
  tok->stack[depth].state = json_tokener_state_eatws;
  tok->stack[depth].saved_state = json_tokener_state_start;
  json_object_put(tok->stack[depth].current);
  tok->stack[depth].current = NULL;
  free(tok->stack[depth].obj_field_name);
  tok->stack[depth].obj_field_name = NULL;
}
#if 0
void json_tokener_reset(struct json_tokener *tok)
{
  int i;
  if (!tok)
    return;

  for(i = tok->depth; i >= 0; i--)
    json_tokener_reset_level(tok, i);
  tok->depth = 0;
  tok->err = json_tokener_success;
}
#endif



#if !HAVE_STRNDUP
/* CAW: compliant version of strndup() */
char* strndup(const char* str, size_t n)
{
  if(str) {
    size_t len = strlen(str);
    size_t nn = json_min(len,n);
    char* s = (char*)malloc(sizeof(char) * (nn + 1));

    if(s) {
      memcpy(s, str, nn);
      s[nn] = '\0';
    }

    return s;
  }

  return NULL;
}
#endif

#define state  tok->stack[tok->depth].state
#define saved_state  tok->stack[tok->depth].saved_state
#define current tok->stack[tok->depth].current
#define obj_field_name tok->stack[tok->depth].obj_field_name

/* Optimization:
 * dlna_json_tokener_parse_ex() consumed a lot of CPU in its main loop,
 * iterating character-by character.  A large performance boost is
 * achieved by using tighter loops to locally handle units such as
 * comments and strings.  Loops that handle an entire token within 
 * their scope also gather entire strings and pass them to 
 * printbuf_memappend() in a single call, rather than calling
 * printbuf_memappend() one char at a time.
 *
 * POP_CHAR() and ADVANCE_CHAR() macros are used for code that is
 * common to both the main loop and the tighter loops.
 */

/* POP_CHAR(dest, tok) macro:
 *   Not really a pop()...peeks at the current char and stores it in dest.
 *   Returns 1 on success, sets tok->err and returns 0 if no more chars.
 *   Implicit inputs:  str, len vars
 */
#define POP_CHAR(dest, tok)                                                  \
  (((tok)->char_offset == len) ?                                          \
   (((tok)->depth == 0 && state == json_tokener_state_eatws && saved_state == json_tokener_state_finish) ? \
    (((tok)->err = json_tokener_success), 0)                              \
    :                                                                   \
    (((tok)->err = json_tokener_continue), 0)                             \
   ) :                                                                 \
   (((dest) = *str), 1)                                                 \
  )

/* ADVANCE_CHAR() macro:
 *   Incrementes str & tok->char_offset.
 *   For convenience of existing conditionals, returns the old value of c (0 on eof)
 *   Implicit inputs:  c var
 */
#define ADVANCE_CHAR(str, tok) \
  ( ++(str), ((tok)->char_offset)++, c)

/* End optimization macro defs */


static struct json_object* dlna_json_tokener_parse_ex(struct json_tokener *tok,
  const char *str, int len)
{
  struct json_object *obj = NULL;
  char c = '\1';

  tok->char_offset = 0;
  tok->err = json_tokener_success;

  while (POP_CHAR(c, tok)) {

redo_char:
    switch(state) {

    case json_tokener_state_eatws:
      /* Advance until we change state */
      while (isspace(c)) {
        if ((!ADVANCE_CHAR(str, tok)) || (!POP_CHAR(c, tok)))
          goto out;
      }
      if(c == '/') {
        printbuf_reset(tok->pb);
        printbuf_memappend_fast(tok->pb, &c, 1);
        state = json_tokener_state_comment_start;
      } else {
        state = saved_state;
        goto redo_char;
      }
      break;

    case json_tokener_state_start:
      switch(c) {
      case '{':
        state = json_tokener_state_eatws;
        saved_state = json_tokener_state_object_field_start;
        current = json_object_new_object();
        break;
      case '[':
        state = json_tokener_state_eatws;
        saved_state = json_tokener_state_array;
        current = json_object_new_array();
        break;
      case 'N':
      case 'n':
        state = json_tokener_state_null;
        printbuf_reset(tok->pb);
        tok->st_pos = 0;
        goto redo_char;
      case '"':
      case '\'':
        state = json_tokener_state_string;
        printbuf_reset(tok->pb);
        tok->quote_char = c;
        break;
      case 'T':
      case 't':
      case 'F':
      case 'f':
        state = json_tokener_state_boolean;
        printbuf_reset(tok->pb);
        tok->st_pos = 0;
        goto redo_char;
#if defined(__GNUC__)
      case '0' ... '9':
#else
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
#endif
      case '-':
        state = json_tokener_state_number;
        printbuf_reset(tok->pb);
        tok->is_double = 0;
        goto redo_char;
      default:
        tok->err = json_tokener_error_parse_unexpected;
        goto out;
      }
      break;

    case json_tokener_state_finish:
      if(tok->depth == 0) goto out;
      obj = json_object_get(current);
      json_tokener_reset_level(tok, tok->depth);
      tok->depth--;
      goto redo_char;

    case json_tokener_state_null:
      printbuf_memappend_fast(tok->pb, &c, 1);
      if(strncasecmp(json_null_str, tok->pb->buf,
          json_min(tok->st_pos+1, strlen(json_null_str))) == 0) {
        if(tok->st_pos == strlen(json_null_str)) {
          current = NULL;
          saved_state = json_tokener_state_finish;
          state = json_tokener_state_eatws;
          goto redo_char;
        }
      } else {
        tok->err = json_tokener_error_parse_null;
        goto out;
      }
      tok->st_pos++;
      break;

    case json_tokener_state_comment_start:
      if(c == '*') {
        state = json_tokener_state_comment;
      } else if(c == '/') {
        state = json_tokener_state_comment_eol;
      } else {
        tok->err = json_tokener_error_parse_comment;
        goto out;
      }
      printbuf_memappend_fast(tok->pb, &c, 1);
      break;

    case json_tokener_state_comment:
      {
        /* Advance until we change state */
        const char *case_start = str;
        while(c != '*') {
          if (!ADVANCE_CHAR(str, tok) || !POP_CHAR(c, tok)) {
            printbuf_memappend_fast(tok->pb, case_start, str-case_start);
            goto out;
          } 
        }
        printbuf_memappend_fast(tok->pb, case_start, 1+str-case_start);
        state = json_tokener_state_comment_end;
      }
      break;

    case json_tokener_state_comment_eol:
      {
        /* Advance until we change state */
        const char *case_start = str;
        while(c != '\n') {
          if (!ADVANCE_CHAR(str, tok) || !POP_CHAR(c, tok)) {
            printbuf_memappend_fast(tok->pb, case_start, str-case_start);
            goto out;
          }
        }
        printbuf_memappend_fast(tok->pb, case_start, str-case_start);
        MC_DEBUG("json_tokener_comment: %s\n", tok->pb->buf);
        state = json_tokener_state_eatws;
      }
      break;

    case json_tokener_state_comment_end:
      printbuf_memappend_fast(tok->pb, &c, 1);
      if(c == '/') {
        MC_DEBUG("json_tokener_comment: %s\n", tok->pb->buf);
        state = json_tokener_state_eatws;
      } else {
        state = json_tokener_state_comment;
      }
      break;

    case json_tokener_state_string:
      {
        /* Advance until we change state */
        const char *case_start = str;
        while(1) {
          if(c == tok->quote_char) {
            printbuf_memappend_fast(tok->pb, case_start, str-case_start);
            current = json_object_new_string(tok->pb->buf);
            saved_state = json_tokener_state_finish;
            state = json_tokener_state_eatws;
            break;
          } else if(c == '\\') {
            printbuf_memappend_fast(tok->pb, case_start, str-case_start);
            saved_state = json_tokener_state_string;
            state = json_tokener_state_string_escape;
            break;
          }
          if (!ADVANCE_CHAR(str, tok) || !POP_CHAR(c, tok)) {
            printbuf_memappend_fast(tok->pb, case_start, str-case_start);
            goto out;
          }
        }
      }
      break;

    case json_tokener_state_string_escape:
      switch(c) {
      case '"':
      case '\\':
      case '/':
        printbuf_memappend_fast(tok->pb, &c, 1);
        state = saved_state;
        break;
      case 'b':
      case 'n':
      case 'r':
      case 't':
        if(c == 'b') printbuf_memappend_fast(tok->pb, "\b", 1);
        else if(c == 'n') printbuf_memappend_fast(tok->pb, "\n", 1);
        else if(c == 'r') printbuf_memappend_fast(tok->pb, "\r", 1);
        else if(c == 't') printbuf_memappend_fast(tok->pb, "\t", 1);
        state = saved_state;
        break;
      case 'u':
        tok->ucs_char = 0;
        tok->st_pos = 0;
        state = json_tokener_state_escape_unicode;
        break;
      default:
        tok->err = json_tokener_error_parse_string;
        goto out;
      }
      break;

    case json_tokener_state_escape_unicode:
      /* Note that the following code is inefficient for handling large
       * chunks of extended chars, calling printbuf_memappend() once
       * for each multi-byte character of input.
       * This is a good area for future optimization.
       */
      {
        /* Advance until we change state */
        while(1) {
          if(strchr(json_hex_chars, c)) {
            tok->ucs_char += ((unsigned int)hexdigit(c) << ((3-tok->st_pos++)*4));
            if(tok->st_pos == 4) {
              unsigned char utf_out[3];
              if (tok->ucs_char < 0x80) {
                utf_out[0] = tok->ucs_char;
                printbuf_memappend_fast(tok->pb, (char*)utf_out, 1);
              } else if (tok->ucs_char < 0x800) {
                utf_out[0] = 0xc0 | (tok->ucs_char >> 6);
                utf_out[1] = 0x80 | (tok->ucs_char & 0x3f);
                printbuf_memappend_fast(tok->pb, (char*)utf_out, 2);
              } else {
                utf_out[0] = 0xe0 | (tok->ucs_char >> 12);
                utf_out[1] = 0x80 | ((tok->ucs_char >> 6) & 0x3f);
                utf_out[2] = 0x80 | (tok->ucs_char & 0x3f);
                printbuf_memappend_fast(tok->pb, (char*)utf_out, 3);
              }
              state = saved_state;
              break;
            }
          } else {
            tok->err = json_tokener_error_parse_string;
            goto out;
          }
          if (!ADVANCE_CHAR(str, tok) || !POP_CHAR(c, tok))
            goto out;
        }
      }
      break;

    case json_tokener_state_boolean:
      printbuf_memappend_fast(tok->pb, &c, 1);
      if(strncasecmp(json_true_str, tok->pb->buf,
          json_min(tok->st_pos+1, strlen(json_true_str))) == 0) {
        if(tok->st_pos == strlen(json_true_str)) {
          current = json_object_new_boolean(1);
          saved_state = json_tokener_state_finish;
          state = json_tokener_state_eatws;
          goto redo_char;
        }
      } else if(strncasecmp(json_false_str, tok->pb->buf,
          json_min(tok->st_pos+1, strlen(json_false_str))) == 0) {
        if(tok->st_pos == strlen(json_false_str)) {
          current = json_object_new_boolean(0);
          saved_state = json_tokener_state_finish;
          state = json_tokener_state_eatws;
          goto redo_char;
        }
      } else {
        tok->err = json_tokener_error_parse_boolean;
        goto out;
      }
      tok->st_pos++;
      break;

    case json_tokener_state_number:
      {
        /* Advance until we change state */
        const char *case_start = str;
        int case_len=0;
        while(c && strchr(json_number_chars, c)) {
          ++case_len;
          if(c == '.' || c == 'e') tok->is_double = 1;
          if (!ADVANCE_CHAR(str, tok) || !POP_CHAR(c, tok)) {
            printbuf_memappend_fast(tok->pb, case_start, case_len);
            goto out;
          }
        }
        if (case_len>0)
          printbuf_memappend_fast(tok->pb, case_start, case_len);
      }
      {
        int numi;
        double numd;
        if(!tok->is_double && sscanf(tok->pb->buf, "%d", &numi) == 1) {
          current = json_object_new_int(numi);
        } else if(tok->is_double && sscanf(tok->pb->buf, "%lf", &numd) == 1) {
          current = json_object_new_double(numd);
        } else {
          tok->err = json_tokener_error_parse_number;
          goto out;
        }
        saved_state = json_tokener_state_finish;
        state = json_tokener_state_eatws;
        goto redo_char;
      }
      break;

    case json_tokener_state_array:
      if(c == ']') {
        saved_state = json_tokener_state_finish;
        state = json_tokener_state_eatws;
      } else {
        if(tok->depth >= JSON_TOKENER_MAX_DEPTH-1) {
          tok->err = json_tokener_error_depth;
          goto out;
        }
        state = json_tokener_state_array_add;
        tok->depth++;
        json_tokener_reset_level(tok, tok->depth);
        goto redo_char;
      }
      break;

    case json_tokener_state_array_add:
      json_object_array_add(current, obj);
      saved_state = json_tokener_state_array_sep;
      state = json_tokener_state_eatws;
      goto redo_char;

    case json_tokener_state_array_sep:
      if(c == ']') {
        saved_state = json_tokener_state_finish;
        state = json_tokener_state_eatws;
      } else if(c == ',') {
        saved_state = json_tokener_state_array;
        state = json_tokener_state_eatws;
      } else {
        tok->err = json_tokener_error_parse_array;
        goto out;
      }
      break;

    case json_tokener_state_object_field_start:
      if(c == '}') {
        saved_state = json_tokener_state_finish;
        state = json_tokener_state_eatws;
      } else if (c == '"' || c == '\'') {
        tok->quote_char = c;
        printbuf_reset(tok->pb);
        state = json_tokener_state_object_field;
      } else {
        tok->err = json_tokener_error_parse_object_key_name;
        goto out;
      }
      break;

    case json_tokener_state_object_field:
      {
        /* Advance until we change state */
        const char *case_start = str;
        while(1) {
          if(c == tok->quote_char) {
            printbuf_memappend_fast(tok->pb, case_start, str-case_start);
            obj_field_name = strdup(tok->pb->buf);
            saved_state = json_tokener_state_object_field_end;
            state = json_tokener_state_eatws;
            break;
          } else if(c == '\\') {
            printbuf_memappend_fast(tok->pb, case_start, str-case_start);
            saved_state = json_tokener_state_object_field;
            state = json_tokener_state_string_escape;
            break;
          }
          if (!ADVANCE_CHAR(str, tok) || !POP_CHAR(c, tok)) {
            printbuf_memappend_fast(tok->pb, case_start, str-case_start);
            goto out;
          }
        }
      }
      break;

    case json_tokener_state_object_field_end:
      if(c == ':') {
        saved_state = json_tokener_state_object_value;
        state = json_tokener_state_eatws;
      } else {
        tok->err = json_tokener_error_parse_object_key_sep;
        goto out;
      }
      break;

    case json_tokener_state_object_value:
      if(tok->depth >= JSON_TOKENER_MAX_DEPTH-1) {
        tok->err = json_tokener_error_depth;
        goto out;
      }
      state = json_tokener_state_object_value_add;
      tok->depth++;
      json_tokener_reset_level(tok, tok->depth);
      goto redo_char;

    case json_tokener_state_object_value_add:
      json_object_object_add(current, obj_field_name, obj);
      free(obj_field_name);
      obj_field_name = NULL;
      saved_state = json_tokener_state_object_sep;
      state = json_tokener_state_eatws;
      goto redo_char;

    case json_tokener_state_object_sep:
      if(c == '}') {
        saved_state = json_tokener_state_finish;
        state = json_tokener_state_eatws;
      } else if(c == ',') {
        saved_state = json_tokener_state_object_field_start;
        state = json_tokener_state_eatws;
      } else {
        tok->err = json_tokener_error_parse_object_value_sep;
        goto out;
      }
      break;

    }
    if (!ADVANCE_CHAR(str, tok))
      goto out;
  } /* while(POP_CHAR) */

out:
  if (!c) { /* We hit an eof char (0) */
    if(state != json_tokener_state_finish &&
      saved_state != json_tokener_state_finish)
      tok->err = json_tokener_error_parse_eof;
  }

  if(tok->err == json_tokener_success) return json_object_get(current);
  MC_DEBUG("dlna_json_tokener_parse_ex: error %s at offset %d\n",
    json_tokener_errors[tok->err], tok->char_offset);
  return NULL;
}

struct json_object* dlna_json_tokener_parse(const char *str)
  //struct json_object* dlna_json_tokener_parse(char *str)
{
  struct json_tokener* tok;
  struct json_object* obj;

  char *s = (char*)str, *ss = (char*)str;

  if(!s)
    return NULL;

  while(CDS_CHAR_SPACE == *s)
    s++;
  if(*s != '{')
    return NULL;

  s = ss + strlen(ss) - 1;
  while(CDS_CHAR_SPACE == *s)
    s--;
  if(*s != '}')
    return NULL;

  tok = json_tokener_new();
  obj = dlna_json_tokener_parse_ex(tok, str, -1);
  if(tok->err != json_tokener_success)
    //    obj = (struct json_object*)error_ptr(-tok->err);
    obj=NULL;
  json_tokener_free(tok);
  return obj;
}


