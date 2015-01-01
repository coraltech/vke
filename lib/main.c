/**
 ******************************************************************************
 ***                                                                        ***
 *                  VKE   -   Variable Key Encryption                         *
 ***                                                                        ***
 ******************************************************************************
 */
//------------------------------------------------------------------------------
// Dependencies

#include <stdio.h>  // FILE, stderr, printf
#include <stdlib.h> // malloc, free
#include <time.h>   // CLOCKS_PER_SEC, clock_t, clock

#include <alias.h>  // true, false
#include <data.h>   // config, obj, layer
#include <cli.h>    // process_args
#include <vke.h>    // initialize, check, combine, finalize
#include <layer.h>  // free_layers

//------------------------------------------------------------------------------
// Execution gateway

int main(int argc, char* argv[]) {
  config cfg;
  obj src;

  char *help[] =
      {
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
          "                                                                                   " };

  process_args(&cfg, argc, argv);

  if (cfg.key_length
      && (!initialize(&cfg, &src, argv[cfg.src_indx], 0, "rb+", true))) {
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
  int key_indx = cfg.key_indx;
  int errors = 0;

  if (cfg.keys != NULL) {
    // First pass - Verify to minimize the chances of screwing up our file.
    layer* temp = cfg.keys;
    do {
      temp->key = (struct obj*) malloc(sizeof(struct obj));

      if (temp->key == NULL) {
        printf("Cannot create memory for key %s", temp->name);
        errors++;
      } else if (initialize(&cfg, temp->key, temp->name, (key_indx - 1), "rb",
          false)) {
        if (!check(&cfg, &src, temp->key)) {
          errors++;
        }
      } else {
        errors++;
      }
      key_indx++;
    } while ((temp = temp->next) != NULL);

    // Second pass - Combine source and keys to toggle encryption / decryption.
    if (errors == 0) {
      temp = cfg.keys;
      do {
        if (!combine(&cfg, &src, temp->key, output_stream)) {
          errors++;
        }
      } while ((temp = temp->next) != NULL);
    }
  }

  if (!finalize(&cfg, &src)) {
    errors++;
  }
  if (!free_layers(&cfg)) {
    errors++;
  }

  if (errors > 0) {
    exit((errors + 1));
  }

  int msec = ((clock_t)(clock() - cfg.start) * 1000 / CLOCKS_PER_SEC);
  printf("Done in %dsec & %dms\n\n", msec / 1000, msec % 1000);

  exit(EXIT_SUCCESS);
}
