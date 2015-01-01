
//------------------------------------------------------------------------------
// Dependencies

#include <stdio.h>   // printf
#include <string.h>  // strlen
#include <stdlib.h>  // malloc, free

//------------------------------------------------------------------------------
// String utilities

char* reverse_string(char* str) {
  char tmp, *src, *dst;
  size_t len;

  if (str != NULL) {
    len = strlen (str);

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
