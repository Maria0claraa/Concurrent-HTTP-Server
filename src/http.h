// Protótipos HTTP



#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// HTTP methods supported
typedef enum {
    HTTP_GET,
    HTTP_HEAD,
    HTTP_UNSUPPORTED
} http_method_t;

// HTTP version
typedef enum {
    HTTP_1_0,
    HTTP_1_1,
    HTTP_UNKNOWN
} http_version_t;

// HTTP request structure
typedef struct {
    http_method_t method;      // GET, HEAD, etc.
    char path[1024];           // Requested path (/index.html)
    http_version_t version;    // HTTP/1.0 or HTTP/1.1
    char *headers;             // Raw headers (for future extension)
    size_t content_length;     // Content length for POST
} http_request_t;

// HTTP response structure
typedef struct {
    int status_code;           // 200, 404, etc.
    char *content_type;        // text/html, image/png, etc.
    size_t content_length;     // Response body size
    char *body;                // Response body (for dynamic content)
} http_response_t;

// ===== HTTP PARSER API =====

// Parse HTTP request from buffer
int http_parse_request(const char *buffer, http_request_t *request);

// Free resources allocated during parsing
void http_free_request(http_request_t *request);

// Get MIME type from file extension
const char* http_get_mime_type(const char *filename);

// Create HTTP response header
char* http_create_response_header(int status_code, const char *content_type, size_t content_length);

// Get status message for status code
const char* http_status_message(int status_code);

// URL decode function
void http_url_decode(char *dst, const char *src);

// Check if path is safe (no directory traversal)
int http_is_safe_path(const char *path);

#endif 