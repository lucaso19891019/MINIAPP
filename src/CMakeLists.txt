cmake_minimum_required(VERSION 3.23)
project(lbm-proxy-app)

if(Kokkos_SYCL OR USE_KOKKOS_CUDA OR Kokkos_OMPT)
set(USE_KOKKOS ON)
find_package(Kokkos REQUIRED)
endif()

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

if(USE_KOKKOS)
  target_link_libraries(lbm-proxy-app Kokkos::kokkos)
endif()

if(Kokkos_SYCL OR SYCL)
if(SYCL_POLARIS)
        set(GTL_DIR "/opt/cray/pe/mpich/8.1.16/gtl")
	target_link_directories(lbm-proxy-app PUBLIC "${GTL_DIR}/lib")
	target_link_libraries(lbm-proxy-app mpi_gtl_cuda)
endif()


if(SYCL_DEVICE_NV_A100)
	set(SYCL_DEVICE "\'--cuda-gpu-arch=sm_80\'")
	set(SYCL_FLAGS "-fsycl" "-fsycl-targets=nvptx64-nvidia-cuda" "-Xsycl-target-backend" ${SYCL_DEVICE})
elseif(SYCL_DEVICE_NV_V100)
	set(SYCL_DEVICE "\'--cuda-gpu-arch=sm_70\'")
	set(SYCL_FLAGS "-fsycl" "-fsycl-targets=nvptx64-nvidia-cuda" "-Xsycl-target-backend" ${SYCL_DEVICE})
elseif(SYCL_DEVICE_INTEL)
	set(SYCL_FLAGS "-cxx=dpcpp")
endif()

if(SYCL AND SYCL_HAND)
	set(SYCL_HAND_FLAG "-DHAND")
if(SYCL_LIKE_KOKKOS)
	set(SYLC_HAND_FLAG "-DHAND" "-DLIKE_KOKKOS")
endif()
else()
	set(SYCL_HAND_FLAG "")
endif()

endif()


if(Kokkos_OMPT)
	set(OMPT_FLAGS "-cxx=icpx" "-fiopenmp" "-fPIC" "-Wno-openmp-mapping" "-D__STRICT_ANSI__" "-fopenmp-targets=spir64")
endif()

if(SYCL)
	target_compile_options(lbm-proxy-app PUBLIC -DSOA -DUSE_SYCL -DHAND -fsycl -fsycl-targets=nvptx64-nvidia-cuda -Xsycl-target-backend '--cuda-gpu-arch=sm_80')
	target_link_options(lbm-proxy-app PUBLIC -fsycl -fsycl-targets=nvptx64-nvidia-cuda -Xsycl-target-backend '--cuda-gpu-arch=sm_80')
elseif(Kokkos_SYCL)
	target_compile_options(lbm-proxy-app PUBLIC -DSOA -DUSE_KOKKOS_SYCL -fsycl -fsycl-targets=nvptx64-nvidia-cuda -Xsycl-target-backend '--cuda-gpu-arch=sm_80')
	target_link_options(lbm-proxy-app PUBLIC -fsycl -fsycl-targets=nvptx64-nvidia-cuda -Xsycl-target-backend '--cuda-gpu-arch=sm_80')
elseif(Kokkos_OMPT)
	target_compile_options(lbm-proxy-app PUBLIC -DSOA -DUSE_KOKKOS_OPENMPTARGET "${OMPT_FLAGS}")
	target_link_options(lbm-proxy-app PUBLIC "${OMPT_FLAGS}")
elseif(USE_KOKKOS_CUDA)
	target_compile_options(lbm-proxy-app PUBLIC -DSOA -DUSE_KOKKOS_CUDA)
endif()

target_include_directories(lbm-proxy-app PUBLIC ${lbminiapp_SOURCE_DIR} ${geometries_DIR} ${kernels_DIR} ${lattices_DIR} ${propagation_DIR})
