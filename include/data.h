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
  size_t size;
  size_t indx;
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
  unsigned int indx;
  obj* key;
  struct layer* next;
} layer;

/**
 * Application configuration settings
 */
typedef struct config {
  bool show_help;
  bool show_version;
  bool dry_run;
  bool quiet;
  unsigned int hash_threshold;
  size_t src_indx;
  size_t key_length;
  struct layer* keys;
  clock_t start;
} config;

#endif
