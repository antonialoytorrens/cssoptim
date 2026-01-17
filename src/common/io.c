#include "cssoptim/io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_file(const char *filename, size_t *length) {
  FILE *f = fopen(filename, "rb");
  if (!f) return NULL;
  
  if (fseek(f, 0, SEEK_END) != 0) {
    fclose(f);
    return NULL;
  }
  long size = ftell(f);
  if (size < 0) {
    fclose(f);
    return NULL;
  }
  rewind(f);
  
  char *buf = malloc((size_t)size + 1);
  if (!buf) {
    fclose(f);
    return NULL;
  }
  
  size_t read_bytes = fread(buf, 1, (size_t)size, f);
  if (read_bytes < (size_t)size && ferror(f)) {
    free(buf);
    fclose(f);
    return NULL;
  }
  
  buf[read_bytes] = '\0';
  if (length) *length = read_bytes;
  
  fclose(f);
  return buf;
}

bool write_file(const char *filename, const char *content) {
  FILE *f = fopen(filename, "wb");
  if (!f) return false;
  
  size_t len = strlen(content);
  if (fwrite(content, 1, len, f) < len) {
    fclose(f);
    return false;
  }
  
  fclose(f);
  return true;
}
