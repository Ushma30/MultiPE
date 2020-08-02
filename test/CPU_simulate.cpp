#include "utility.h"
#include "MultiPE.h"
#include "goldenModel.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <type_traits>
#include <vector>

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
    std::cout << "Kernel invocation..." << std::endl;
    MultiPE(aMem.data(), bMem.data(), cMem.data());

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