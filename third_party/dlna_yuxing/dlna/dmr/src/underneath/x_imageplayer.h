

/*--------------------------------------------------------------*/
/*
* Golden Yuxing CONFIDENTIAL
* Copyright (c) 2003, 2004 Yuxing Corporation.  All rights reserved.
* 
* The computer program contained herein contains proprietary information
* which is the property of Golden Yuxing Co., Ltd.  The program may be used
* and/or copied only with the written permission of Golden Yuxing Co., Ltd.
* or in accordance with the terms and conditions stipulated in the
* agreement/contract under which the programs have been supplied.
*
*    filename:               mediaPlayerEx.h
*    author(s):              valiant
*    version:                ver 0.1
*    date:					 2005/03/15
* History
* ----------------
*/
/*---------------------------------------------------------*/
/*---------------------------------------------------------*/
#ifndef _X_IMAGE_PLAYER_H_
#define _X_IMAGE_PLAYER_H_

#include "dlna_type.h"

typedef struct _dmr_image_player_ t_DMR_IMGP;
struct _dmr_image_player_ 
{
	t_PLAYER_EVENT	callback;
	void*				user;
	int					player;

	char*				url;
	t_MEDIA_INFO		minfo;
	int 				cancel;

	int				(*SetUri)(t_DMR_IMGP *player, char *uri, t_MEDIA_INFO *mInfo);
	int				(*Play)(t_DMR_IMGP *player, int slideMode);
	int				(*Zoom)(t_DMR_IMGP *player, int percent);
	int				(*Move)(t_DMR_IMGP *player, int x, int y);
	int				(*Rotate)(t_DMR_IMGP *player, int radian);
	int				(*Stop)(t_DMR_IMGP *player);
	t_MEDIA_INFO*	(*GetMediaInfo)(t_DMR_IMGP *player);

	void			(*Release)(t_DMR_IMGP *player);
};

t_DMR_IMGP* DMR_IPlayer_Create(t_PLAYER_EVENT callBack, void *user);
void DMR_IPlayer_Release(t_DMR_IMGP *player);
int DMR_IPlayer_SetUri(t_DMR_IMGP *player, char *uri, t_MEDIA_INFO *mInfo);
int DMR_IPlayer_Play(t_DMR_IMGP *player, int slideMode);
int DMR_IPlayer_Zoom(t_DMR_IMGP *player, int percent);
int DMR_IPlayer_Move(t_DMR_IMGP *player, int x, int y);
int DMR_IPlayer_Rotate(t_DMR_IMGP *player, int radian);
int DMR_IPlayer_Stop(t_DMR_IMGP *player);
t_MEDIA_INFO* DMR_IPlayer_GetMediaInfo(t_DMR_IMGP *player);
#endif /* _X_IMAGE_PLAYER_H_ */
/*---------------------------------------------------------*/

