cmake_minimum_required(VERSION 3.0)
project(sdaccel_mm)

# Target options 
set(MM_PART_NAME "xcu50-fsvh2104-2-e" CACHE STRING "Part name for HLS.")
set(MM_DSA_NAME "xilinx_u50_gen3x16_xdma_201920_3" CACHE STRING "DSA string for Vitis.")
set(MM_TARGET_CLOCK 300 CACHE STRING "Target clock for kernel (<=0 uses DSA default).")
set(MM_TARGET_CLOCK_UNCERTAINTY 1.08 CACHE STRING "Clock uncertainty for HLS.")
set(MM_TMP_DIR /tmp CACHE STRING "Temporary directory for SDx build files.")
set(MM_ENABLE_PROFILING OFF CACHE BOOL "Collect profiling information.")
set(MM_ENABLE_DEBUGGING OFF CACHE BOOL "Inject debugging cores to design.")
set(MM_QUICK_BUILD OFF CACHE BOOL "Enable quick build option for Vitis.")
set(MM_VITIS_FLAGS "" CACHE STRING "Extra flags for Vitis.")
set(MM_VITIS_COMPILE_FLAGS "" CACHE STRING "Extra flags for Vitis compilation phase.")
set(MM_VITIS_LINKER_FLAGS "" CACHE STRING "Extra flags for Vitis linking phase.")

set(VECTOR_SIZE 1024 CACHE STRING "Vector size.")
set(ADD_PARALLEL_VECTOR_SIZE 16 CACHE STRING "Parallel vector size.")


set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake
    ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/hlslib/cmake)

find_package(Vitis REQUIRED)
find_package(Threads REQUIRED)

include_directories(include hlslib/include ${CMAKE_BINARY_DIR} ${Vitis_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_definitions("-DHLSLIB_STREAM_TIMEOUT=16")
if(((${Vitis_MAJOR_VERSION} LESS 2018) AND (${Vitis_MINOR_VERSION} LESS 3)) OR ${Vitis_MAJOR_VERSION} LESS 2017)
  add_definitions(-DHLSLIB_LEGACY_SDX=1)
else()
  add_definitions(-DHLSLIB_LEGACY_SDX=0)
endif()
if(MM_DATA_TYPE STREQUAL "half")
  add_definitions("-DMM_HALF_PRECISION")
endif()

# Hardware configuration header for HLS
configure_file(${CMAKE_SOURCE_DIR}/common/config.h.in ${CMAKE_SOURCE_DIR}/include/config.h)

# C++ source code 
set(MM_KERNEL_SRC
    ${CMAKE_SOURCE_DIR}/src/VADD.cpp
    ${CMAKE_SOURCE_DIR}/src/Memory.cpp
    ${CMAKE_SOURCE_DIR}/src/Top.cpp)
add_library(mmkernel ${MM_KERNEL_SRC})
target_link_libraries(mmkernel ${CMAKE_THREAD_LIBS_INIT})
 
 
# Software test
add_executable(CPU_simulate test/CPU_simulate.cpp)
target_link_libraries(CPU_simulate ${Vitis_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${Vitis_LIBRARIES} mmkernel)
add_test(CPU_simulate CPU_simulate)
enable_testing()
# 
# # Synthesis flags
# set(MM_SYNTHESIS_FLAGS "${MM_SYNTHESIS_FLAGS} -std=c++11 -O3 -DMM_SYNTHESIS -DHLSLIB_SYNTHESIS")
# if(MM_ADD_RESOURCE)
#   set(MM_SYNTHESIS_FLAGS "${MM_SYNTHESIS_FLAGS} -DMM_ADD_RESOURCE=${MM_ADD_RESOURCE}") 
# endif() 
# if(MM_MULT_RESOURCE) 
#   set(MM_SYNTHESIS_FLAGS "${MM_SYNTHESIS_FLAGS} -DMM_MULT_RESOURCE=${MM_MULT_RESOURCE}") 
# endif() 
# 
# 
# # Hardware kernel
# string(REPLACE " " ";" MM_VITIS_FLAGS "${MM_VITIS_FLAGS}")
# set(MM_VITIS_COMMAND
#   -s
#   ${MM_VITIS_FLAGS}
#   -I${CMAKE_SOURCE_DIR}/include
#   -I${CMAKE_SOURCE_DIR}/hlslib/include
#   -I${CMAKE_BINARY_DIR}
#   --kernel ${MM_KERNEL_NAME} 
#   --platform ${MM_DSA_NAME})
# if(MM_ENABLE_PROFILING)
#   set(MM_VITIS_COMMAND ${MM_VITIS_COMMAND}
#     --profile_kernel "data:all:all:all"
#     --profile_kernel "stall:all:all"
#     --profile_kernel "exec:all:all")
# endif()
# if(Vitis_IS_LEGACY)
#   set(MM_VITIS_COMMAND ${MM_VITIS_COMMAND}
#   --xp prop:kernel.${MM_KERNEL_NAME}.kernel_flags="${MM_SYNTHESIS_FLAGS}"
#   --max_memory_ports all)
# else()
#   set(MM_VITIS_COMMAND ${MM_VITIS_COMMAND}
#       --advanced.prop kernel.${MM_KERNEL_NAME}.kernel_flags="${MM_SYNTHESIS_FLAGS}")
# endif()
# if(MM_QUICK_BUILD)
#   set(MM_VITIS_COMMAND ${MM_VITIS_COMMAND} -Oquick)
# else()
#   set(MM_VITIS_COMMAND ${MM_VITIS_COMMAND} -O3)
# endif()
# if(MM_ENABLE_DEBUGGING)
#   set(MM_VITIS_COMMAND ${MM_VITIS_COMMAND} -g
#     --dk chipscope:MultiPEKernel_1:M_AXI_GMEM0
#     --dk chipscope:MultiPEKernel_1:M_AXI_GMEM1
#     --dk chipscope:MultiPEKernel_1:M_AXI_GMEM2)
# endif()
# if(MM_TARGET_CLOCK GREATER 0)
#   set(MM_VITIS_COMMAND ${MM_VITIS_COMMAND}
#       --kernel_frequency ${MM_TARGET_CLOCK})
# endif()
# if(MM_TWO_DIMMS)
#   # TODO: Newer versions of Vitis have again changed how this is done, and now
#   # require a separate config file with the global memory mappings.
#   if(NOT (MM_DSA_NAME MATCHES "dynamic"))
#     set(MM_VITIS_COMMAND ${MM_VITIS_COMMAND}
#       --xp misc:map_connect=add.kernel.${MM_KERNEL_NAME}_1.M_AXI_GMEM0.core.OCL_REGION_0.M00_AXI
#       --xp misc:map_connect=add.kernel.${MM_KERNEL_NAME}_1.M_AXI_GMEM1.core.OCL_REGION_0.M01_AXI
#       --xp misc:map_connect=add.kernel.${MM_KERNEL_NAME}_1.M_AXI_GMEM2.core.OCL_REGION_0.M01_AXI)
#   else()
#     set(MM_VITIS_COMMAND ${MM_VITIS_COMMAND}
#       --sp ${MM_KERNEL_NAME}_1.m_axi_gmem0:bank0
#       --sp ${MM_KERNEL_NAME}_1.m_axi_gmem1:bank1
#       --sp ${MM_KERNEL_NAME}_1.m_axi_gmem2:bank1)
#   endif()
# endif()
# 
# if(((${Vitis_MAJOR_VERSION} LESS 2018) AND (${Vitis_MINOR_VERSION} LESS 3)) OR ${Vitis_MAJOR_VERSION} LESS 2017)
#   add_custom_target(build_kernel
#     COMMAND XILINX_PATH=${CMAKE_BINARY_DIR} ${Vitis_COMPILER} -t hw
#     ${MM_VITIS_COMMAND} ${MM_VITIS_COMPILE_FLAGS} ${MM_VITIS_LINKER_FLAGS}
#     ${MM_KERNEL_SRC} -o MultiPE_hw.xclbin) 
# else()
#   # Hardware emulation
#   add_custom_command(
#     OUTPUT ${CMAKE_BINARY_DIR}/MultiPE_hw_emu.xo
#     COMMENT "Compiling for hardware emulation."
#     COMMAND ${CMAKE_COMMAND} -E env
#             XILINX_PATH=${CMAKE_BINARY_DIR}
#             ${Vitis_COMPILER} -c -t hw_emu
#             ${MM_VITIS_COMMAND} ${MM_VITIS_COMPILE_FLAGS} ${MM_VITIS_FLAGS}
#             ${MM_KERNEL_SRC} -o MultiPE_hw_emu.xo
#     DEPENDS ${MM_KERNEL_SRC})
#   add_custom_target(compile_hardware_emulation DEPENDS
#                     ${CMAKE_BINARY_DIR}/MultiPE_hw_emu.xo)
#   add_custom_command(
#     OUTPUT ${CMAKE_BINARY_DIR}/MultiPE_hw_emu.xclbin
#     COMMENT "Linking for hardware emulation."
#     COMMAND ${CMAKE_COMMAND} -E env
#             XILINX_PATH=${CMAKE_BINARY_DIR}
#             ${Vitis_COMPILER} -l -t hw_emu
#             ${MM_VITIS_COMMAND} ${MM_VITIS_LINKER_FLAGS} ${MM_VITIS_FLAGS}
#             MultiPE_hw_emu.xo
#             -o MultiPE_hw_emu.xclbin
#     DEPENDS ${CMAKE_BINARY_DIR}/MultiPE_hw_emu.xo)
#   add_custom_command(
#     OUTPUT ${CMAKE_BINARY_DIR}/emconfig.json
#     COMMENT "Generating emconfig.json file for hardware emulation."
#     COMMAND ${VITIS_ROOT_DIR}/bin/emconfigutil --platform
#             ${MM_DSA_NAME})
#   add_custom_target(link_hardware_emulation DEPENDS
#                     ${CMAKE_BINARY_DIR}/MultiPE_hw_emu.xclbin
#                     ${CMAKE_BINARY_DIR}/emconfig.json)
#   # Hardware
#   add_custom_command(
#     OUTPUT ${CMAKE_BINARY_DIR}/MultiPE_hw.xo
#     COMMENT "Compiling for hardware."
#     COMMAND ${CMAKE_COMMAND} -E env
#             XILINX_PATH=${CMAKE_BINARY_DIR}
#             ${Vitis_COMPILER} -c -t hw
#             ${MM_VITIS_COMMAND} ${MM_VITIS_COMPILE_FLAGS} ${MM_VITIS_FLAGS}
#             ${MM_KERNEL_SRC} -o MultiPE_hw.xo
#     DEPENDS ${MM_KERNEL_SRC})
#   add_custom_target(compile_hardware DEPENDS
#                     ${CMAKE_BINARY_DIR}/MultiPE_hw.xo)
#   add_custom_command(
#     OUTPUT ${CMAKE_BINARY_DIR}/MultiPE_hw.xclbin
#     COMMENT "Linking for hardware (this will run the full synthesis, placement and routing flow)."
#     COMMAND ${CMAKE_COMMAND} -E env
#             XILINX_PATH=${CMAKE_BINARY_DIR}
#             ${Vitis_COMPILER} -l -t hw
#             ${MM_VITIS_COMMAND} ${MM_VITIS_LINKER_FLAGS} ${MM_VITIS_FLAGS}
#             MultiPE_hw.xo
#             -o MultiPE_hw.xclbin
#     DEPENDS ${CMAKE_BINARY_DIR}/MultiPE_hw.xo)
#   add_custom_target(link_hardware DEPENDS
#                     ${CMAKE_BINARY_DIR}/MultiPE_hw.xclbin)
# endif()
# 
# # Host code to launch kernel
# add_executable(RunHardware.exe host/RunHardware.cpp)
# target_link_libraries(RunHardware.exe ${Vitis_LIBRARIES} ${BLAS_LIBRARIES} mmkernel)
# if(MM_TWO_DIMMS)
#   target_compile_definitions(RunHardware.exe MM_TWO_DIMMS)
# endif()
# if(MM_DATA_TYPE STREQUAL "half")
#   target_link_libraries(RunHardware.exe ${Vitis_FLOATING_POINT_LIBRARY})
# endif()
# if(MM_POWER_METER)
#   target_link_libraries(RunHardware.exe powermeter)
# endif()