#ifndef HTTP_H
#define HTTP_H

#include <matrix.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef void (*http_response_callback_t)(const char* body, size_t len, bool complete);

int http_get(const char* host_url, uint16_t port, const char* url_sub_path,
             http_response_callback_t callback, Matrix* mtrx);

#endif  // HTTP_H
