
//------------------------------------------------------------------------------
// Dependencies

#include <alias.h>  // bool
#include <data.h>   // config, layer

//------------------------------------------------------------------------------
// Function prototypes

bool add_layer(config* cfg, char* name, unsigned int indx);
bool free_layers(config* cfg);
void free_layer(layer* layr);
