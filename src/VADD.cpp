#include "VADD.h"
#include "hlslib/xilinx/Simulation.h"

void ReadKernel_VADD(MemoryPack_t const in[], Stream<MemoryPack_t> &out)
{
    for (int t = 0; t < T / ADD_PARALLEL_VECTOR_SIZE; ++t)
    {
#pragma HLS PIPELINE
        out.Push(in[t]);
    }
}

void Compute_VADD(Stream<MemoryPack_t> &A, Stream<MemoryPack_t> &B, Stream<MemoryPack_t> &C)
{
    for (int t = 0; t < T/ADD_PARALLEL_VECTOR_SIZE; ++t)
    {
#pragma HLS PIPELINE
        auto a = A.Pop();
        auto b = B.Pop();
        auto res = a + b;
        C.Push(res);
#ifndef HLSLIB_SYNTHESIS
        std::cout << "C[" << t << +"]=" << res << std::endl;
#endif
    }
}

void VADD(Stream<MemoryPack_t> &A, MemoryPack_t const B[], Stream<MemoryPack_t> &C)
{
#pragma HLS DATAFLOW

    Stream<MemoryPack_t> add_pipe[1];

#ifndef HLSLIB_SYNTHESIS
    for (unsigned n = 0; n < 1; ++n)
    {
        add_pipe[n].set_name(("add_pipe[" + std::to_string(n) + "]").c_str());
    }
#endif

    HLSLIB_DATAFLOW_INIT();

    // HLSLIB_DATAFLOW_FUNCTION emulates concurrent execution in hardware by
    // wrapping function calls in std::thread

    HLSLIB_DATAFLOW_FUNCTION(ReadKernel_VADD, B, add_pipe[0]);

    HLSLIB_DATAFLOW_FUNCTION(Compute_VADD, A, add_pipe[0], C);

    HLSLIB_DATAFLOW_FINALIZE();
}