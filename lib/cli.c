
//------------------------------------------------------------------------------
// Dependencies

#include <stdio.h>   // printf
#include <stdlib.h>  // free
#include <string.h>  // strcmp
#include <time.h>    // clock

#include <alias.h>   // true, false
#include <data.h>    // config, layer
#include <layer.h>   // add_layer, free_layer
#include <cli.h>

//------------------------------------------------------------------------------
// Argument processing

/**
 * Process CLI arguments
 */
void process_args(config* cfg, int argc, char* argv[]) {
  cfg->show_help      = false;
  cfg->dry_run        = false;
  cfg->quiet          = false;
  cfg->hash_threshold = 200;
  cfg->src_indx       = 1;
  cfg->key_length     = 0;
  cfg->keys           = NULL;
  cfg->start          = clock();

  int arg_indx     = 1;
  int arg_layers   = 0;
  int source_found = false;

  while (arg_indx < argc) {
    char* arg = argv[arg_indx];

    if ((strcmp(arg, "-h") == 0) || (strcmp(arg, "--help") == 0)) {
      cfg->show_help = true;
      break;
    } else if ((strcmp(arg, "-d") == 0) || (strcmp(arg, "--dry_run") == 0)) {
      cfg->dry_run = true;
    } else if ((strcmp(arg, "-q") == 0) || (strcmp(arg, "--quiet") == 0)) {
      cfg->quiet = true;
    } else if (arg[0] == '-') {
      printf("Unrecognized option: %s\n", arg);
      cfg->show_help = true;
      break;
    } else {
      add_layer(cfg, arg, arg_layers);
      arg_layers++;

      if (!source_found) {
        source_found = true;
        cfg->src_indx = arg_indx;
      }
    }
    arg_indx++;
  }

  if (argc == 1) {
    cfg->show_help = true;
  }
  if (arg_layers) {
    cfg->key_length  = arg_layers - 1;

    layer* src_layer = cfg->keys;
    cfg->keys = cfg->keys->next;

    free_layer(src_layer);
  }
}
