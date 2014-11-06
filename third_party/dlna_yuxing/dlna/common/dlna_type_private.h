#ifndef _DLNA_TYPE_PRIVATE_H_
#define _DLNA_TYPE_PRIVATE_H_

typedef	char*	t_DLNA_OBJECTID;
typedef	char*	t_DLNA_JSON;

/* for yuxing fake object connects to true function(method)*/
typedef int		(*t_DISPATCH_FUNC)(int hnd, char* funcName, char*para, char *value, int len);

#define DEFAULT_DISPATCH_FUNC		t_DISPATCH_FUNC	DispatchFunc	
typedef struct _dmc_base_s_ {
	DEFAULT_DISPATCH_FUNC;
}ClassDmcBase, *pClassDmcBase;


#endif /* _DLNA_TYPE_PRIVATE_H_ */

