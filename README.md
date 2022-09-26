# I. LBM Proxy App (Kokkos with CUDA backend) on Polaris
## 1. Log in Polaris
```
	ssh [username]@polaris.alcf.anl.gov
```
## and enter passcode.
## 2. Download kokkos and this mini app.
```
	git clone https://github.com/kokkos/kokkos
	git clone https://github.com/lucaso19891019/MINIAPP
```
## 3. Go to computing node and build kokkos.
```
	qsub -I -l select=[number of nodes] -l walltime=[hrs:min:sec] -A [account name] -q [queue name]
	cd ~/kokkos
	mkdir build
	cd build
	module load cmake cudatoolkit-standalone/11.6.2
	cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX="./kokkos-3.6.01" -DCMAKE_CXX_COMPILER=CC -DKokkos_ENABLE_OPENMP=OFF -DKokkos_ENABLE_SERIAL=ON -DKokkos_ARCH_ZEN2=ON -DKokkos_ARCH_AMPERE80=ON -DKokkos_ENABLE_CUDA=ON -DKokkos_ENABLE_AGGRESSIVE_VECTORIZATION=ON -DKokkos_ENABLE_TESTS=OFF -DBUILD_TESTING=OFF -DKokkos_ENABLE_CUDA_LAMBDA=ON -DCMAKE_CXX_STANDARD=17 ..
	make -j32
	make install
	export KOKKOS_HOME=$PWD/kokkos-3.6.01
	export CPATH=$KOKKOS_HOME/include:$CPATH
	export MPICH_GPU_SUPPORT_ENABLED=1
```
## 4. Compile the mini app.
```
	cd ~/MINIAPP
	cmake -DCMAKE_CXX_COMPILER=CC -DCMAKE_C_COMPILER=cc -DCMAKE_CXX_STANDARD=17 -DKokkos_CUDA=ON -S src/ -B test_build/
	make -C test_build/ -j16
```
# II. LBM Proxy App (Kokkos with SYCL backend) on Polaris
## 1. Log in Polaris
```
	ssh [username]@polaris.alcf.anl.gov
```
## and enter passcode.
## 2. Download kokkos and this mini app.
```
	git clone -b develop https://github.com/kokkos/kokkos kokkos-dev
	git clone https://github.com/lucaso19891019/MINIAPP
```
## 3. Go to computing node and build kokkos.
```
	qsub -I -l select=[number of nodes] -l walltime=[hrs:min:sec] -A [account name] -q [queue name]
	cd ~/kokkos-dev
	mkdir build
	cd build
	module load cmake mpiwrappers/cray-mpich-llvm llvm-sycl/2022-06
	module load cudatoolkit-standalone/11.6.2
	cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX="./kokkos-3.6.01" -DCMAKE_CXX_COMPILER=clang++ -DKokkos_ENABLE_OPENMP=OFF -DKokkos_ENABLE_SERIAL=ON -DKokkos_ARCH_ZEN2=ON -DKokkos_ARCH_AMPERE80=ON -DKokkos_ENABLE_SYCL=ON -DKokkos_ENABLE_AGGRESSIVE_VECTORIZATION=ON -DKokkos_ENABLE_TESTS=OFF -DBUILD_TESTING=OFF -DCMAKE_CXX_STANDARD=17 ..
	make -j32
	make install
	export KOKKOS_HOME=$PWD/kokkos-3.6.01
	export CPATH=$KOKKOS_HOME/include:$CPATH
	export MPICH_GPU_SUPPORT_ENABLED=1
```
## 4. Compile the mini app.
```
	cd ~/MINIAPP
	cmake -DCMAKE_CXX_COMPILER=mpicxx -DCMAKE_C_COMPILER=mpicc -DCMAKE_CXX_STANDARD=17 -DKokkos_SYCL=ON -S src/ -B test_build/
	make -C test_build/ -j16
```
# III. LBM Proxy App (SYCL) on Polaris
## 1. Log in Polaris
```
	ssh [username]@polaris.alcf.anl.gov
```
## and enter passcode.
## 2. Download this mini app.
```
	git clone https://github.com/lucaso19891019/MINIAPP
```
## 3. Go to computing node and compile.
```
	qsub -I -l select=[number of nodes] -l walltime=[hrs:min:sec] -A [account name] -q [queue name]
	module load cmake mpiwrappers/cray-mpich-llvm llvm-sycl/2022-06
	module load cudatoolkit-standalone/11.6.2
	export MPICH_GPU_SUPPORT_ENABLED=1
	cd ~/MINIAPP
	cmake -DCMAKE_CXX_COMPILER=mpicxx -DCMAKE_C_COMPILER=mpicc -DCMAKE_CXX_STANDARD=17 -DSYCL=ON -S src/ -B test_build/
	make -C test_build/ -j16
```
# IV. Run
```
	mpiexec --np [number of processors] -ppn [number of processors per node] -envall ./set_affinity_gpu_polaris.sh ./test_build/lbm-proxy-app test/[selected input file]
```
## each node has 4 GPUs, so the number of processors per node should be no greater than 4.
	

