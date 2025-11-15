// Processamento HTTP

// analisa pedidios HTTP recebidos
// extrai o método, caminho e versão HTTP
// determina MIME type de todos os ficheiros
// cria os cabeçalhos de resposta HTTP
// valida os paths 

#include "http.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
// Parse HTTP request from buffer
int http_parse_request(const char *buffer, http_request_t *request) {
    if (!buffer || !request) {
        return -1;
    }

    // Initialize request structure
    memset(request, 0, sizeof(http_request_t));
    request->method = HTTP_UNSUPPORTED;
    request->version = HTTP_UNKNOWN;

    // Copy buffer for parsing (strtok modifies the string)
    char *copy = strdup(buffer);
    if (!copy) {
        return -1;
    }

    char *line = strtok(copy, "\r\n");
    if (!line) {
        free(copy);
        return -1;
    }

    // Parse request line: "METHOD PATH VERSION"
    char method[16], path[1024], version[16];
    if (sscanf(line, "%15s %1023s %15s", method, path, version) != 3) {
        free(copy);
        return -1;
    }

    // Parse HTTP method
    if (strcmp(method, "GET") == 0) {
        request->method = HTTP_GET;
    } else if (strcmp(method, "HEAD") == 0) {
        request->method = HTTP_HEAD;
    } else {
        request->method = HTTP_UNSUPPORTED;
    }

    // Parse path (URL decode if needed)
    strncpy(request->path, path, sizeof(request->path) - 1);
    request->path[sizeof(request->path) - 1] = '\0';

    // Simple URL decode for spaces
    http_url_decode(request->path, request->path);

    // Parse HTTP version
    if (strcmp(version, "HTTP/1.0") == 0) {
        request->version = HTTP_1_0;
    } else if (strcmp(version, "HTTP/1.1") == 0) {
        request->version = HTTP_1_1;
    } else {
        request->version = HTTP_UNKNOWN;
    }

    // Parse headers (simple version - just store them)
    request->headers = strdup(buffer + strlen(line) + 2); // Skip request line + CRLF

    free(copy);
    return 0;
}

// Free resources allocated during parsing
void http_free_request(http_request_t *request) {
    if (request && request->headers) {
        free(request->headers);
        request->headers = NULL;
    }
}

// Get MIME type from file extension
const char* http_get_mime_type(const char *filename) {
    if (!filename) return "application/octet-stream";

    const char *ext = strrchr(filename, '.');
    if (!ext) return "text/plain";

    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) {
        return "text/html";
    } else if (strcmp(ext, ".css") == 0) {
        return "text/css";
    } else if (strcmp(ext, ".js") == 0) {
        return "application/javascript";
    } else if (strcmp(ext, ".png") == 0) {
        return "image/png";
    } else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
        return "image/jpeg";
    } else if (strcmp(ext, ".gif") == 0) {
        return "image/gif";
    } else if (strcmp(ext, ".svg") == 0) {
        return "image/svg+xml";
    } else if (strcmp(ext, ".json") == 0) {
        return "application/json";
    } else {
        return "text/plain";
    }
}

// Create HTTP response header
char* http_create_response_header(int status_code, const char *content_type, size_t content_length) {
    const char *status_msg = http_status_message(status_code);
    
    // Calculate approximate size needed
    size_t header_size = 256 + (content_type ? strlen(content_type) : 0);
    char *header = malloc(header_size);
    if (!header) return NULL;

    // Get current time for Date header
    time_t now = time(NULL);
    struct tm *gm = gmtime(&now);
    char date[64];
    strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", gm);

    // Build response header
    snprintf(header, header_size,
        "HTTP/1.1 %d %s\r\n"
        "Server: Concurrent-HTTP-Server\r\n"
        "Date: %s\r\n"
        "Connection: close\r\n",
        status_code, status_msg, date);

    if (content_type) {
        char temp[512];
        snprintf(temp, sizeof(temp), "Content-Type: %s\r\n", content_type);
        strcat(header, temp);
    }

    if (content_length > 0) {
        char temp[128];
        snprintf(temp, sizeof(temp), "Content-Length: %zu\r\n", content_length);
        strcat(header, temp);
    }

    strcat(header, "\r\n");  // End of headers
    return header;
}

// Get status message for status code
const char* http_status_message(int status_code) {
    switch (status_code) {
        case 200: return "OK";
        case 404: return "Not Found";
        case 403: return "Forbidden";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 400: return "Bad Request";
        default: return "Unknown";
    }
}

// Simple URL decode function (handles %20 for spaces)
void http_url_decode(char *dst, const char *src) {
    if (!dst || !src) return;
    
    char *p = dst;
    while (*src) {
        if (*src == '%' && src[1] && src[2]) {
            // Simple %20 decoding (spaces)
            if (src[1] == '2' && src[2] == '0') {
                *p++ = ' ';
                src += 3;
            } else {
                *p++ = *src++;
            }
        } else if (*src == '+') {
            *p++ = ' ';
            src++;
        } else {
            *p++ = *src++;
        }
    }
    *p = '\0';
}

// Check if path is safe (no directory traversal)
int http_is_safe_path(const char *path) {
    if (!path) return 0;
    
    // Prevent directory traversal
    if (strstr(path, "..")) {
        return 0;
    }
    
    // Prevent absolute paths
    if (path[0] == '/') {
        return 1; // This is OK for our document root
    }
    
    return 1;
}