
//------------------------------------------------------------------------------
// Dependencies

#include <stdio.h>   // printf
#include <stdlib.h>  // malloc, free
#include <string.h>  // strlen, strcpy
#include <time.h>    // CLOCKS_PER_SEC, clock_t, clock

#include <alias.h>   // bool, true, false
#include <data.h>    // config, layer
#include <layer.h>

//------------------------------------------------------------------------------
// Layer specific functionality

/**
 * Add a new encryption/decryption layer to the configuration arguments
 */
bool add_layer(config* cfg, char* name, unsigned int indx) {
  layer* operation = malloc(sizeof(layer));

  if (!cfg->quiet) {
    int msec = ((clock_t)(clock() - cfg->start) * 1000 / CLOCKS_PER_SEC);
    printf("Adding new layer %s (%dsec & %dms)\n", name, msec / 1000, msec % 1000);
  }

  if (operation == NULL) {
    printf("Cannot allocate memory for layer %s\n", name);
    return false;
  }

  operation->name = (char*)malloc((strlen(name) + 1) * sizeof(char));
  strcpy(operation->name, name);

  operation->indx = indx;
  operation->key  = NULL;
  operation->next = NULL;

  if (cfg->keys == NULL) {
    cfg->keys = operation;
  } else {
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

  if (!cfg->quiet) {
    int msec = ((clock_t)(clock() - cfg->start) * 1000 / CLOCKS_PER_SEC);
    printf("Cleaning up all layers (%dsec & %dms)\n", msec / 1000, msec % 1000);
  }

  if (cfg->keys != NULL) {
    layer* layr = cfg->keys;
    while (layr != NULL) {
      layer temp;
      memcpy(&temp, layr, sizeof(layer));

      free_layer(layr);
      layr = temp.next;
    }
  }
  return true;
}

/**
 * Free a registered layer
 */
void free_layer(layer* layr) {
  if (layr != NULL) {
    free(layr->name);
    free(layr);
  }
}
