#pragma once

#include "MultiPE.h"
#include "hlslib/xilinx/SDAccel.h"

template <int width, class Container>
inline auto Pack(Container const &in) {
  std::vector<
      hlslib::DataPack<Data_t, width>, 
      hlslib::ocl::AlignedAllocator<hlslib::DataPack<Data_t, width>, 4096>>
      result(in.size() / width);
  for (int i = 0, iMax = in.size() / width; i < iMax; ++i) {
    result[i].Pack(&in[i * width]);
  }
  return result;
}

template <int width, class Container>
inline auto Unpack(Container const &in) {
  std::vector<Data_t> result(in.size() * width);
  for (int i = 0, iMax = in.size(); i < iMax; ++i) { 
    in[i].Unpack(&result[i * width]);
  }
  return result;
}