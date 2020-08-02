#ifndef __MULTI_PE_HEADER__
#define __MULTI_PE_HEADER__

#include "config.h"
#include "hlslib/xilinx/DataPack.h"
#include "hlslib/xilinx/Stream.h"

using hlslib::Stream;
using hlslib::DataPack;

using MemoryPack_t = DataPack<Data_t, ADD_PARALLEL_VECTOR_SIZE>;

void MultiPE(MemoryPack_t const a[], MemoryPack_t const b[], MemoryPack_t c[]);

#endif