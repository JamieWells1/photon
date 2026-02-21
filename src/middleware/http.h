#ifndef HTTP_H
#define HTTP_H

#include <stdbool.h>
#include <stddef.h>

typedef void (*http_response_callback)(const char* body, size_t len, bool complete);

int http_get(const char* host_url, const char* url_sub_path, http_response_callback callback);

#endif  // HTTP_H
