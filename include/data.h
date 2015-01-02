#ifndef VKE_DATA_DEFINED
#define VKE_DATA_DEFINED

//------------------------------------------------------------------------------
// Dependencies

#include <stdio.h> // FILE
#include <time.h>  // clock_t

#include <alias.h> // bool

//------------------------------------------------------------------------------
// Data structures

/**
 * Data object (source and key files/text)
 */
typedef struct obj {
  char* name;
  bool initialized;
  bool is_file;
  FILE* data;
  unsigned int size;
  unsigned int indx;
  char* buff;
  char* hash;
  char* rev_str;
  char* rev_hash;
  char* final_hash;
} obj;

/**
 * Linked encryption layer (processed argument)
 */
typedef struct layer {
  char* name;
  obj* key;
  struct layer* next;
} layer;

/**
 * Application configuration settings
 */
typedef struct config {
  bool show_help;
  bool dry_run;
  bool quiet;
  unsigned int hash_threshold;
  unsigned int src_indx;
  unsigned int key_indx;
  unsigned int key_length;
  struct layer* keys;
  clock_t start;
} config;

#endif
