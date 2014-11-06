#include "base/md5.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"

#include <string>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "compat.h"
#include "utils.h"
#include "digest.h"
#include "crypto/crypto.h"

void
digest_md5_to_hex(const unsigned char *md5buf, char *md5hex)
{
	int i;
	for (i=0; i<MD5_SIZE*2; i++) {
		int val = (i%2) ? md5buf[i/2]&0x0f : (md5buf[i/2]&0xf0)>>4;
		md5hex[i] = (val<10) ? '0'+val : 'a'+(val-10);
	}
}

void
digest_get_response(const char *username, const char *realm,
                    const char *password, const char *nonce,
                    const char *method, const char *uri,
                    char *response)
{
#if 1
  std::string md5_1;
  base::StringAppendF(&md5_1, "%s:%s:%s", username, realm, password);
  std::string result_1(base::MD5String(md5_1));

  std::string md5_2;
  base::StringAppendF(&md5_2, "%s:%s", method, uri);
  std::string result_2(base::MD5String(md5_1));

  std::string md5_3;
  base::StringAppendF(&md5_3, "%s:%s:%s", result_1.c_str(), nonce, result_2.c_str());
  std::string result_3(base::MD5String(md5_3));

  //we assume response' length enough to store MD5 value.
  strcpy(response, result_3.c_str());
//#else
  shairplay::MD5_CTX md5ctx;
	unsigned char md5buf[MD5_SIZE];
	char md5hex[MD5_SIZE*2];
	/* Calculate first inner MD5 hash */
	shairplay::MD5_Init(&md5ctx);
	shairplay::MD5_Update(&md5ctx, (const unsigned char *)username, strlen(username));
	shairplay::MD5_Update(&md5ctx, (const unsigned char *)":", 1);
	shairplay::MD5_Update(&md5ctx, (const unsigned char *)realm, strlen(realm));
	shairplay::MD5_Update(&md5ctx, (const unsigned char *)":", 1);
	shairplay::MD5_Update(&md5ctx, (const unsigned char *)password, strlen(password));
	shairplay::MD5_Final(md5buf, &md5ctx);
	digest_md5_to_hex(md5buf, md5hex);

	/* Calculate second inner MD5 hash */
	shairplay::MD5_Init(&md5ctx);
	shairplay::MD5_Update(&md5ctx, (const unsigned char *)method, strlen(method));
	shairplay::MD5_Update(&md5ctx, (const unsigned char *)":", 1);
	shairplay::MD5_Update(&md5ctx, (const unsigned char *)uri, strlen(uri));
	shairplay::MD5_Final(md5buf, &md5ctx);

	/* Calculate outer MD5 hash */
	shairplay::MD5_Init(&md5ctx);
	shairplay::MD5_Update(&md5ctx, (const unsigned char *)md5hex, sizeof(md5hex));
	shairplay::MD5_Update(&md5ctx, (const unsigned char *)":", 1);
	shairplay::MD5_Update(&md5ctx, (const unsigned char *)nonce, strlen(nonce));
	shairplay::MD5_Update(&md5ctx, (const unsigned char *)":", 1);
	digest_md5_to_hex(md5buf, md5hex);
	shairplay::MD5_Update(&md5ctx, (const unsigned char *)md5hex, sizeof(md5hex));
	shairplay::MD5_Final(md5buf, &md5ctx);

	/* Store the final result to response */
	digest_md5_to_hex(md5buf, response);
#endif
}

void
digest_generate_nonce(char *result, int resultlen)
{
#if 1
  unsigned int current_time;
  SYSTEM_GET_TIME(current_time);
  base::StringPiece src;
  src.set(&current_time, sizeof(current_time));
  std::string out = base::MD5String(src);

  strncpy(result, out.c_str(), resultlen-1);
#else
	shairplay::MD5_CTX md5ctx;
	unsigned char md5buf[MD5_SIZE];
	char md5hex[MD5_SIZE*2];
	unsigned int time;

	SYSTEM_GET_TIME(time);
  //time = current_time;
	shairplay::MD5_Init(&md5ctx);
	shairplay::MD5_Update(&md5ctx, (unsigned char *)&time, sizeof(time));
	shairplay::MD5_Final(md5buf, &md5ctx);
	digest_md5_to_hex(md5buf, md5hex);

	memset(result, 0, resultlen);
	strncpy(result, md5hex, resultlen-1);
#endif
}

int
digest_is_valid(const char *our_realm, const char *password,
                const char *our_nonce, const char *method,
                const char *our_uri, const char *authorization)
{
	char *auth;
	char *current;
	char *value;
	int success;

	/* Get values from authorization */
	char *username = NULL;
	char *realm = NULL;
	char *nonce = NULL;
	char *uri = NULL;
	char *response = NULL;

	/* Buffer for our response */
	char our_response[MD5_SIZE*2+1];

	if (!authorization) {
		return 0;
	}
	current = auth = strdup(authorization);
	if (!auth) {
		return 0;
	}

	/* Check that the type is digest */
	if (strncmp("Digest", current, 6)) {
		free(auth);
		return 0;
	}
	current += 6;

	while ((value = utils_strsep(&current, ",")) != NULL) {
		char *first, *last;

		/* Find first and last characters */
		first = value;
		last = value+strlen(value)-1;

		/* Trim spaces from the value */
		while (*first == ' ' && first < last) first++;
		while (*last == ' ' && last > first) last--;

		/* Validate the last character */
		if (*last != '"') continue;
		else *last = '\0';

		/* Store value if it is relevant */
		if (!strncmp("username=\"", first, 10))
			username = first+10;
		if (!strncmp("realm=\"", first, 7))
			realm = first+7;
		if (!strncmp("nonce=\"", first, 7))
			nonce = first+7;
		if (!strncmp("uri=\"", first, 5))
			uri = first+5;
		if (!strncmp("response=\"", first, 10))
			response = first+10;
	}

	if (!username || !realm || !nonce || !uri || !response) {
		return 0;
	}
	if (strcmp(realm, our_realm) || strcmp(nonce, our_nonce) || strcmp(uri, our_uri)) {
		return 0;
	}

	/* Calculate our response */
	memset(our_response, 0, sizeof(our_response));
	digest_get_response(username, realm, password, nonce,
	                    method, uri, our_response);
	success = !strcmp(response, our_response);
	free(auth);

	return success;
}


