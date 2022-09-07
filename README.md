# LBM Proxy App with Kokkos/SYCL
## 1. JLSE Login 
```
	$ ssh -Y username@login.jlse.anl.gov
```
input pass phrase and approve in DUO
## 2. Download the proxy app
### (1) Exit compute node, if you are on compute node.
```
	$ exit
	$ cd ~
```
### (2) Follow 1. if you have not logged into JLSE.
### (3) Download the proxy app
```
	$ git clone https://gitlab.jlse.anl.gov/intel-dga/vis/lbmminiapp
```
Type in username and password if required.
## 3. Compile (Kokkos) (Jump to 4. to compile SYCL code.)	
### (1) Login to compute node if you have not done so.
```
	$ qsub -q arcticus -n 1 -t 30 -I
	$ export MODULEPATH=/soft/restricted/CNDA/modulefiles
	$ module load oneapi
```
### (2) Load Kokkos module
#### (a) SYCL backend (Jump to 3.(2)(b) to compile with OpenMP Target backend)
```
	$ module load kokkos/sycl_intel
	$ cd lbmminiapp
	$ cp Makefile_kokkos_sycl_ats Makefile
```
#### (b) OpenMP Target backend
``` 
	$ module load kokkos/openmptarget_intel
	$ cd lbmminiapp
	$ cp Makefile_kokkos_omptarget_ats Makefile
```
### (3) Compile the code
```
	$ make
```
### (4) Jump to 5. to run.
## 4. Compile (SYCL)
### (1) Login to compute node if you have not done so.
```
	$ qsub -q arcticus -n 1 -t 30 -I
	$ export MODULEPATH=/soft/restricted/CNDA/modulefiles
	$ module load oneapi
```
### (2) Compile the code (remove exisiting Kokkos dependencies if they exist. Use "make clean" from the Kokkos Makefile.)
```
	$ cd lbmminiapp
	$ cp Makefile_sycl Makefile 
	$ make
```
## 5. Run
```
	$ mpirun -n 2 ./test_build/lbm-proxy-app test/8_input_aa_unroll.txt
```
### (1) Users can choose different input files in test directory, to select different propagation patterns. (AA, AB and AB Pull).

#### (a) To test with single tile
```
	$ ZE_AFFINITY_MASK=0.0 ./test_build/lbm-proxy-app test/8_input_aa_unroll.txt
```
#### (b) To test with single GPU
```
	$ ZE_AFFINITY_MASK=0 ./test_build/lbm-proxy-app test/8_input_aa_unroll.txt
```
## 6. Profile
### (1) Use iprof
```
	$ module load iprof
	$ ZE_AFFINITY_MASK=0.0 iprof ./test_build/lbm-proxy-app test/8_input_aa_unroll.txt
```
### (2) Use Advisor
```
	$ ZE_AFFINITY_MASK=0.0  advisor --collect=roofline --profile-gpu -- ./test_build/lbm-proxy-app test/8_input_aa_unroll.txt
```
