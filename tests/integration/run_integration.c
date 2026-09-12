#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cnt/cco.h>

char* read_file(const char* path, size_t* out_len) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* buf = malloc(len + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    
    size_t read_bytes = fread(buf, 1, len, f);
    buf[read_bytes] = '\0';
    if (out_len) *out_len = read_bytes;
    
    fclose(f);
    return buf;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.cco>\n", argv[0]);
        return -1;
    }
    
    const char* path = argv[1];
    size_t len = 0;
    char* src = read_file(path, &len);
    if (!src) {
        fprintf(stderr, "Failed to read %s\n", path);
        return -1;
    }
    
    cco_object_t* obj = cco_parse_string(src, len, NULL);
    free(src);
    
    if (obj) {
        cco_object_release(obj);
        return 0; // Success
    } else {
        return cco_get_last_error(); // Failure
    }
}
