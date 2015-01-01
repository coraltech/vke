
//------------------------------------------------------------------------------
// Dependencies

#include <string.h>  // strlen

#include <data.h>    // obj
#include <alias.h>   // bool, true, false

//------------------------------------------------------------------------------
// String utilities

char* reverse_string(char* str) {
  char tmp, *src, *dst;
  size_t len;

  if (str != NULL) {
    len = strlen(str);

    if (len > 1) {
      src = str;
      dst = src + len - 1;

      while (src < dst) {
        tmp    = *src;
        *src++ = *dst;
        *dst-- = tmp;
      }
    }
  }
  return str;
}

//------------------------------------------------------------------------------
// Encryption related utilities

bool fill_key_buffer(obj* key) {
  if (!key->is_file) {
    int key_buff_indx = (int)strlen(key->buff);

    int select_indx   = 0;

    while (key_buff_indx < (buff_size - 1)) {
      key->buff[key_buff_indx] = key->buff[select_indx];
      key->buff[(key_buff_indx + 1)] = '\0';
      key_buff_indx++;
      select_indx++;
    }
    key->size = strlen(key->buff);
  }
  return true;
}
