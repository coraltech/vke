
//------------------------------------------------------------------------------
// Dependencies

#include <stdio.h>  // FILE

#include <alias.h>  // bool
#include <data.h>   // config, obj

//------------------------------------------------------------------------------
// Function prototypes

bool initialize(config* cfg, obj* info, char* name, int indx,
    const char* access, bool force_file);

bool check(config* cfg, obj* src, obj* key);
bool combine(config* cfg, obj* src, obj* key, FILE* output_stream);

bool finalize(config* cfg, obj* src);
bool finalize_source(config* cfg, obj* src);
bool finalize_key(config* cfg, obj* key);
