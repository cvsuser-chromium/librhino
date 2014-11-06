#ifndef DTCP_CLIENT_H
#define DTCP_CLIENT_H

int dtcp_client_init(void);
int dtcp_client_close(void);
void * dtcp_client_do_ake(char *remote_ip, unsigned  short remote_dtcp_port);
int dtcp_client_sink_create(char *remote_ip, unsigned  short remote_dtcp_port);
int dtcp_client_sink_destroy(void);
void dtcp_client_buf_get(char **addr, int *buf_len);
int dtcp_client_clear_data(char *encrypted_data, unsigned int encrypted_data_size, char *clear_content, unsigned int *clear_content_size);

void dtcp_client_buf_get(char **addr, int *buf_len);
void dtcp_client_buf_reset(void);

#endif
