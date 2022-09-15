cmake_minimum_required(VERSION 3.23)
project(lbm-proxy-app)
set(Kokkos_DIR "$ENV{KOKKOS_HOME}/lib64/cmake/Kokkos")
find_package(Kokkos REQUIRED)

set(lbminiapp_SOURCE_DIR ".")
set(geometries_DIR "${lbminiapp_SOURCE_DIR}/geometries")
set(kernels_DIR "${lbminiapp_SOURCE_DIR}/kernels")
set(lattices_DIR "${lbminiapp_SOURCE_DIR}/lattices")
set(propagation_DIR "${lbminiapp_SOURCE_DIR}/propagation")

set(top_SRCS
  ${lbminiapp_SOURCE_DIR}/Communication.cpp
  ${lbminiapp_SOURCE_DIR}/Parameters.cpp
  ${lbminiapp_SOURCE_DIR}/Main.cpp)

set(geometries_SRCS
  ${geometries_DIR}/GeometryAAAB.cpp
  ${geometries_DIR}/Geometry.cpp)

set(kernels_SRCS
  ${kernels_DIR}/KernelAA_AOS_Base.cpp
  ${kernels_DIR}/KernelAA_SOA_Base.cpp
  ${kernels_DIR}/KernelAA_SOA_Unroll.cpp
  ${kernels_DIR}/KernelAB_AOS_Base.cpp
  ${kernels_DIR}/KernelAB_SOA_Base.cpp
  ${kernels_DIR}/KernelAB_SOA_Pull.cpp
  ${kernels_DIR}/KernelAB_SOA_Unroll.cpp
  ${kernels_DIR}/KernelAB_SOA_Unroll_Pull.cpp)

set(lattices_SRCS
  ${lattices_DIR}/LatticeD3Q19.cpp
  ${lattices_DIR}/LatticeD3Q27.cpp)

set(propagation_SRCS
  ${propagation_DIR}/PropagationAA.cpp
  ${propagation_DIR}/PropagationAB.cpp
  ${propagation_DIR}/Propagation.cpp)

set(SOURCE_FILES ${top_SRCS} ${geometries_SRCS} ${kernels_SRCS} ${lattices_SRCS} ${propagation_SRCS})

add_executable(lbm-proxy-app ${SOURCE_FILES})
target_link_libraries(lbm-proxy-app Kokkos::kokkos)

if(SYCL)
        set(GTL_DIR "/opt/cray/pe/mpich/8.1.16/gtl")
	target_link_directories(lbm-proxy-app PUBLIC "${GTL_DIR}/lib")
	target_link_libraries(lbm-proxy-app mpi_gtl_cuda)
	target_compile_options(lbm-proxy-app PUBLIC -DSOA -DUSE_KOKKOS_SYCL -fsycl-targets=nvptx64-nvidia-cuda)
	target_link_options(lbm-proxy-app PUBLIC -Xsycl-target-backend "--cuda-gpu-arch=sm_80")
elseif(CUDA)
	target_compile_options(lbm-proxy-app PUBLIC -DSOA -DUSE_KOKKOS_CUDA)
endif()

target_include_directories(lbm-proxy-app PUBLIC ${lbminiapp_SOURCE_DIR} ${geometries_DIR} ${kernels_DIR} ${lattices_DIR} ${propagation_DIR})
