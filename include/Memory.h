#ifndef __MEMORY_HEADER__
#define __MEMORY_HEADER__

#include "MultiPE.h"

void Load(MemoryPack_t const *in, Stream<MemoryPack_t> &out);
void Store(Stream<MemoryPack_t> &in, MemoryPack_t *out);

#endif