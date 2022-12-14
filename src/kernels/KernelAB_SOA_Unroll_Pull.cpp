
#include <iostream>


#include "KernelAB_SOA_Unroll_Pull.hh"

/******************************************************************************/
void KernelAB_SOA_Unroll_Pull::setup()
{
#ifdef USE_KOKKOS
   stencil_d_ = myViewPDF("stencil_d_",_LATTICESIZE_ * _DIMS_);
   weight_d_ = myViewPDF("weight_d_",_LATTICESIZE_);
   stencil_h_ = Kokkos::create_mirror_view(stencil_d_);
   weight_h_ = Kokkos::create_mirror_view(weight_d_);
   
   for (int stencilIdx = 0; stencilIdx < _LATTICESIZE_; stencilIdx++)
   {
      stencil_h_(_DIMS_*stencilIdx+0) = lattice_.stencil_[stencilIdx].dir[0];
      stencil_h_(_DIMS_*stencilIdx+1) = lattice_.stencil_[stencilIdx].dir[1];
      stencil_h_(_DIMS_*stencilIdx+2) = lattice_.stencil_[stencilIdx].dir[2];
      weight_h_(stencilIdx) = lattice_.stencil_[stencilIdx].c;
   }
   
   Kokkos::deep_copy(stencil_d_,stencil_h_);
   Kokkos::deep_copy(weight_d_,weight_h_);

   Kokkos::fence();
#else
   stencil_ = new Pdf[_LATTICESIZE_ * _DIMS_];
   weight_ = new Pdf[_LATTICESIZE_];


   for (int stencilIdx = 0; stencilIdx < _LATTICESIZE_; stencilIdx++)
   {
      stencil_[_DIMS_*stencilIdx+0] = lattice_.stencil_[stencilIdx].dir[0];
      stencil_[_DIMS_*stencilIdx+1] = lattice_.stencil_[stencilIdx].dir[1];
      stencil_[_DIMS_*stencilIdx+2] = lattice_.stencil_[stencilIdx].dir[2];
      weight_[stencilIdx] = lattice_.stencil_[stencilIdx].c;
   }
#ifdef USE_SYCL
	weight_d_=sycl::malloc_device<Pdf>(_LATTICESIZE_,q_);
	stencil_d_=sycl::malloc_device<Pdf>(_LATTICESIZE_* _DIMS_,q_);
	q_.memcpy(weight_d_,weight_,_LATTICESIZE_*sizeof(Pdf));
	q_.memcpy(stencil_d_,stencil_,_LATTICESIZE_* _DIMS_*sizeof(Pdf));
	q_.wait();
#endif
#endif
   return;
}

/******************************************************************************/
/******************************************************************************/
#ifdef USE_KOKKOS
void KernelAB_SOA_Unroll_Pull::timestepForce(myViewPDF dstrb_src, myViewPDF dstrb_tgt, int startIdx, int countIdx)
{
   auto this_adjacency_d_=geometry_.adjacency_d_;
   
   int nFluid = geometry_.getNumFluidPts();
   Kokkos::parallel_for(myPolicy(startIdx, startIdx+countIdx),KOKKOS_CLASS_LAMBDA (const int fluidIdx)
   {

     Pdf f[3] = {_GRAVITY_, 0.0, 0.0};
      Pdf d[_LATTICESIZE_];
      d[0] = dstrb_src(this_adjacency_d_(nFluid * 0 + fluidIdx));
      d[1] = dstrb_src(this_adjacency_d_(nFluid * 1 + fluidIdx));
      d[2] = dstrb_src(this_adjacency_d_(nFluid * 2 + fluidIdx));
      d[3] = dstrb_src(this_adjacency_d_(nFluid * 3 + fluidIdx));
      d[4] = dstrb_src(this_adjacency_d_(nFluid * 4 + fluidIdx));
      d[5] = dstrb_src(this_adjacency_d_(nFluid * 5 + fluidIdx));
      d[6] = dstrb_src(this_adjacency_d_(nFluid * 6 + fluidIdx));
      d[7] = dstrb_src(this_adjacency_d_(nFluid * 7 + fluidIdx));
      d[8] = dstrb_src(this_adjacency_d_(nFluid * 8 + fluidIdx));
      d[9] = dstrb_src(this_adjacency_d_(nFluid * 9 + fluidIdx));
      d[10] = dstrb_src(this_adjacency_d_(nFluid * 10 + fluidIdx));
      d[11] = dstrb_src(this_adjacency_d_(nFluid * 11 + fluidIdx));
      d[12] = dstrb_src(this_adjacency_d_(nFluid * 12 + fluidIdx));
      d[13] = dstrb_src(this_adjacency_d_(nFluid * 13 + fluidIdx));
      d[14] = dstrb_src(this_adjacency_d_(nFluid * 14 + fluidIdx));
      d[15] = dstrb_src(this_adjacency_d_(nFluid * 15 + fluidIdx));
      d[16] = dstrb_src(this_adjacency_d_(nFluid * 16 + fluidIdx));
      d[17] = dstrb_src(this_adjacency_d_(nFluid * 17 + fluidIdx));
      d[18] = dstrb_src(nFluid * 18 + fluidIdx);

      Pdf rho = 0.0;
      Pdf v[3] = {0.0, 0.0, 0.0};

      rho += d[0];   v[0] += d[0];
      rho += d[1];   v[0] -= d[1];
      rho += d[2];                  v[1] += d[2];
      rho += d[3];                  v[1] -= d[3];
      rho += d[4];                                 v[2] += d[4];
      rho += d[5];                                 v[2] -= d[5];
      rho += d[6];   v[0] += d[6];  v[1] += d[6];
      rho += d[7];   v[0] += d[7];  v[1] -= d[7];
      rho += d[8];   v[0] += d[8];                 v[2] += d[8];
      rho += d[9];   v[0] += d[9];                 v[2] -= d[9];
      rho += d[10];  v[0] -= d[10]; v[1] += d[10];
      rho += d[11];  v[0] -= d[11]; v[1] -= d[11];
      rho += d[12];  v[0] -= d[12];                v[2] += d[12];
      rho += d[13];  v[0] -= d[13];                v[2] -= d[13];
      rho += d[14];                 v[1] += d[14]; v[2] += d[14];
      rho += d[15];                 v[1] += d[15]; v[2] -= d[15];
      rho += d[16];                 v[1] -= d[16]; v[2] += d[16];
      rho += d[17];                 v[1] -= d[17]; v[2] -= d[17];
      rho += d[18];

      // add contribution of gravity to momentum
      v[0] += 0.5*f[0];
      v[1] += 0.5*f[1];
      v[2] += 0.5*f[2];

      // convert momentum to velocity
      Pdf invRho = 1.0/rho;
      v[0] *= invRho;
      v[1] *= invRho;
      v[2] *= invRho;

      Pdf vv = _INVCS2_ * (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
      Pdf fv = _INVCS2_ * (f[0] * v[0] + f[1] * v[1] + f[2] * v[2]);

      Pdf cv, cf;

      cv = _INVCS2_ * v[0];
      cf = _INVCS2_ * f[0];
      d[0] = (1.0-_OMEGA_)*d[0] + _OMEGA_ * _19W0_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W0_ * (cf - fv + cv*cf);

      cv = -_INVCS2_ * v[0];
      cf = -_INVCS2_ * f[0];
      d[1] = (1.0-_OMEGA_)*d[1] + _OMEGA_ * _19W0_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W0_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * v[1];
      cf = _INVCS2_ * f[1];
      d[2] = (1.0-_OMEGA_)*d[2] + _OMEGA_ * _19W0_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W0_ * (cf - fv + cv*cf);

      cv = -_INVCS2_ * v[1];
      cf = -_INVCS2_ * f[1];
      d[3] = (1.0-_OMEGA_)*d[3] + _OMEGA_ * _19W0_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W0_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * v[2];
      cf = _INVCS2_ * f[2];
      d[4] = (1.0-_OMEGA_)*d[4] + _OMEGA_ * _19W0_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W0_ * (cf - fv + cv*cf);

      cv = -_INVCS2_ * v[2];
      cf = -_INVCS2_ * f[2];
      d[5] = (1.0-_OMEGA_)*d[5] + _OMEGA_ * _19W0_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W0_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (v[0]+v[1]);
      cf = _INVCS2_ * (f[0]+f[1]);
      d[6] = (1.0-_OMEGA_)*d[6] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (v[0]-v[1]);
      cf = _INVCS2_ * (f[0]-f[1]);
      d[7] = (1.0-_OMEGA_)*d[7] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (v[0]+v[2]);
      cf = _INVCS2_ * (f[0]-f[1]);
      d[8] = (1.0-_OMEGA_)*d[8] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (v[0]-v[2]);
      cf = _INVCS2_ * (f[0]-f[1]);
      d[9] = (1.0-_OMEGA_)*d[9] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (-v[0]+v[1]);
      cf = _INVCS2_ * (-f[0]+f[1]);
      d[10] = (1.0-_OMEGA_)*d[10] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (-v[0]-v[1]);
      cf = _INVCS2_ * (-f[0]-f[1]);
      d[11] = (1.0-_OMEGA_)*d[11] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (-v[0]+v[2]);
      cf = _INVCS2_ * (-f[0]+f[2]);
      d[12] = (1.0-_OMEGA_)*d[12] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (-v[0]-v[2]);
      cf = _INVCS2_ * (-f[0]-f[2]);
      d[13] = (1.0-_OMEGA_)*d[13] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (v[1]+v[2]);
      cf = _INVCS2_ * (f[1]+f[2]);
      d[14] = (1.0-_OMEGA_)*d[14] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (v[1]-v[2]);
      cf = _INVCS2_ * (f[1]-f[2]);
      d[15] = (1.0-_OMEGA_)*d[15] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (-v[1]+v[2]);
      cf = _INVCS2_ * (-f[1]+f[2]);
      d[16] = (1.0-_OMEGA_)*d[16] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (-v[1]-v[2]);
      cf = _INVCS2_ * (-f[1]-f[2]);
      d[17] = (1.0-_OMEGA_)*d[17] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      d[18] = (1.0-_OMEGA_)*d[18] + _OMEGA_ * _19W2_ * rho * (1.0 - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W2_ * (-fv);

      dstrb_tgt(nFluid * 0 + fluidIdx) = d[0];
      dstrb_tgt(nFluid * 1 + fluidIdx) = d[1];
      dstrb_tgt(nFluid * 2 + fluidIdx) = d[2];
      dstrb_tgt(nFluid * 3 + fluidIdx) = d[3];
      dstrb_tgt(nFluid * 4 + fluidIdx) = d[4];
      dstrb_tgt(nFluid * 5 + fluidIdx) = d[5];
      dstrb_tgt(nFluid * 6 + fluidIdx) = d[6];
      dstrb_tgt(nFluid * 7 + fluidIdx) = d[7];
      dstrb_tgt(nFluid * 8 + fluidIdx) = d[8];
      dstrb_tgt(nFluid * 9 + fluidIdx) = d[9];
      dstrb_tgt(nFluid * 10 + fluidIdx) = d[10];
      dstrb_tgt(nFluid * 11 + fluidIdx) = d[11];
      dstrb_tgt(nFluid * 12 + fluidIdx) = d[12];
      dstrb_tgt(nFluid * 13 + fluidIdx) = d[13];
      dstrb_tgt(nFluid * 14 + fluidIdx) = d[14];
      dstrb_tgt(nFluid * 15 + fluidIdx) = d[15];
      dstrb_tgt(nFluid * 16 + fluidIdx) = d[16];
      dstrb_tgt(nFluid * 17 + fluidIdx) = d[17];
      dstrb_tgt(nFluid * 18 + fluidIdx) = d[18];

   } );

   return;
}
#else
#ifdef USE_SYCL
void KernelAB_SOA_Unroll_Pull::timestepForce(Pdf* dstrb_src, Pdf* dstrb_tgt, int startIdx, int countIdx,sycl::queue q)
#else
void KernelAB_SOA_Unroll_Pull::timestepForce(Pdf* dstrb_src, Pdf* dstrb_tgt, int startIdx, int countIdx)
#endif
{
   int nFluid = geometry_.getNumFluidPts();
   const Pdf f[3] = {_GRAVITY_, 0.0, 0.0};
#ifdef USE_SYCL
      q.parallel_for(
#ifdef HAND
                   sycl::nd_range{sycl::range<1>{size_t(((countIdx + (wg_size- 1)) / wg_size) * wg_size)},sycl::range<1>{wg_size}},[=,adjacency_d_=this->geometry_.adjacency_d_](sycl::id<1> locIdx)
#else
                   sycl::range<1>{size_t(countIdx)},sycl::id<1>{size_t(startIdx)},[=,adjacency_d_=this->geometry_.adjacency_d_](sycl::id<1> fluidIdx)
#endif
		 
#else
      for (int fluidIdx=startIdx; fluidIdx<startIdx+countIdx; fluidIdx++)
#endif
      {
#if (defined(USE_SYCL) && defined(HAND))
      if(locIdx < countIdx){
#endif

      Pdf d[_LATTICESIZE_];
#ifdef USE_SYCL
#ifdef HAND
      auto fluidIdx=startIdx+locIdx;
#endif
      d[0] = dstrb_src[adjacency_d_[nFluid * 0 + fluidIdx]];
      d[1] = dstrb_src[adjacency_d_[nFluid * 1 + fluidIdx]];
      d[2] = dstrb_src[adjacency_d_[nFluid * 2 + fluidIdx]];
      d[3] = dstrb_src[adjacency_d_[nFluid * 3 + fluidIdx]];
      d[4] = dstrb_src[adjacency_d_[nFluid * 4 + fluidIdx]];
      d[5] = dstrb_src[adjacency_d_[nFluid * 5 + fluidIdx]];
      d[6] = dstrb_src[adjacency_d_[nFluid * 6 + fluidIdx]];
      d[7] = dstrb_src[adjacency_d_[nFluid * 7 + fluidIdx]];
      d[8] = dstrb_src[adjacency_d_[nFluid * 8 + fluidIdx]];
      d[9] = dstrb_src[adjacency_d_[nFluid * 9 + fluidIdx]];
      d[10] = dstrb_src[adjacency_d_[nFluid * 10 + fluidIdx]];
      d[11] = dstrb_src[adjacency_d_[nFluid * 11 + fluidIdx]];
      d[12] = dstrb_src[adjacency_d_[nFluid * 12 + fluidIdx]];
      d[13] = dstrb_src[adjacency_d_[nFluid * 13 + fluidIdx]];
      d[14] = dstrb_src[adjacency_d_[nFluid * 14 + fluidIdx]];
      d[15] = dstrb_src[adjacency_d_[nFluid * 15 + fluidIdx]];
      d[16] = dstrb_src[adjacency_d_[nFluid * 16 + fluidIdx]];
      d[17] = dstrb_src[adjacency_d_[nFluid * 17 + fluidIdx]];
#else
      d[0] = dstrb_src[geometry_.adjacency_[nFluid * 0 + fluidIdx]];
      d[1] = dstrb_src[geometry_.adjacency_[nFluid * 1 + fluidIdx]];
      d[2] = dstrb_src[geometry_.adjacency_[nFluid * 2 + fluidIdx]];
      d[3] = dstrb_src[geometry_.adjacency_[nFluid * 3 + fluidIdx]];
      d[4] = dstrb_src[geometry_.adjacency_[nFluid * 4 + fluidIdx]];
      d[5] = dstrb_src[geometry_.adjacency_[nFluid * 5 + fluidIdx]];
      d[6] = dstrb_src[geometry_.adjacency_[nFluid * 6 + fluidIdx]];
      d[7] = dstrb_src[geometry_.adjacency_[nFluid * 7 + fluidIdx]];
      d[8] = dstrb_src[geometry_.adjacency_[nFluid * 8 + fluidIdx]];
      d[9] = dstrb_src[geometry_.adjacency_[nFluid * 9 + fluidIdx]];
      d[10] = dstrb_src[geometry_.adjacency_[nFluid * 10 + fluidIdx]];
      d[11] = dstrb_src[geometry_.adjacency_[nFluid * 11 + fluidIdx]];
      d[12] = dstrb_src[geometry_.adjacency_[nFluid * 12 + fluidIdx]];
      d[13] = dstrb_src[geometry_.adjacency_[nFluid * 13 + fluidIdx]];
      d[14] = dstrb_src[geometry_.adjacency_[nFluid * 14 + fluidIdx]];
      d[15] = dstrb_src[geometry_.adjacency_[nFluid * 15 + fluidIdx]];
      d[16] = dstrb_src[geometry_.adjacency_[nFluid * 16 + fluidIdx]];
      d[17] = dstrb_src[geometry_.adjacency_[nFluid * 17 + fluidIdx]];
#endif
      d[18] = dstrb_src[nFluid * 18 + fluidIdx];

      Pdf rho = 0.0;
      Pdf v[3] = {0.0, 0.0, 0.0};

      rho += d[0];   v[0] += d[0];
      rho += d[1];   v[0] -= d[1];
      rho += d[2];                  v[1] += d[2];
      rho += d[3];                  v[1] -= d[3];
      rho += d[4];                                 v[2] += d[4];
      rho += d[5];                                 v[2] -= d[5];
      rho += d[6];   v[0] += d[6];  v[1] += d[6];
      rho += d[7];   v[0] += d[7];  v[1] -= d[7];
      rho += d[8];   v[0] += d[8];                 v[2] += d[8];
      rho += d[9];   v[0] += d[9];                 v[2] -= d[9];
      rho += d[10];  v[0] -= d[10]; v[1] += d[10];
      rho += d[11];  v[0] -= d[11]; v[1] -= d[11];
      rho += d[12];  v[0] -= d[12];                v[2] += d[12];
      rho += d[13];  v[0] -= d[13];                v[2] -= d[13];
      rho += d[14];                 v[1] += d[14]; v[2] += d[14];
      rho += d[15];                 v[1] += d[15]; v[2] -= d[15];
      rho += d[16];                 v[1] -= d[16]; v[2] += d[16];
      rho += d[17];                 v[1] -= d[17]; v[2] -= d[17];
      rho += d[18];

      // add contribution of gravity to momentum
      v[0] += 0.5*f[0];
      v[1] += 0.5*f[1];
      v[2] += 0.5*f[2];

      // convert momentum to velocity
      Pdf invRho = 1.0/rho;
      v[0] *= invRho;
      v[1] *= invRho;
      v[2] *= invRho;

      Pdf vv = _INVCS2_ * (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
      Pdf fv = _INVCS2_ * (f[0] * v[0] + f[1] * v[1] + f[2] * v[2]);

      Pdf cv, cf;

      cv = _INVCS2_ * v[0];
      cf = _INVCS2_ * f[0];
      d[0] = (1.0-_OMEGA_)*d[0] + _OMEGA_ * _19W0_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W0_ * (cf - fv + cv*cf);

      cv = -_INVCS2_ * v[0];
      cf = -_INVCS2_ * f[0];
      d[1] = (1.0-_OMEGA_)*d[1] + _OMEGA_ * _19W0_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W0_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * v[1];
      cf = _INVCS2_ * f[1];
      d[2] = (1.0-_OMEGA_)*d[2] + _OMEGA_ * _19W0_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W0_ * (cf - fv + cv*cf);

      cv = -_INVCS2_ * v[1];
      cf = -_INVCS2_ * f[1];
      d[3] = (1.0-_OMEGA_)*d[3] + _OMEGA_ * _19W0_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W0_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * v[2];
      cf = _INVCS2_ * f[2];
      d[4] = (1.0-_OMEGA_)*d[4] + _OMEGA_ * _19W0_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W0_ * (cf - fv + cv*cf);

      cv = -_INVCS2_ * v[2];
      cf = -_INVCS2_ * f[2];
      d[5] = (1.0-_OMEGA_)*d[5] + _OMEGA_ * _19W0_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W0_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (v[0]+v[1]);
      cf = _INVCS2_ * (f[0]+f[1]);
      d[6] = (1.0-_OMEGA_)*d[6] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (v[0]-v[1]);
      cf = _INVCS2_ * (f[0]-f[1]);
      d[7] = (1.0-_OMEGA_)*d[7] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (v[0]+v[2]);
      cf = _INVCS2_ * (f[0]-f[1]);
      d[8] = (1.0-_OMEGA_)*d[8] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (v[0]-v[2]);
      cf = _INVCS2_ * (f[0]-f[1]);
      d[9] = (1.0-_OMEGA_)*d[9] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (-v[0]+v[1]);
      cf = _INVCS2_ * (-f[0]+f[1]);
      d[10] = (1.0-_OMEGA_)*d[10] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (-v[0]-v[1]);
      cf = _INVCS2_ * (-f[0]-f[1]);
      d[11] = (1.0-_OMEGA_)*d[11] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (-v[0]+v[2]);
      cf = _INVCS2_ * (-f[0]+f[2]);
      d[12] = (1.0-_OMEGA_)*d[12] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (-v[0]-v[2]);
      cf = _INVCS2_ * (-f[0]-f[2]);
      d[13] = (1.0-_OMEGA_)*d[13] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (v[1]+v[2]);
      cf = _INVCS2_ * (f[1]+f[2]);
      d[14] = (1.0-_OMEGA_)*d[14] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (v[1]-v[2]);
      cf = _INVCS2_ * (f[1]-f[2]);
      d[15] = (1.0-_OMEGA_)*d[15] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (-v[1]+v[2]);
      cf = _INVCS2_ * (-f[1]+f[2]);
      d[16] = (1.0-_OMEGA_)*d[16] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      cv = _INVCS2_ * (-v[1]-v[2]);
      cf = _INVCS2_ * (-f[1]-f[2]);
      d[17] = (1.0-_OMEGA_)*d[17] + _OMEGA_ * _19W1_ * rho * (1.0 + cv + 0.5 * cv * cv - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W1_ * (cf - fv + cv*cf);

      d[18] = (1.0-_OMEGA_)*d[18] + _OMEGA_ * _19W2_ * rho * (1.0 - 0.5 * vv) + (1.0-0.5*_OMEGA_) * _19W2_ * (-fv);

      dstrb_tgt[nFluid * 0 + fluidIdx] = d[0];
      dstrb_tgt[nFluid * 1 + fluidIdx] = d[1];
      dstrb_tgt[nFluid * 2 + fluidIdx] = d[2];
      dstrb_tgt[nFluid * 3 + fluidIdx] = d[3];
      dstrb_tgt[nFluid * 4 + fluidIdx] = d[4];
      dstrb_tgt[nFluid * 5 + fluidIdx] = d[5];
      dstrb_tgt[nFluid * 6 + fluidIdx] = d[6];
      dstrb_tgt[nFluid * 7 + fluidIdx] = d[7];
      dstrb_tgt[nFluid * 8 + fluidIdx] = d[8];
      dstrb_tgt[nFluid * 9 + fluidIdx] = d[9];
      dstrb_tgt[nFluid * 10 + fluidIdx] = d[10];
      dstrb_tgt[nFluid * 11 + fluidIdx] = d[11];
      dstrb_tgt[nFluid * 12 + fluidIdx] = d[12];
      dstrb_tgt[nFluid * 13 + fluidIdx] = d[13];
      dstrb_tgt[nFluid * 14 + fluidIdx] = d[14];
      dstrb_tgt[nFluid * 15 + fluidIdx] = d[15];
      dstrb_tgt[nFluid * 16 + fluidIdx] = d[16];
      dstrb_tgt[nFluid * 17 + fluidIdx] = d[17];
      dstrb_tgt[nFluid * 18 + fluidIdx] = d[18];

   }
#ifdef USE_SYCL
#ifdef HAND
   }
#endif
);
#endif

   return;
}
#endif
/******************************************************************************/
