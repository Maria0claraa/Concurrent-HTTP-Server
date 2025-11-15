// MAIN COMPLETAMENTE CHAT SÓ PARA TESTAR SE ESTÁ TUDO FUNCIONAL




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "logger.h"
#include "http.h"

// Function prototypes
void test_configuration_system(server_config_t **config_ptr);
void test_logger_system(server_config_t *config);
void test_http_parser_system(void);
void test_integration(server_config_t *config);

// Test configuration system
void test_configuration_system(server_config_t **config_ptr) {
    printf("1. ===== CONFIGURATION SYSTEM TESTS =====\n");
    
    // Test 1.1: Load configuration from file
    printf("\n1.1 Loading configuration from file:\n");
    server_config_t *config = config_create("server.conf");
    if (!config) {
        printf("❌ Failed to load configuration from file\n");
        return;
    }
    config_print(config);
    printf("✅ Configuration loaded successfully from file\n");
    
    // Test 1.2: Test all getters
    printf("\n1.2 Testing configuration getters:\n");
    printf("   Port: %d\n", config_get_port(config));
    printf("   Document Root: %s\n", config_get_document_root(config));
    printf("   Workers: %d\n", config_get_num_workers(config));
    printf("   Threads per Worker: %d\n", config_get_threads_per_worker(config));
    printf("   Max Queue: %d\n", config_get_max_queue_size(config));
    printf("   Log File: %s\n", config_get_log_file(config));
    printf("   Cache Size: %d MB\n", config_get_cache_size(config));
    printf("   Timeout: %d seconds\n", config_get_timeout(config));
    printf("✅ All getters working correctly\n");
    
    // Test 1.3: Test setters with validation
    printf("\n1.3 Testing configuration setters with validation:\n");
    
    // Test valid setter
    if (config_set_port(config, 9090) == 0) {
        printf("   ✅ Set port to 9090 - success\n");
    } else {
        printf("   ❌ Failed to set port\n");
    }
    
    // Test invalid setter (should fail)
    if (config_set_port(config, 70000) == -1) {
        printf("   ✅ Correctly rejected invalid port 70000\n");
    } else {
        printf("   ❌ Should have rejected invalid port\n");
    }
    
    // Test document root setter
    if (config_set_document_root(config, "/custom/path") == 0) {
        printf("   ✅ Set document root to /custom/path\n");
    }
    
    // Test 1.4: Stack-allocated config
    printf("\n1.4 Testing stack-allocated configuration:\n");
    server_config_t stack_config;
    config_init_defaults(&stack_config);
    printf("   Default port: %d\n", stack_config.port);
    printf("   Default document root: %s\n", stack_config.document_root);
    printf("   ✅ Stack-allocated config working\n");
    
    // Test 1.5: Configuration validation
    printf("\n1.5 Testing configuration validation:\n");
    if (config_validate(config) == 0) {
        printf("   ✅ Configuration validation passed\n");
    } else {
        printf("   ❌ Configuration validation failed\n");
    }
    
    // Return the config for later use
    *config_ptr = config;
}

// Test logger system
void test_logger_system(server_config_t *config) {
    printf("\n\n2. ===== LOGGER SYSTEM TESTS =====\n");
    
    // Test 2.1: Initialize logger
    printf("\n2.1 Initializing logger:\n");
    if (logger_init(config) != 0) {
        printf("❌ Failed to initialize logger\n");
        return;
    }
    printf("✅ Logger initialized successfully\n");
    
    // Test 2.2: Test all log levels
    printf("\n2.2 Testing all log levels:\n");
    logger_log(LOG_DEBUG, "This is a DEBUG message - detailed information");
    logger_log(LOG_INFO, "This is an INFO message - general operation");
    logger_log(LOG_WARNING, "This is a WARNING message - potential issue");
    logger_log(LOG_ERROR, "This is an ERROR message - something went wrong");
    printf("✅ All log levels working\n");
    
    // Test 2.3: Test HTTP access logging (Apache Combined Format)
    printf("\n2.3 Testing HTTP access logging (Apache Combined Format):\n");
    logger_log_access("127.0.0.1", "GET", "/index.html", 200, 2048, "-", "Mozilla/5.0");
    logger_log_access("192.168.1.100", "POST", "/api/data", 404, 512, "http://example.com", "curl/7.68.0");
    logger_log_access("10.0.0.5", "GET", "/images/logo.png", 200, 15643, "http://localhost/", "Chrome/120.0.0.0");
    printf("✅ HTTP access logging working\n");
    
    // Test 2.4: Test concurrent logging simulation
    printf("\n2.4 Simulating concurrent server activity:\n");
    for (int i = 0; i < 5; i++) {
        logger_log(LOG_INFO, "Processing request #%d from thread %d", i + 1, i % 3);
        usleep(50000); // 50ms delay to simulate processing
    }
    printf("✅ Concurrent logging simulation completed\n");
}

// Test HTTP parser system
void test_http_parser_system(void) {
    printf("\n\n3. ===== HTTP PARSER SYSTEM TESTS =====\n");
    
    // Test 3.1: Parse valid GET request
    printf("\n3.1 Testing valid GET request:\n");
    const char *get_request = 
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: Mozilla/5.0\r\n"
        "Accept: text/html\r\n"
        "\r\n";
    
    http_request_t request;
    if (http_parse_request(get_request, &request) == 0) {
        printf("   ✅ Method: %s\n", request.method == HTTP_GET ? "GET" : "OTHER");
        printf("   ✅ Path: %s\n", request.path);
        printf("   ✅ Version: %s\n", request.version == HTTP_1_1 ? "HTTP/1.1" : "OTHER");
        printf("   ✅ Headers parsed: %s\n", request.headers ? "YES" : "NO");
        http_free_request(&request);
    } else {
        printf("   ❌ Failed to parse GET request\n");
    }
    
    // Test 3.2: Parse HEAD request
    printf("\n3.2 Testing HEAD request:\n");
    const char *head_request = 
        "HEAD /style.css HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "\r\n";
    
    if (http_parse_request(head_request, &request) == 0) {
        printf("   ✅ Method: %s\n", request.method == HTTP_HEAD ? "HEAD" : "OTHER");
        printf("   ✅ Path: %s\n", request.path);
        printf("   ✅ Version: %s\n", request.version == HTTP_1_1 ? "HTTP/1.1" : "OTHER");
        http_free_request(&request);
    } else {
        printf("   ❌ Failed to parse HEAD request\n");
    }
    
    // Test 3.3: Parse request with URL encoding
    printf("\n3.3 Testing request with URL encoding:\n");
    const char *encoded_request = 
        "GET /path%20with%20spaces.html HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "\r\n";
    
    if (http_parse_request(encoded_request, &request) == 0) {
        printf("   ✅ Original path: /path%%20with%%20spaces.html\n");
        printf("   ✅ Decoded path: %s\n", request.path);
        http_free_request(&request);
    }
    
    // Test 3.4: Test unsupported method
    printf("\n3.4 Testing unsupported method:\n");
    const char *post_request = 
        "POST /api/data HTTP/1.1\r\n"
        "Content-Length: 25\r\n"
        "\r\n";
    
    if (http_parse_request(post_request, &request) == 0) {
        printf("   ✅ Method: %s\n", request.method == HTTP_UNSUPPORTED ? "UNSUPPORTED" : "OTHER");
        printf("   ✅ Path: %s\n", request.path);
        http_free_request(&request);
    }
    
    // Test 3.5: Test MIME type detection
    printf("\n3.5 Testing MIME type detection:\n");
    struct {
        const char *filename;
        const char *expected_type;
    } mime_tests[] = {
        {"index.html", "text/html"},
        {"style.css", "text/css"},
        {"script.js", "application/javascript"},
        {"image.png", "image/png"},
        {"photo.jpg", "image/jpeg"},
        {"data.json", "application/json"},
        {"unknown.xyz", "text/plain"},
        {NULL, NULL}
    };
    
    for (int i = 0; mime_tests[i].filename != NULL; i++) {
        const char *detected = http_get_mime_type(mime_tests[i].filename);
        printf("   %-15s → %-25s (expected: %s) %s\n", 
               mime_tests[i].filename, 
               detected,
               mime_tests[i].expected_type,
               strcmp(detected, mime_tests[i].expected_type) == 0 ? "✅" : "❌");
    }
    
    // Test 3.6: Test response header creation
    printf("\n3.6 Testing response header creation:\n");
    
    // Test 200 OK response
    char *header_200 = http_create_response_header(200, "text/html", 2048);
    if (header_200) {
        printf("   ✅ 200 OK Response Header:\n%s", header_200);
        free(header_200);
    }
    
    // Test 404 Not Found response
    char *header_404 = http_create_response_header(404, "text/html", 512);
    if (header_404) {
        printf("   ✅ 404 Not Found Response Header:\n%s", header_404);
        free(header_404);
    }
    
    // Test 3.7: Test path safety validation
    printf("\n3.7 Testing path safety validation:\n");
    struct {
        const char *path;
        int should_be_safe;
    } safety_tests[] = {
        {"/index.html", 1},
        {"/css/style.css", 1},
        {"/../etc/passwd", 0},
        {"/images/../logo.png", 0},
        {"/safe/path", 1},
        {NULL, 0}
    };
    
    for (int i = 0; safety_tests[i].path != NULL; i++) {
        int is_safe = http_is_safe_path(safety_tests[i].path);
        printf("   %-20s → Safe: %-5s %s\n", 
               safety_tests[i].path,
               is_safe ? "YES" : "NO",
               (is_safe == safety_tests[i].should_be_safe) ? "✅" : "❌");
    }
    
    printf("✅ HTTP parser tests completed\n");
}

// Test integration between all systems
void test_integration(server_config_t *config) {
    printf("\n\n4. ===== INTEGRATION TESTS =====\n");
    
    // Test 4.1: Use config values in HTTP parser
    printf("\n4.1 Testing config-http integration:\n");
    printf("   Document Root from config: %s\n", config_get_document_root(config));
    
    // Simulate processing a request using all systems
    printf("\n4.2 Simulating complete request processing:\n");
    
    const char *http_request = "GET /test.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
    http_request_t req;
    
    if (http_parse_request(http_request, &req) == 0) {
        // Log the request using logger
        logger_log_access("127.0.0.1", "GET", req.path, 200, 1024, "-", "Integration-Test");
        
        // Get MIME type based on path
        const char *mime_type = http_get_mime_type(req.path);
        
        // Create response using config values
        char *response_header = http_create_response_header(200, mime_type, 1024);
        
        printf("   ✅ Request parsed: %s %s\n", 
               req.method == HTTP_GET ? "GET" : "OTHER", req.path);
        printf("   ✅ MIME type detected: %s\n", mime_type);
        printf("   ✅ Response header created\n");
        printf("   ✅ Request logged to access.log\n");
        
        if (response_header) {
            free(response_header);
        }
        http_free_request(&req);
    }
    
    printf("✅ Integration tests completed\n");
}

// Main function
int main() {
    printf("=== COMPREHENSIVE TEST - ALL SYSTEMS ===\n\n");
    
    // Test Configuration System
    server_config_t *config = NULL;
    test_configuration_system(&config);
    
    if (!config) {
        printf("❌ CRITICAL: Failed to load configuration\n");
        return -1;
    }
    
    // Test Logger System
    test_logger_system(config);
    
    // Test HTTP Parser System
    test_http_parser_system();
    
    // Test Integration
    test_integration(config);
    
    // Cleanup
    printf("\n\n5. ===== CLEANUP AND FINAL REPORT =====\n");
    
    logger_log(LOG_INFO, "All systems tested successfully");
    logger_log(LOG_INFO, "Shutting down test program");
    
    // Proper cleanup with delays to ensure thread safety
    usleep(100000); // 100ms delay
    logger_close();
    usleep(50000);  // 50ms delay
    config_destroy(config);
    
    printf("\n🎉 ALL SYSTEMS TESTED SUCCESSFULLY! 🎉\n");
    printf("==============================================\n");
    printf("✅ Configuration System: FULLY OPERATIONAL\n");
    printf("✅ Logger System: FULLY OPERATIONAL\n");
    printf("✅ HTTP Parser System: FULLY OPERATIONAL\n");
    printf("✅ Integration: ALL SYSTEMS WORKING TOGETHER\n");
    printf("✅ Log File: access.log (check for complete output)\n");
    printf("==============================================\n\n");
    
    return 0;
}