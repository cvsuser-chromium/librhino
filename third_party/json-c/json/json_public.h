/*******************************************************************************
	公司：
			Yuxing software
	纪录：
			2011-1-19 21:12:26 create by ****
	模块：
			json封装及解析模块公共头文件，供外部使用
	简述：
			定义了json封装和解析所需的公共函数，并在文件末尾附有使用范例
 *******************************************************************************/
#ifndef H_JSON_PUBLIC_H
#define H_JSON_PUBLIC_H

#include "json.h"

#ifdef __cplusplus
extern "C" {
#endif

struct json_object* json_object_create_int(const int i);

struct json_object* json_object_create_string(const char *s);

struct json_object* json_object_create_boolean(json_bool b);

struct json_object* json_object_create_double(double d);

/****/

struct json_object* json_object_create_array(void);

const char* json_object2json_string(struct json_object *jso);

struct json_object* json_object_create_object(void);

void json_object_delete(struct json_object *obj);

void json_object_add_object(struct json_object* obj, const char *key, struct json_object *val);

int json_array_add_object(struct json_object *obj,struct json_object *val);

void json_object_del_object(struct json_object* obj, const char *key);



/**********
jason type define
typedef enum json_type {
  json_type_null,
  json_type_boolean,
  json_type_double,
  json_type_int,
  json_type_object,
  json_type_array,
  json_type_string
} json_type;
*****/

enum json_type json_get_object_type(struct json_object *obj);

json_bool json_get_object_boolean(struct json_object *obj);

int json_get_object_int(struct json_object *obj);

double json_get_object_double(struct json_object *obj);

const char* json_get_object_string(struct json_object *obj);

struct array_list* json_get_object_array(struct json_object *obj);

int json_get_object_array_length(struct json_object *obj);

int json_object_put_array_idx(struct json_object *obj, int idx,struct json_object *val);

struct json_object* json_object_get_array_idx(struct json_object *obj,int idx);

struct json_object* json_object_get_object_bykey(struct json_object* obj,const char *key);

struct json_object* json_tokener_parse_string(const char *str);
#ifdef __cplusplus
}
#endif

#endif
/******************  封装json的使用范例*****************************
*******************************************************************
{ "filecout": 1, "file": [ { "filename": "3.ts", "prog_id": "2011-01-19SSD", "channel_num": 8 } ] }
 实现如下：
 my_array = json_object_create_array();
 my_object= json_object_create_object();
 json_object_object_add(my_object,"filecout",json_object_create_int(1));
 new_obj =json_object_create_object();
 json_object_add_object(new_obj,"filename",json_object_new_string("3.ts"));
 json_object_add_object(new_obj,"prog_id",json_object_new_string("2011-01-19SSD"));
 json_object_add_object(new_obj,"channel_num",json_object_new_int(8));
 json_object_add_object(my_array,new_obj);
 json_object_object_add(my_object,"files",my_array);
 printf("my_object:%s\n",json_object_2_json_string(my_object));
 ****
 ******************************************************************
 *************************解析json的使用范例**********************
 ** { "filecout": 1, "file": [ { "filename": "3.ts", "prog_id": "2011-01-19SSD", "channel_num": 8 } ] }

  my_object=json_tokener_parse_string("{ \"filecout\": 1, \"file\": [ { \"filename\": \"3.ts\", \"prog_id\": \"2011-01-19SSD\", \"channel_num\": 8 } ] }");
  new_obj=json_object_get_object_bykey(my_object,"filecout");
  printf("\n filecount:%d\n",json_object_get_int(new_obj));
  new_obj=json_object_object_get(my_object,"files");

  if(json_get_object_type(new_obj)==json_type_array)
  {
     json_object *obj1 = json_object_get_array_idx(my_array, 0);
     json_object *obj2 = json_object_get_object_bykey(obj1,"filename");
     printf("\nfilename:%s\n",json_object_get_string(obj2));
     obj2 = json_object_get_object_bykey(obj1,"prog_id");
     printf("\nprog_id:%s\n",json_get_object_string(obj2));
     obj2 = json_object_get_object_bykey(obj1,"channel_num");
     printf("\nchannel_num:%d\n",json_get_object_int(obj2));
  }
************************************************************************************************/

