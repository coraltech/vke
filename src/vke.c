/**
 ******************************************************************************
 ***                                                                        ***
 *                  VKE   -   Variable Key Encryption                         *
 ***                                                                        ***
 ******************************************************************************
 */
//-----------------------------------------------------------------------------
// Dependencies

#include <stdio.h>  // FILE, stdout, sprintf, printf, fopen
#include <stdlib.h> // calloc, malloc, free
#include <string.h> // strlen, strcpy
#include <unistd.h> // getpass

//-----------------------------------------------------------------------------
// Aliases

#define buff_size 102400
#define true 1
#define false 0

typedef unsigned int bool;

//-----------------------------------------------------------------------------
// Data structures

/**
 * Data object (source and key files/text)
 */
typedef struct obj
{
  char* name;
  bool  is_file;
  FILE* data;
  unsigned int size;
  unsigned int indx;
  char* buff;
}
obj;

/**
 * Linked encryption layer (processed argument)
 */
typedef struct layer
{
  char*  name;
  obj*   key;
  struct layer* next;
}
layer;

/**
 * Application configuration settings
 */
typedef struct config
{
  bool show_help;
  bool dry_run;
  unsigned int src_indx;
  unsigned int key_indx;
  struct layer* keys;
}
config;

//-----------------------------------------------------------------------------
// Function prototypes

void process_args(config* cfg, int argc, char* argv[]);

bool initialize(config* cfg, obj* info, char* name, int indx, const char* access, bool force_file);
bool check(config* cfg, obj* src, obj* key);
bool combine(config* cfg, obj* src, obj* key, FILE* output_stream);
bool finalize(config* cfg, obj* src);
bool finalize_key(config* cfg, obj* key);

bool add_layer(config* cfg, char* name);
bool free_layers(config* cfg);

//-----------------------------------------------------------------------------
// Execution gateway

int main(int argc, char* argv[]) {
  config cfg;
	obj    src;
	
  char *help[] = {
    "                                                                                   ",
    " Usage: vke  [-hd]  <source.file>  <key.file | key text | 'prompt'> ...            ",
    "                                                                                   ",
    "          -h | --help     Display this help information                            ",
    "          -d | --dry_run  Test encryption / decryption without editing source file ",
    "                                                                                   ",
    "-----------------------------------------------------------------------------------",
    "                                                                                   ",
    "                        VKE - Variable Key Encryption                              ",
    "                                                                                   ",
    " Simple utility for encrypting source files based on combinations of file          ",
    " based, parameter, and prompted passphrases.                                       ",
    "                                                                                   ",
    " Typing the word <prompt> will cause the program to prompt you for a               ",
    " passphrase before encryption and decryption can begin.  Multiple prompted         ",
    " passphrases may be used by specifying <prompt> multiple times.                    ",
    "                                                                                   ",
    " The same keys must be used to encrypt and decrypt files but the ordering          ", 
    " of the keys has no effect on the result.                                          ",
    "                                                                                   ", 
    " This utility performs destructive operations on the source file and may in        ", 
    " extreme cirumstances cause data loss.  Always have backups handy.                 ",
    "                                                                                   ",
    "     Author: Adrian Webb (adrian.webb@coraltech.net)                               ",
    "                                                                                   "
  };

  process_args(&cfg, argc, argv);


  if (argc > 1 && (!initialize(&cfg, &src, argv[cfg.src_indx], 0, "rb+", true))) {
    cfg.show_help = true;
  }
	if (cfg.show_help) {
		size_t i;
    for (i = 0; i < sizeof(help) / sizeof(*help); i++) {
      printf("%s\n", help[i]);
    }
    exit(1);
	}

	
  FILE* output_stream = ((cfg.dry_run) ? stderr : src.data);
  int   key_indx      = cfg.key_indx;
  int   errors        = 0;

  if (cfg.keys != NULL) {
    // First pass - Verify to minimize the chances of screwing up our file.
    layer* temp = cfg.keys;
    do {
      temp->key = (struct obj*)malloc(sizeof(struct obj));

      if (temp->key == NULL) {
        printf("Cannot create memory for key %s", temp->name);
        errors++;
      }
      else if(initialize(&cfg, temp->key, temp->name, (key_indx - 1), "rb", false)) {
        if(!check(&cfg, &src, temp->key)) {
          errors++;
        }
      }
      else {
        errors++;
      }
      key_indx++;
    } 
    while ((temp = temp->next) != NULL);

    // Second pass - Combine source and keys to toggle encryption / decryption.
    if (errors == 0) {
      temp = cfg.keys;
      do {
        if (!combine(&cfg, &src, temp->key, output_stream)) {
          errors++;
        }
      } 
      while ((temp = temp->next) != NULL);
    }
  }

	if (!finalize(&cfg, &src)) {
    errors++;
  }
  if (!free_layers(&cfg)) {
    errors++;
  }
  
  printf("\n\n");

  if (errors > 0) {
    exit((errors + 1));
  }  
  exit(EXIT_SUCCESS);
}

//-----------------------------------------------------------------------------
// Argument processing

/**
 * Process CLI arguments
 */
void process_args(config* cfg, int argc, char* argv[]) {
  cfg->show_help = false;
  cfg->dry_run   = false;
  cfg->src_indx  = 1;
  cfg->key_indx  = 2;
  cfg->keys      = NULL;

  int arg_indx = 1;
  
  printf("Parsing arguments\n");

  while(arg_indx < argc) {
    char* arg = argv[arg_indx];

    if ((strcmp(arg, "-hd") == 0) || (strcmp(arg, "-dh") == 0)) {
      cfg->show_help = true;
    }
    else if ((strcmp(arg, "-h") == 0) || (strcmp(arg, "--help") == 0)) {
      cfg->show_help = true;
    }
    else if ((strcmp(arg, "-d") == 0) || (strcmp(arg, "--dry_run") == 0)) {
      cfg->dry_run = true;
    }
    else {
      add_layer(cfg, arg);
    }
    arg_indx++;
  }

  if (argc == 1) {
    cfg->show_help = true;
  }

  layer* src_layer = cfg->keys;
  cfg->keys        = cfg->keys->next;
  free(src_layer);
}

//-----------------------------------------------------------------------------
// Encryption / Decryption processing

/**
 * Initialize a source file and encryption keys
 */
bool initialize(config* cfg, obj* info, char* name, int indx, const char* access, bool force_file) {
	info->name    = name;
  info->is_file = true;
  
  printf("Initializing: %s [ %i ]\n", name, indx);

	if(!(info->buff = (char*)calloc(buff_size, 1)))	{
		printf("Unable to buffer %s\n", info->name);
		return false;
	}
	if(!(info->data = fopen(info->name, access)))	{
		if (force_file) {
      printf("Unable to open %s\n", info->name);
      free(info->buff);
      return false;
    }
    else {
      info->buff = name;

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
        }
        else {
          printf("Passphrase and confirmation for key %i do not match\n\n", indx);
          return false;
        }
      }

      info->size    = strlen(info->buff);
      info->indx    = 0;
      info->is_file = false;
    }
	}
	else {
    fseek(info->data, 0, SEEK_END);
    info->size = ftell(info->data);
	
    fseek(info->data, 0, SEEK_SET);
    info->indx = 0;
  }
	return true;
}

/**
 * Run sanity checks on the source file and encryption keys
 */
bool check(config* cfg, obj* src, obj* key) {
  int src_read;
  int key_read;
  
  int indx;
  
  printf("Verifying success of key %s\n", key->name);
  
  fseek(src->data, 0, SEEK_SET);
  src->indx = 0;
  
  while(src->indx < src->size) {
    if((src_read = fread(src->buff, 1, buff_size, src->data)) < 1) {
      printf("Unable to read from %s\n", src->name);
      return false;
    }
    if (key->is_file) {
      if((key_read = fread(key->buff, 1, buff_size, key->data)) < buff_size) {
        if(key_read < 1) {
          printf("Unable to read from %s\n", key->name);
          return false;
        }
        fseek(key->data, 0, SEEK_SET);
        key->indx = 0;
      }
    }
    else {
      key_read = key->size;
    }
    
    indx = 0;
    while((indx < src_read) && (indx < key_read)) {
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

/**
 * Combine the source file with an encryption key
 */
bool combine(config* cfg, obj* src, obj* key, FILE* output_stream) {
	int src_read;
	int key_read;

	int indx;
  
  printf("Combining source %s with key %s\n", src->name, key->name);

	fseek(src->data, 0, SEEK_SET);
	src->indx = 0;

	while(src->indx < src->size) {
    if((src_read = fread(src->buff, 1, buff_size, src->data)) < 1) {
			printf("Unable to read from %s\n", src->name);
			return false;
		}
		if (key->is_file) {
      if((key_read = fread(key->buff, 1, buff_size, key->data)) < buff_size) {
        if(key_read < 1) {
          printf("Unable to read from %s\n", key->name);
          return false;
        }
        fseek(key->data, 0, SEEK_SET);
        key->indx = 0;
      }
    }
    else {
      key_read = key->size;
    }

		indx = 0;
		while((indx < src_read) && (indx < key_read)) {
      src->buff[indx] = src->buff[indx] ^ key->buff[indx];
			indx++;
			src->indx++;
			key->indx++;
		}

		fseek(src->data, (-1 * src_read), SEEK_CUR);

    if (cfg->dry_run) {
      // Simulate write
      fseek(src->data, indx, SEEK_CUR);
    }

    if(fwrite(src->buff, 1, indx, output_stream) < indx) {
      printf("Unable to write %s\n", src->name);
      return false;
    }
    fflush(output_stream);
	}
	return true;
}

/**
 * Finalize source or key objects and cleanup
 */
bool finalize(config* cfg, obj* src) {
  printf("Finalizing session for source %s\n", src->name);
  
  int errors = 0;
  
  if (src->is_file) {
    fclose(src->data);
  }
  
  layer* temp = cfg->keys;
  do {
    if (!finalize_key(cfg, temp->key)) {
      errors++;
    }
  } 
  while ((temp = temp->next) != NULL);
  
  //free(info);
	return ((errors == 0) ? true : false);
}

/**
 * Finalize key object and cleanup
 */
bool finalize_key(config* cfg, obj* key) {
  printf("Finalizing session for key %s\n", key->name);
  
  if (key->is_file) {
    //fclose(key->data);
  }
  return true;
}

//-----------------------------------------------------------------------------
// Layer utilities

/**
 * Add a new encryption/decryption layer to the configuration arguments
 */
bool add_layer(config* cfg, char* name) {
  printf("Adding new layer %s\n", name);
  
  layer* operation = malloc(sizeof(layer));
  
  if (operation == NULL) {
    printf("Cannot allocate memory for layer %s\n", name);
    return false;
  }

  operation->name = name;
  operation->key  = NULL;
  operation->next = NULL;

  if (cfg->keys == NULL) {
    cfg->keys = operation;
  }
  else {
    // Add one operation onto the end
    layer* temp = cfg->keys;
    while (temp->next != NULL) {
      temp = temp->next;
    }
    temp->next = operation;
  }
  return true;
}

/**
 * Free all created encyption/decryption layers from memory
 */
bool free_layers(config* cfg) {
  printf("Cleaning up all layers\n");
  
  if (cfg->keys != NULL) {
    layer* temp = cfg->keys;
    while (temp->next != NULL) {
      layer* orig = temp;
      temp = temp->next;
      //free(orig);
    }
    //free(temp);
  }
  return true;
}
