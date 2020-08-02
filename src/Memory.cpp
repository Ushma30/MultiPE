#include "Memory.h"

void Load(MemoryPack_t const *in, Stream<MemoryPack_t> &out)
{
    for (int t = 0; t < T/ADD_PARALLEL_VECTOR_SIZE; ++t)
    {
#pragma HLS PIPELINE
        out.Push(in[t]);
    }
}

void Store(Stream<MemoryPack_t> &in, MemoryPack_t *out)
{
    for (int t = 0; t < T/ADD_PARALLEL_VECTOR_SIZE; ++t)
    {
#pragma HLS PIPELINE
        out[t] = in.Pop();
    }
}