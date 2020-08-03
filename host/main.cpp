#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <type_traits>
#include <vector>

#include "utility.h"
#include "MultiPE.h"
#include "goldenModel.h"
#include "hlslib/xilinx/Utility.h"

void vadd(int const a[], int const b[], int c[])
{
    for (int t = 0; t < T; ++t)
    {
        c[t] = a[t] + b[t];
    }
}

int main(int, char **)
{

    std::cout << "Simulation is started..." << std::endl;
    std::vector<Data_t> A(T);
    std::vector<Data_t> B(T);
    std::vector<Data_t> cReference(T, 0);

    std::default_random_engine rng(5);
    typename std::conditional<
        std::is_integral<Data_t>::value, std::uniform_int_distribution<unsigned long>,
        std::uniform_real_distribution<double>>::type dist(1, 10);

    /*
    std::for_each(A.begin(), A.end(),
                  [&dist, &rng](Data_t &in) { in = Data_t(dist(rng)); });
    std::for_each(B.begin(), B.end(),
                  [&dist, &rng](Data_t &in) { in = Data_t(dist(rng)); });
    */

    hlslib::UnsetEnvironmentVariable("XCL_EMULATION_MODE");
    std::string path = "MultiPE_hw.xclbin";

    std::iota(A.begin(), A.end(), 0);
    std::iota(B.begin(), B.end(), 10);
    std::vector<MemoryPack_t, hlslib::ocl::AlignedAllocator<MemoryPack_t, 4096>> aMem;
    std::vector<MemoryPack_t, hlslib::ocl::AlignedAllocator<MemoryPack_t, 4096>> bMem, cMem;

    std::cout << "Kernel memory initialization..." << std::endl;

    aMem = Pack<ADD_PARALLEL_VECTOR_SIZE>(A);
    bMem = Pack<ADD_PARALLEL_VECTOR_SIZE>(B);
    cMem = Pack<ADD_PARALLEL_VECTOR_SIZE>(cReference);

    std::cout << "Golden model invocation..." << std::endl;
    VADD_Ref(A.data(), B.data(), cReference.data());

    try
    {
        std::cout << "Initializing OpenCL context...\n"
                  << std::flush;
        hlslib::ocl::Context context;

        std::cout << "Programming device...\n"
                  << std::flush;
        auto program = context.MakeProgram(path);

        std::cout << "Initializing device memory...\n"
                  << std::flush;
#ifdef MM_TWO_DIMMS
        auto aDevice = context.MakeBuffer<MemoryPack_t, hlslib::ocl::Access::read>(
            hlslib::ocl::MemoryBank::bank0, size_n * size_k / kMemoryWidthK);
        auto bDevice = context.MakeBuffer<MemoryPack_t, hlslib::ocl::Access::read>(
            hlslib::ocl::MemoryBank::bank1, size_k * size_m / kMemoryWidthM);
        auto cDevice =
            context.MakeBuffer<MemoryPackM_t, hlslib::ocl::Access::write>(
                hlslib::ocl::MemoryBank::bank1, size_n * size_m / kMemoryWidthM);
#else
        auto aDevice = context.MakeBuffer<MemoryPack_t, hlslib::ocl::Access::read>(
            T / kMemoryWidth);
        auto bDevice = context.MakeBuffer<MemoryPack_t, hlslib::ocl::Access::read>(
            T / kMemoryWidth);
        auto cDevice =
            context.MakeBuffer<MemoryPack_t, hlslib::ocl::Access::write>(
                T / kMemoryWidth);
#endif

        std::cout << "Copying memory to device...\n"
                  << std::flush;
        aDevice.CopyFromHost(aMem.cbegin());
        bDevice.CopyFromHost(bMem.cbegin());
        cDevice.CopyFromHost(cMem.cbegin());

        std::cout << "Creating kernel...\n"
                  << std::flush;
        auto kernel = program.MakeKernel("MultiPE", aDevice,
                                         bDevice, cDevice);

        std::cout << "Executing kernel...\n"
                  << std::flush;
        const auto elapsed = kernel.ExecuteTask();

        const auto perf = 1e-9 *
                          (2 * static_cast<float>(T)) /
                          elapsed.first;

        std::cout << "Kernel executed in " << elapsed.first
                  << " seconds, corresponding to a performance of " << perf
                  << " GOp/s.\n";
        std::cout << "Copying back result...\n"
                  << std::flush;
        cDevice.CopyToHost(cMem.begin());
    }
    catch (std::runtime_error const &err)
    {
        std::cerr << "Execution failed with error: \"" << err.what() << "\"."
                  << std::endl;
        return 1;
    }

    std::cout << "Verifying results...\n"
              << std::flush;

    const auto cTest = Unpack<ADD_PARALLEL_VECTOR_SIZE>(cMem);

    for (int i = 0; i < T; ++i)
    {
        const auto diff = std::abs(cReference[i] - cTest[i]);
        if (diff > 1e-4)
        {
            std::cerr << "Mismatch at " << i << ": " << cTest[i] << " (should be "
                      << cReference[i] << ")." << std::endl;
            return 1;
        }
    }
    std::cout
        << "Test ran successfully.\n";

    return 0;
}