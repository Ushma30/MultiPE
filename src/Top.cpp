#include "hlslib/xilinx/Simulation.h"
#include "Memory.h"
#include "VADD.h"

void MultiPE(MemoryPack_t const a[], MemoryPack_t const b[], MemoryPack_t c[])
{
#pragma HLS INTERFACE m_axi port = a bundle = gmem0 offset = slave
#pragma HLS INTERFACE m_axi port = b bundle = gmem1 offset = slave
#pragma HLS INTERFACE m_axi port = c bundle = gmem1 offset = slave
#pragma HLS INTERFACE s_axilite port = in
#pragma HLS INTERFACE s_axilite port = out
#pragma HLS INTERFACE s_axilite port = c
#pragma HLS INTERFACE s_axilite port = return

#pragma HLS DATAFLOW

    Stream<MemoryPack_t> pipe[2];

#ifndef HLSLIB_SYNTHESIS
    for (unsigned n = 0; n < 2; ++n)
    {
        pipe[n].set_name(("pipe[" + std::to_string(n) + "]").c_str());
    }
#endif

    HLSLIB_DATAFLOW_INIT();

    // HLSLIB_DATAFLOW_FUNCTION emulates concurrent execution in hardware by
    // wrapping function calls in std::thread

    HLSLIB_DATAFLOW_FUNCTION(Load, a, pipe[0]);

    HLSLIB_DATAFLOW_FUNCTION(VADD, pipe[0], b, pipe[1]);

    HLSLIB_DATAFLOW_FUNCTION(Store, pipe[1], c);

    HLSLIB_DATAFLOW_FINALIZE();
}