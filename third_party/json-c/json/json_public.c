#include "json/json_public.h"

struct json_object* json_object_create_int(const int i)
{
	return json_object_new_int(i);
}

struct json_object* json_object_create_string(const char *s)
{
	return json_object_new_string(s);
}

struct json_object* json_object_create_boolean(json_bool b)
{
		return json_object_new_boolean(b);
}

struct json_object* json_object_create_double(double d)
{
	 return json_object_new_double(d);
}

struct json_object* json_object_create_array(void)
{
	return json_object_new_array();
}

const char* json_object2json_string(struct json_object *jso)
{
	return json_object_to_json_string(jso);
}

struct json_object* json_object_create_object(void)
{
	return json_object_new_object();
}

void json_object_add_object(struct json_object* obj, const char *key, struct json_object *val)
{
	json_object_object_add(obj,key,val);
}

void json_object_del_object(struct json_object* obj, const char *key)
{
	 json_object_object_del(obj,key);
}

int json_array_add_object(struct json_object *obj,struct json_object *val)
{
	return json_object_array_add(obj,val);
}

void json_object_delete(struct json_object *obj)
{
	json_object_put(obj);
}


enum json_type json_get_object_type(struct json_object *obj)
{
	 return json_object_get_type(obj);
}

json_bool json_get_object_boolean(struct json_object *obj)
{
	 return json_object_get_boolean(obj);
}

int json_get_object_int(struct json_object *obj)
{
	return json_object_get_int(obj);
}

double json_get_object_double(struct json_object *obj)
{
	return json_object_get_double(obj);
}

const char* json_get_object_string(struct json_object *obj)
{
	return json_object_get_string(obj);
}

struct array_list* json_get_object_array(struct json_object *obj)
{
	return json_object_get_array(obj);
}

int json_get_object_array_length(struct json_object *obj)
{
	return json_object_array_length(obj);
}

int json_object_put_array_idx(struct json_object *obj, int idx,struct json_object *val)
{
	return json_object_array_put_idx(obj,idx,val);
}

struct json_object* json_object_get_array_idx(struct json_object *obj,int idx)
{
	return json_object_array_get_idx(obj,idx);
}

struct json_object* json_object_get_object_bykey(struct json_object* obj,const char *key)
{
	return json_object_object_get(obj,key);
}

struct json_object* json_tokener_parse_string(const char *str)
{
	return json_tokener_parse(str);
}

