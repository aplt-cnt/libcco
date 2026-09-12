# libcco Developer Quick Start Guide (Quick Start)

Welcome to `libcco`! This is a high-performance, crash-proof, dynamic AST configuration parsing engine written in pure C11. The CCO (Config & Command Object) syntax abandons the `{}` and `[]` found in traditional JSON/HOCON, **wrapping everything in `()`**, and intelligently infers types through key-value pair topology.

This document will quickly get you up to speed on the core integration and development practices of `libcco`.

---

## 1. Quick Integration and Compilation

`libcco` has no third-party dependencies. You can directly add `src/` and `include/` to your build system, or use the included Makefile to build the static library:

```bash
# Generate build/debug/liblibcco.a
make build.debug

# Or generate a release build with -O3 optimization
make build.release
```

In your project, you only need to include the header file and link the static library:

```c
#include <cnt/cco.h>
// Compile: cc your_code.c -I/path/to/libcco/include -L/path/to/libcco/build/debug -llibcco
```

---

## 2. Core C API Usage Examples

The C API of `libcco` is extremely concise and highly memory-safe. All object tree operations are carried by the `cco_object_t` handle.

### 2.1 Basic Parsing and Value Retrieval

CCO supports a **top-level implicit dictionary (Shorthand Map)**, meaning the outermost layer does not need to be wrapped in `()`.

```c
#include <cnt/cco.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    // Pure CCO syntax, using only ()
    const char* cco_text = 
        "server: (\n"
        "    host: \"127.0.0.1\",\n"
        "    port: 8080,\n"
        "    debug: true\n"
        ")\n";

    // 1. Parse the string (passing NULL for opts will use the default secure sandbox configuration)
    cco_object_t* config = cco_parse_string(cco_text, strlen(cco_text), NULL);
    if (!config) {
        printf("Parsing failed! Error code: %d\n", cco_get_last_error());
        return 1;
    }

    // 2. Validate the type and extract the fields
    if (cco_object_get_type(config) == CCO_TYPE_MAP) {
        // Get the 'server' node from the dictionary
        cco_object_t* server_obj = cco_map_get_by_key(config, "server");

        if (server_obj && cco_object_get_type(server_obj) == CCO_TYPE_MAP) {
            // Get host (String)
            cco_object_t* host_obj = cco_map_get_by_key(server_obj, "host");
            const char* host_str; size_t len;
            if (cco_object_get_string(host_obj, &host_str, &len)) {
                printf("Host: %s\n", host_str);
            }

            // Get port (Integer)
            cco_object_t* port_obj = cco_map_get_by_key(server_obj, "port");
            int64_t port;
            if (cco_object_get_integer(port_obj, &port)) {
                printf("Port: %lld\n", (long long)port);
            }
        }
    }

    // 3. Must release the root node (the subtree will be reclaimed along with it)
    cco_object_release(config);
    return 0;
}
```

---

## 3. Explosion-proof Sandbox and Advanced Options

For untrusted external input files, `libcco` provides a robust security sandbox. By passing in the `cco_parse_options_t` struct, you can limit the parser's depth and memory allocation.

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 4096
#define MAX_PATH_LEN 1024
#define MAX_HEADER_SIZE 8192
#define LOG_FILE "server.log"

// HTTP status codes
#define HTTP_200 "200 OK"
#define HTTP_400 "400 Bad Request"
#define HTTP_403 "403 Forbidden"
#define HTTP_404 "404 Not Found"
#define HTTP_405 "405 Method Not Allowed"
#define HTTP_500 "500 Internal Server Error"
#define HTTP_501 "501 Not Implemented"

// MIME types
typedef struct {
    const char *extension;
    const char *mime_type;
} mime_type_t;

mime_type_t mime_types[] = {
    {".html", "text/html"},
    {".htm", "text/html"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".json", "application/json"},
    {".png", "image/png"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".gif", "image/gif"},
    {".ico", "image/x-icon"},
    {".txt", "text/plain"},
    {".pdf", "application/pdf"},
    {".xml", "application/xml"},
    {".svg", "image/svg+xml"},
    {".mp4", "video/mp4"},
    {".mp3", "audio/mpeg"},
    {".zip", "application/zip"},
    {NULL, NULL}
};

// Client information structure
typedef struct {
    int socket;
    struct sockaddr_in address;
    char client_ip[INET_ADDRSTRLEN];
    int port;
} client_info_t;

// Global variables
int server_socket;
char *document_root;
int server_port;
volatile sig_atomic_t running = 1;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function declarations
void signal_handler(int sig);
void *handle_client(void *arg);
void log_request(const char *client_ip, const char *method, const char *path, int status);
const char *get_mime_type(const char *path);
void send_error_response(int client_socket, const char *status, const char *message);
void send_file_response(int client_socket, const char *file_path, const char *mime_type);
int parse_http_request(const char *buffer, char *method, char *path, char *version);
void url_decode(char *dst, const char *src);
int is_safe_path(const char *path);
void get_current_time(char *buffer, size_t size);

// Signal handler
void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        running = 0;
        close(server_socket);
    }
}

// Get current time string
void get_current_time(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

// Log request
void log_request(const char *client_ip, const char *method, const char *path, int status) {
    pthread_mutex_lock(&log_mutex);

    FILE *log_fp = fopen(LOG_FILE, "a");
    if (log_fp) {
        char time_buffer[64];
        get_current_time(time_buffer, sizeof(time_buffer));
        fprintf(log_fp, "[%s] %s \"%s %s\" %d\n",
                time_buffer, client_ip, method, path, status);
        fclose(log_fp);
    }

    pthread_mutex_unlock(&log_mutex);
}

// Get MIME type
const char *get_mime_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";

    for (int i = 0; mime_types[i].extension != NULL; i++) {
        if (strcasecmp(ext, mime_types[i].extension) == 0) {
            return mime_types[i].mime_type;
        }
    }
    return "application/octet-stream";
}

// URL decode
void url_decode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if ((*src == '%') && ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a' - 'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a' - 'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16 * a + b;
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

// Check if path is safe (prevent directory traversal)
int is_safe_path(const char *path) {
    if (strstr(path, "..") != NULL) {
        return 0;
    }
    return 1;
}

// Parse HTTP request
int parse_http_request(const char *buffer, char *method, char *path, char *version) {
    char *line_end = strstr(buffer, "\r\n");
    if (!line_end) return -1;

    char request_line[MAX_HEADER_SIZE];
    size_t line_len = line_end - buffer;
    if (line_len >= sizeof(request_line)) return -1;

    strncpy(request_line, buffer, line_len);
    request_line[line_len] = '\0';

    if (sscanf(request_line, "%15s %1023s %15s", method, path, version) != 3) {
        return -1;
    }

    return 0;
}

// Send error response
void send_error_response(int client_socket, const char *status, const char *message) {
    char response[BUFFER_SIZE];
    char body[512];

    snprintf(body, sizeof(body),
             "<html><head><title>%s</title></head>"
             "<body><h1>%s</h1><p>%s</p></body></html>",
             status, status, message);

    snprintf(response, sizeof(response),
             "HTTP/1.1 %s\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             status, strlen(body), body);

    send(client_socket, response, strlen(response), 0);
}

// Send file response
void send_file_response(int client_socket, const char *file_path, const char *mime_type) {
    struct stat file_stat;
    if (stat(file_path, &file_stat) < 0) {
        send_error_response(client_socket, HTTP_404, "File not found");
        return;
    }

    if (!S_ISREG(file_stat.st_mode)) {
        send_error_response(client_socket, HTTP_403, "Access denied");
        return;
    }

    FILE *fp = fopen(file_path, "rb");
    if (!fp) {
        send_error_response(client_socket, HTTP_403, "Cannot open file");
        return;
    }

    // Send response headers
    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header),
             "HTTP/1.1 %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n"
             "\r\n",
             HTTP_200, mime_type, file_stat.st_size);

    send(client_socket, header, strlen(header), 0);

    // Send file content
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        if (send(client_socket, buffer, bytes_read, 0) < 0) {
            break;
        }
    }

    fclose(fp);
}

// Handle client request
void *handle_client(void *arg) {
    client_info_t *client = (client_info_t *)arg;
    int client_socket = client->socket;
    char buffer[MAX_HEADER_SIZE];
    char method[16], path[MAX_PATH_LEN], version[16];
    char decoded_path[MAX_PATH_LEN];
    char full_path[MAX_PATH_LEN * 2];

    // Receive request
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        close(client_socket);
        free(client);
        pthread_exit(NULL);
    }
    buffer[bytes_received] = '\0';

    // Parse request
    if (parse_http_request(buffer, method, path, version) < 0) {
        send_error_response(client_socket, HTTP_400, "Bad Request");
        log_request(client->client_ip, "UNKNOWN", "UNKNOWN", 400);
        close(client_socket);
        free(client);
        pthread_exit(NULL);
    }

    // Only support GET method
    if (strcmp(method, "GET") != 0) {
        send_error_response(client_socket, HTTP_501, "Method Not Implemented");
        log_request(client->client_ip, method, path, 501);
        close(client_socket);
        free(client);
        pthread_exit(NULL);
    }

    // URL decode
    url_decode(decoded_path, path);

    // Security check
    if (!is_safe_path(decoded_path)) {
        send_error_response(client_socket, HTTP_403, "Forbidden");
        log_request(client->client_ip, method, path, 403);
        close(client_socket);
        free(client);
        pthread_exit(NULL);
    }

    // Build full path
    if (strcmp(decoded_path, "/") == 0) {
        snprintf(full_path, sizeof(full_path), "%s/index.html", document_root);
    } else {
        snprintf(full_path, sizeof(full_path), "%s%s", document_root, decoded_path);
    }

    // Get MIME type
    const char *mime_type = get_mime_type(full_path);

    // Send file
    struct stat file_stat;
    if (stat(full_path, &file_stat) < 0) {
        send_error_response(client_socket, HTTP_404, "Not Found");
        log_request(client->client_ip, method, path, 404);
    } else {
        send_file_response(client_socket, full_path, mime_type);
        log_request(client->client_ip, method, path, 200);
    }

    close(client_socket);
    free(client);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <document_root>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    server_port = atoi(argv[1]);
    document_root = argv[2];

    if (server_port <= 0 || server_port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }

    // Check if document root exists
    struct stat st;
    if (stat(document_root, &st) < 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Document root does not exist or is not a directory\n");
        exit(EXIT_FAILURE);
    }

    // Set signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, SIG_IGN);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("HTTP Server started on port %d\n", server_port);
    printf("Document root: %s\n", document_root);
    printf("Press Ctrl+C to stop\n");

    // Main loop
    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            if (running) {
                perror("accept");
            }
            continue;
        }

        // Create client info
        client_info_t *client = malloc(sizeof(client_info_t));
        if (!client) {
            close(client_socket);
            continue;
        }

        client->socket = client_socket;
        client->address = client_addr;
        inet_ntop(AF_INET, &client_addr.sin_addr, client->client_ip, INET_ADDRSTRLEN);
        client->port = ntohs(client_addr.sin_port);

        // Create thread to handle client
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, client) != 0) {
            perror("pthread_create");
            close(client_socket);
            free(client);
            continue;
        }

        pthread_detach(thread);
    }

    close(server_socket);
    printf("\nServer stopped\n");

    return 0;
}
```

```c
cco_parse_options_t opts;
opts.max_depth = 10;                     // Limit the maximum nesting depth (to prevent stack overflow)
opts.max_document_size = 1024 * 1024;    // Limit the maximum file size to 1MB
opts.max_total_allocation = 5 * 1024 * 1024; // Limit the maximum memory usage to 5MB
opts.lenient_brackets = true;            // Enable lenient mode (automatic error correction)

cco_object_t* config = cco_parse_string(text, len, &opts);
```

### 🎯 Lenient Error Correction Mode (Lenient Brackets)

When `lenient_brackets = true` is enabled (which is the default), if a user habitually types JSON-style `{}` or `[]`, the engine **will not crash or reject it**, but will intelligently downgrade and correct it to a valid `()` topology, only logging a warning in the diagnostic log, greatly improving the application's tolerance for novice users.

---

## 4. Traversal of Arrays and Dictionaries (Getters)

For dynamically sized collections, use the traversal interface:

**Iterating Over an Array:**

```c
size_t count = cco_array_get_count(arr_obj);
for (size_t i = 0; i < count; i++) {
    cco_object_t* item = cco_array_get_item(arr_obj, i);
    // Process item...
}
```

**Iterating over a Map:**

```c
size_t count = cco_map_get_count(map_obj);
for (size_t i = 0; i < count; i++) {
    const char* key = cco_map_get_key(map_obj, i);
    cco_object_t* val = cco_map_get_value(map_obj, i);
    // Process key and val...
}
```

---

## 5. Modern C++ Wrapper (C++ RAII Wrapper)

If you are developing in C++, you can directly include `<cnt/cco.hpp>`, completely freeing yourself from the mental burden of `cco_object_release`.

```cpp
#include <cnt/cco.hpp>
#include <iostream>
#include <string>

int main() {
    std::string config = R"(
app: (
version: 1.0,
modules: ( "auth", "db" )
)
)";

    try {
        // cnt::cco::Object supports RAII, automatically cleans up memory when out of scope
        cnt::cco::Object obj = cnt::cco::parse(config);

        std::cout << "Parse Type ID: " << obj.type() << "\n";

        // Serialize back to string
        std::cout << "Serialized:\n" << obj.serialize(true) << "\n";
    }
    catch (const cnt::cco::ParseError& e) {
        std::cerr << "Syntax error (Code: " << e.code() << "): " << e.what() << "\n";
    }

    return 0;
}
```

---

## 6. Deeper Exploration

For more combination examples, please go to the project's `examples/` directory:

- `make examples name=tree_print`: Experience beautiful terminal AST tree printing.
- `make examples name=auto_correction`: Experience how the engine saves non-standard `{}` writing through lenient mode.