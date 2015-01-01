
//------------------------------------------------------------------------------
// Dependencies

#include <stdio.h>   // FILE, stderr, sprintf, printf, fopen
#include <stdlib.h>  // calloc, free
#include <string.h>  // strlen, strcpy
#include <unistd.h>  // getpass
#include <time.h>    // CLOCKS_PER_SEC, clock_t, clock

#include <alias.h>   // buff_size, bool, true, false
#include <data.h>    // config, obj, layer
#include <hash.h>    // get_hash
#include <utility.h> // reverse_string, fill_key_buffer
#include <vke.h>

//------------------------------------------------------------------------------
// Initialization

/**
 * Initialize a source file and encryption keys
 */
bool initialize(config* cfg, obj* info, char* name, int indx,
    const char* access, bool force_file) {
  info->name    = name;
  info->is_file = true;

  int msec = ((clock_t)(clock() - cfg->start) * 1000 / CLOCKS_PER_SEC);
  printf("Initializing: %s [ %i ] (%dsec & %dms)\n", name, indx, msec / 1000, msec % 1000);

  if (!(info->buff = (char*) calloc(buff_size, 1))) {
    printf("Unable to buffer %s\n", info->name);
    return false;
  }
  if (!(info->data = fopen(info->name, access))) {
    if (force_file) {
      printf("Unable to open %s\n", info->name);
      free(info->buff);
      return false;
    } else {
      strcpy(info->buff, "");
      strcpy(info->buff, name);

      if (strcmp(info->buff, "prompt") == 0) {
        char pass_prompt[80];
        char pass_input[500];
        sprintf(pass_prompt, "Enter passphrase for key %i: ", indx);
        strcpy(pass_input, getpass(pass_prompt));

        char confirm_prompt[80];
        char confirm_input[500];
        sprintf(confirm_prompt, "Confirm passphrase for key %i: ", indx);
        strcpy(confirm_input, getpass(confirm_prompt));

        if (strcmp(pass_input, confirm_input) == 0) {
          info->buff = pass_input;
        } else {
          printf("Passphrase and confirmation for key %i do not match\n\n",
              indx);
          return false;
        }
      }

      info->size = strlen(info->buff);
      info->indx = 0;
      info->is_file = false;

      if (info->size < 200) {
        info->hash    = get_hash(info->buff);
        info->rev_str = (char*)malloc(strlen(info->buff) * sizeof(char));
        strcpy(info->rev_str, info->buff);

        info->rev_str  = reverse_string(info->rev_str);
        info->rev_hash = get_hash(info->rev_str);

        strcat(info->hash, info->rev_hash);
        strcpy(info->buff, "");
        strcpy(info->buff, info->hash);

        info->size = strlen(info->buff);
      }
    }
  } else {
    fseek(info->data, 0, SEEK_END);
    info->size = ftell(info->data);

    fseek(info->data, 0, SEEK_SET);
    info->indx = 0;
  }
  return true;
}

//------------------------------------------------------------------------------
// Checks and verification

/**
 * Run sanity checks on the source file and encryption keys
 */
bool check(config* cfg, obj* src, obj* key) {
  int src_read;
  int key_read;

  int indx;

  int msec = ((clock_t)(clock() - cfg->start) * 1000 / CLOCKS_PER_SEC);
  printf("Verifying success of key %s [ %i ] (%dsec & %dms)\n", key->name, key->size, msec / 1000, msec % 1000);

  fseek(src->data, 0, SEEK_SET);
  src->indx = 0;

  if (fill_key_buffer(key)) {
    while (src->indx < src->size) {
      if ((src_read = fread(src->buff, 1, buff_size, src->data)) < 1) {
        printf("Unable to read from %s\n", src->name);
        return false;
      }
      if (key->is_file) {
        if ((key_read = fread(key->buff, 1, buff_size, key->data)) < buff_size) {
          if (key_read < 1) {
            printf("Unable to read from %s\n", key->name);
            return false;
          }
          fseek(key->data, 0, SEEK_SET);
          key->indx = 0;
        }
      } else {
        key_read = key->size;
      }

      indx = 0;
      while ((indx < src_read) && (indx < key_read)) {
        src->buff[indx] = src->buff[indx] ^ key->buff[indx];
        indx++;
        src->indx++;
        key->indx++;
      }

      fseek(src->data, (-1 * src_read), SEEK_CUR);
      fseek(src->data, indx, SEEK_CUR);
    }
    return true;
  }
  return false;
}

//------------------------------------------------------------------------------
// Encryption / Decryption

/**
 * Combine the source file with an encryption key
 */
bool combine(config* cfg, obj* src, obj* key, FILE* output_stream) {
  int src_read;
  int key_read;

  int indx;

  int msec = ((clock_t)(clock() - cfg->start) * 1000 / CLOCKS_PER_SEC);
  printf("Combining source %s with key %s (%dsec & %dms)\n", src->name, key->name, msec / 1000, msec % 1000);

  fseek(src->data, 0, SEEK_SET);
  src->indx = 0;

  if (key->is_file) {
    fseek(key->data, 0, SEEK_SET);
    key->indx = 0;
  }

  while (src->indx < src->size) {
    if ((src_read = fread(src->buff, 1, buff_size, src->data)) < 1) {
      printf("Unable to read from %s\n", src->name);
      return false;
    }
    if (key->is_file) {
      if ((key_read = fread(key->buff, 1, buff_size, key->data)) < buff_size) {
        if (key_read < 1) {
          printf("Unable to read from %s\n", key->name);
          return false;
        }
        fseek(key->data, 0, SEEK_SET);
        key->indx = 0;
      }
    } else {
      key_read = key->size;
    }

    indx = 0;
    while ((indx < src_read) && (indx < key_read)) {
      src->buff[indx] = src->buff[indx] ^ key->buff[indx];
      indx++;
      src->indx++;
      key->indx++;
    }

    fseek(src->data, (-1 * src_read), SEEK_CUR);

    if (fwrite(src->buff, 1, indx, output_stream) < indx) {
      printf("Unable to write %s\n", src->name);
      return false;
    }
    fflush(output_stream);
  }
  return true;
}

//------------------------------------------------------------------------------
// Cleanup

/**
 * Finalize source or key objects and cleanup
 */
bool finalize(config* cfg, obj* src) {
  int errors = 0;

  int msec = ((clock_t)(clock() - cfg->start) * 1000 / CLOCKS_PER_SEC);
  printf("Finalizing session for source %s (%dsec & %dms)\n", src->name, msec / 1000, msec % 1000);

  if (src->is_file) {
    fclose(src->data);
  }

  layer* temp = cfg->keys;
  do {
    if (!finalize_key(cfg, temp->key)) {
      errors++;
    }
  } while ((temp = temp->next) != NULL);

  return ((errors == 0) ? true : false);
}

/**
 * Finalize key object and cleanup
 */
bool finalize_key(config* cfg, obj* key) {

  int msec = ((clock_t)(clock() - cfg->start) * 1000 / CLOCKS_PER_SEC);
  printf("Finalizing session for key %s (%dsec & %dms)\n", key->name, msec / 1000, msec % 1000);

  if (key->is_file) {
    fclose(key->data);
  }
  return true;
}
