#ifndef _DLNA_API_CPP_H
#define _DLNA_API_CPP_H

#include <string>
/* dmr */
typedef void (*DMREVRNT_CALLBACK_CPP)(int eventID, const char* eventArg, std::string* arg);

/*basic  dmr interface */
int Dmr_Init(char *path, char *name);
int Dmr_Start(DMREVRNT_CALLBACK_CPP);
int Dmr_Stop(void);
int Dmr_DealRemoteRequest( int AcceptOrNot);
int Raw_Dmr_Init(const char *path,const char *name,const char *mac_addr);
int Raw_Dmr_Start(DMREVRNT_CALLBACK_CPP Callback);

#endif /* _DLNA_API_CPP_H */

