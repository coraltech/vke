
//------------------------------------------------------------------------------
// Dependencies

#include <stdlib.h>  // malloc, free
#include <string.h>  // strlen

#include <sha3.h>    // sha_ctx, rhash_sha3_512_init, rhash_sha3_update,
                     // rhash_sha3_final
#include <hash.h>

//------------------------------------------------------------------------------
// Initialization

char* get_hash(char* input) {
  sha3_ctx* ctx = malloc(sizeof(sha3_ctx));

  rhash_sha3_512_init(ctx);
  rhash_sha3_update(ctx, (const unsigned char*)input, strlen(input));
  rhash_sha3_final(ctx, NULL);

  return (char*)ctx->hash;
}
