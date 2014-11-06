#ifndef _DLNA_DMS_H_
#define _DLNA_DMS_H_

#include <sys/types.h>	/* off_t */
#include <stddef.h>		/* size_t */
#include <time.h>		/* time_t */
#include <pthread.h>
#include <semaphore.h>

#include "dlna_audio_type.h"
#include "coo.h"
#include "json.h"

extern struct ushare_t *ut;

typedef enum _e_SHARING_MEDIA_SOURCE_ {
	e_SOURCE_ROOT = 0,
	e_SOURCE_USB,
	e_SOURCE_PVR,
	e_SOURCE_VOD,
	e_SOURCE_DVB_C,
} e_SOURCE;

typedef enum _e_OBJECT_TYPE_ {
	e_OBJTYPE_ITEM = 1,
	e_OBJTYPE_CONTAINER,
	e_OBJTYPE_BOOKMARK,
} e_OBJTYPE;

typedef enum _e_ID_ERROR_ {
	e_IDE_GENERAL			= -100,
	e_IDE_INVALID_PARENT_ID,
	e_IDE_INVALID_CHILD_ID,
	e_IDE_INVALID_INFO,
	
	e_IDE_ENTRY_NULL,
	e_IDE_ENTRY_UNSUPPORTED,
	e_IDE_NO_OBJCT,
	e_IDE_NO_KEYWORD,
	e_IDE_NO_MIME_TYPE,
	e_IDE_NO_PROTOCOL_INFO,
	
} e_IDE;

typedef unsigned int					uint;
typedef unsigned short					ushort;
typedef unsigned char					uchar;
typedef struct _c_dms_virtual_object_	C_DMS_VO;
typedef struct _c_dms_async_share_		C_DMS_AS;
typedef int (*t_DMS_ADD_FUNC)(C_DMS_AS *dmsas, int parent_id, char *keyword, int user);
typedef void (*t_DMS_FREE_OBJECT)(void *object);
typedef void (*t_DMS_MAKE_OBJECTID)(C_DMS_VO *object, int index, char *buf, int buf_len);
typedef void *(* VFUNCV)(void *arg);


typedef struct _c_dms_file_info_
{
  /** The length of the file. A length less than 0 indicates the size 
   *  is unknown, and data will be sent until 0 bytes are returned from
   *  a read call. */
//  off_t file_length;
  long long  file_length;

  /** The time at which the contents of the file was modified;
   *  The time system is always local (not GMT). */
  time_t last_modified;

  /** If the file is a directory, {\bf is_directory} contains
   * a non-zero value. For a regular file, it should be 0. */
  int is_directory;

  /** If the file or directory is readable, this contains 
   * a non-zero value. If unreadable, it should be set to 0. */
  int is_readable;

  /** The content type of the file. This string needs to be allocated 
   *  by the caller using {\bf ixmlCloneDOMString}.  When finished 
   *  with it, the SDK frees the {\bf DOMString}. */
}C_FILE_INFO;


typedef int (*t_DMS_CMP_OBJECT)(C_DMS_VO*x, C_DMS_VO*y);
typedef struct _c_dms_virtual_file_operation_ {
	int				(*Init)(int is_ipc);
	int 			(*GetInfo)(const char *filename, C_FILE_INFO *info);
	int 			(*Open)(const char *filename, int mode);
	int 			(*Read)(int fileHnd, char *buf, size_t buflen);
	int 			(*ReadEx)(int fileHnd, char *buf, size_t buflen, int *flag);
	int 			(*Write)(int fileHnd, char *buf, size_t buflen);
	int 			(*Seek)(int fileHnd, off_t offset, int origin);
	void 			(*Close)(int fileHnd);
}C_DMS_VFO;


typedef struct _c_dms_container_info_ {
	const char		*upnp_class;		/* upnp_class */
} C_DMS_CI;
typedef struct _cx_dms_container_info_ {
	char			*upnp_class;		  /* upnp_class */
} x_DMS_CI;

typedef struct _c_dms_item_protocol_info_ {
  const char		*id;		/* Profile ID, part of DLNA.ORG_PN= string */
  const char		*mime;		/* Profile MIME type */
  const char		*label;		/* Profile Label */
  uint				class;		/* Profile type: IMAGE / AUDIO / AV */
} C_DMS_IPI;

/* x_DMS_IPI is alias of C_DMS_IPI */
typedef struct _x_dms_item_protocol_info_ {
	char			*id;		/* Profile ID, part of DLNA.ORG_PN= string */
	char			*mime;		/* Profile MIME type */
	char			*label;		/* Profile Label */
	uint			class;		/* Profile type: IMAGE / AUDIO / AV */
} x_DMS_IPI;


typedef struct _c_dms_thumbnail_info_ {
	int				type;		/* 0: embedded JPEG; 1:MJPEG; 2:solid */ 
	uint			offset;
	uint			length;

	ushort			color_depth;	/* if negative, has not been set */
	ushort			reserved;
	ushort			res_width;	
	ushort			res_height;	

	const C_DMS_IPI	*protocol_info;
	uchar			malloced;	/* 0: protocol_info is const; otherwise: protocol_info is malloced */
}C_DMS_THUI;

typedef struct _c_dms_upnp_info_ {
	char			*creator;
	char			*artist;
	char			*actor;
	char			*producer;
	char			*genre;
	char			*description;
}C_DMS_UPNI;

typedef struct _c_dms_pvr_info_ {
	char				*recordedStartDateTime;
	char				*recordedEndDateTime;
}C_DMS_PVRI;

typedef struct _c_dms_tuner_info_ {
	char				*frequency;
	char				*channel;
}C_DMS_TUNI;

typedef struct _c_dms_optional_info_ {
	WAVEFORMATEX		*audio_info;	/* audio information */
	C_DMS_UPNI			*upnp_info;
	C_DMS_PVRI			*pvr;
	C_DMS_TUNI			*tuner;
}C_DMS_OPTI;

typedef struct _c_dms_compact_media_information_
{
	/* basic */
	const C_DMS_IPI		*protocol_info;
	uchar				malloced;	/* 0: protocol_info is const; otherwise: protocol_info is malloced */
	uchar				reserved;
	ushort				color_depth;	/* if negative, has not been set */
	
	char				*title;
	char				*keyword;
	off_t				size;			/* file length */
	time_t				date;			/* original date, really created date */

	/* common */
	uint				duration;		/* total time at ms */
	uint				bitrate;		/* if negative, has not been set */
	ushort				res_width;	
	ushort				res_height;	

	/*  more */
	C_DMS_OPTI			*optional;
	C_DMS_THUI			*thumbnail;

	/* ext */
	struct json_object	*extension;		/* at json */
	char				*user_info;
} C_DMS_CMI;

#define x_DMS_CHILDS_LISTS_NUM	2
typedef struct _c_dms_compact_container_information_
{
	COO_ARRAY			*childs_lists[x_DMS_CHILDS_LISTS_NUM];

	const C_DMS_CI		*mime_type;
	uchar				malloced;	/* 0: mime_type is const; otherwise: mime_type is malloced */
	uchar				minor_type;
	ushort				type;		/* 1 = video . 2 = audio . 3 = picture , 4 = container, otherwise invalid*/

	char				*title;
	char				*keyword;
	time_t				date;		/* original date, really created date */
} C_DMS_CCI;

typedef struct _c_dms_virtual_object_	C_UPNP_ENTRY;
struct _c_dms_virtual_object_ {
	uchar				restricted;
	uchar				searchable;
	uchar 				is_item;	// defined by e_OBJTYPE
	uchar				source_type;// defined by e_SOURCE
	
	C_DMS_VO			*parent;
	
	int 				id;
	uint				updateID;
	void				*object;
	t_DMS_MAKE_OBJECTID	MakeObjectId;
	t_DMS_FREE_OBJECT	FreeObject;
};


extern const C_DMS_CI Container_MIME_Type;

extern int Dms_VirtualFileTree_IsFull(void);
extern int Dms_VirtualFileTree_GetUsbObjectFullpath(int id, char **fullpath);
extern int Dms_VirtualFileTree_AddChildEx (int parent_id, int ca_index, C_DMS_VO *child);
extern C_DMS_VO *Dms_VirtualFileTree_GetObject (int id);
extern C_DMS_VO *Dms_VirtualFileTree_GetRootObject (void);



int Dms_VirtualObject_To_String(C_DMS_VO *entry, int onlybasic, char **info);
int Dms_VirtualObject_From_String(char *info, C_DMS_VO **p_entry);
void Dms_VirtualObject_Trim(C_DMS_VO *entry);

int Dms_VirtualObject_IsItem(C_DMS_VO *entry);
int Dms_VirtualObject_IsContainer(C_DMS_VO *entry);
int Dms_VirtualObject_IsValid(C_DMS_VO *entry);

void Dms_VirtualObject_FreeItem(void *object);
void Dms_VirtualObject_FreeContainerSelf(void *object);
void Dms_VirtualObject_Free(C_DMS_VO *entry);
void Dms_VirtualObject_CreateItem(C_DMS_CMI **pp, const C_DMS_IPI *protocol, const C_DMS_IPI *thumbnail_protocol);
C_DMS_CCI *Dms_VirtualObject_CreateContainerSelf(char *keyword, char *title, const C_DMS_CI *mime_type);
C_DMS_VO *Dms_VirtualObject_Create(int parent_id, void *me, int object_type, int type, t_DMS_MAKE_OBJECTID make_func, t_DMS_FREE_OBJECT free_func);


int Raw_Dms_VirtualFileTree_AddChild(int parent_id, char *info);
void Raw_Dms_VirtualFileTree_DeleteChild(int parent_id, int child_id);
void Raw_Dms_VirtualFileTree_DeleteContainer (int parent_id, int container_id, int keep_if_child);
void Raw_Dms_VirtualFileTree_GetInfo(int id, int onlybasic, char *buf, int len);
int Raw_Dms_VirtualFileTree_GetParentID(int my_id);
int Raw_Dms_VirtualFileTree_GetChildID(int parent_id, char type, char *keyword, int is_recursive);

int Raw_Dms_ShareUsb_AddFolder(int parent_id, char *fullpath);
int Raw_Dms_ShareUsb_RemoveFolder(int parent_id, char *fullpath);

int Dms_AsyncShare_AddObject(C_DMS_AS * dmsas, int parent_id, char *keyword, int user);
int Dms_AsyncShare_RemoveObject(C_DMS_AS *dmsas, int parent_id, char *keyword);
int Dms_AsyncShare_Is_Aborted(C_DMS_AS *dmsas);
int Dms_AsyncShare_Is_Paused(C_DMS_AS *dmsas);
void Dms_AsyncShare_SetPause(C_DMS_AS * dmsas, int mode);
C_DMS_AS *Dms_AsyncShare_Init (t_DMS_ADD_FUNC loop);

void Dms_PresetPvrObjectCmp(t_DMS_CMP_OBJECT cmp_func);
int Dms_PresetPvrFileOperation(C_DMS_VFO *op); /* ret = 0: success; -1: op is null; -2: at least one of Init, Open, Read and Close is null */ 
void Dms_PvrFileOperation_Init(void);

#endif /* _DLNA_DMS_H_ */

