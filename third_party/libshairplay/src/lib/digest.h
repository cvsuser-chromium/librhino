#ifndef SRC_LIB_DIGEST_H
#define SRC_LIB_DIGEST_H

#ifdef __cplusplus
extern "C" {
#endif

void digest_generate_nonce(char *result, int resultlen);
int digest_is_valid(const char *our_realm, const char *password,
                    const char *our_nonce, const char *method,
                    const char *our_uri, const char *authorization);

#ifdef __cplusplus
}
#endif
#endif
