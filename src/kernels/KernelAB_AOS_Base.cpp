
#include <iostream>


#include "KernelAB_AOS_Base.hh"

/******************************************************************************/
void KernelAB_AOS_Base::setup()
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
void KernelAB_AOS_Base::timestepForce(myViewPDF dstrb_src, myViewPDF dstrb_tgt, int startIdx, int countIdx)
{
   auto this_adjacency_d_=geometry_.adjacency_d_;
   
   Kokkos::parallel_for(myPolicy(startIdx, startIdx+countIdx),KOKKOS_CLASS_LAMBDA (const int fluidIdx)
   {
	Pdf f[3] = {_GRAVITY_, 0.0, 0.0};

      // compute rho, momentum
      Pdf rho = 0.0;
      Pdf v[3] = {0.0, 0.0, 0.0};
      for (int stencilIdx = 0; stencilIdx < _LATTICESIZE_; stencilIdx++)
      {
         rho += dstrb_src(_LATTICESIZE_ * fluidIdx + stencilIdx);
         v[0] += dstrb_src(_LATTICESIZE_ * fluidIdx + stencilIdx) * stencil_d_(_DIMS_*stencilIdx+0);
         v[1] += dstrb_src(_LATTICESIZE_ * fluidIdx + stencilIdx) * stencil_d_(_DIMS_*stencilIdx+1);
         v[2] += dstrb_src(_LATTICESIZE_ * fluidIdx + stencilIdx) * stencil_d_(_DIMS_*stencilIdx+2);
      }

      // add contribution of gravity to momentum
      v[0] += 0.5*f[0];
      v[1] += 0.5*f[1];
      v[2] += 0.5*f[2];

      // convert momentum to velocity
      Pdf invRho = 1.0/rho;
      v[0] *= invRho;
      v[1] *= invRho;
      v[2] *= invRho;

      Pdf vdotv = _INVCS2_ * (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
      Pdf fdotv = _INVCS2_ * (f[0] * v[0] + f[1] * v[1] + f[2] * v[2]);

      for (int stencilIdx = 0; stencilIdx < _LATTICESIZE_; stencilIdx++)
      {
         Pdf cdotv = _INVCS2_ * (stencil_d_(_DIMS_*stencilIdx+0) * v[0] +
                                 stencil_d_(_DIMS_*stencilIdx+1) * v[1] +
                                 stencil_d_(_DIMS_*stencilIdx+2) * v[2] );
         Pdf cdotf = _INVCS2_ * (stencil_d_(_DIMS_*stencilIdx+0) * f[0] +
                                 stencil_d_(_DIMS_*stencilIdx+1) * f[1] +
                                 stencil_d_(_DIMS_*stencilIdx+2) * f[2] );
         Pdf eq = weight_d_(stencilIdx) * rho * (1.0 + cdotv + 0.5 * cdotv * cdotv - 0.5 * vdotv);
         Pdf fdist = weight_d_(stencilIdx) * (cdotf - fdotv + cdotv * cdotf);
         int streamLoc = this_adjacency_d_(_LATTICESIZE_ * fluidIdx + stencilIdx);
         dstrb_tgt(streamLoc) =
            (1.0-_OMEGA_) * dstrb_src(_LATTICESIZE_ * fluidIdx + stencilIdx) +
            _OMEGA_ * eq +
            (1.0-0.5*_OMEGA_) * fdist;
      }
   } );

   return;
}
#else
#ifdef USE_SYCL
void KernelAB_AOS_Base::timestepForce(Pdf* dstrb_src, Pdf* dstrb_tgt, int startIdx, int countIdx,sycl::queue q)
#else
void KernelAB_AOS_Base::timestepForce(Pdf* dstrb_src, Pdf* dstrb_tgt, int startIdx, int countIdx)
#endif
{
   Pdf f[3] = {_GRAVITY_, 0.0, 0.0};
#ifdef USE_SYCL
q.parallel_for(sycl::range<1>{size_t(countIdx)},sycl::id<1>{size_t(startIdx)},[=,adjacency_d_=this->geometry_.adjacency_d_,stencil_d_=this->stencil_d_,weight_d_=this->weight_d_](sycl::id<1> fluidIdx)
#else
   for (int fluidIdx=startIdx; fluidIdx<startIdx+countIdx; fluidIdx++)
#endif
   {
      // compute rho, momentum
      Pdf rho = 0.0;
      Pdf v[3] = {0.0, 0.0, 0.0};
      for (int stencilIdx = 0; stencilIdx < _LATTICESIZE_; stencilIdx++)
      {
         rho += dstrb_src[_LATTICESIZE_ * fluidIdx + stencilIdx];
#ifdef USE_SYCL
         v[0] += dstrb_src[_LATTICESIZE_ * fluidIdx + stencilIdx] * stencil_d_[_DIMS_*stencilIdx+0];
         v[1] += dstrb_src[_LATTICESIZE_ * fluidIdx + stencilIdx] * stencil_d_[_DIMS_*stencilIdx+1];
         v[2] += dstrb_src[_LATTICESIZE_ * fluidIdx + stencilIdx] * stencil_d_[_DIMS_*stencilIdx+2];
#else        
         v[0] += dstrb_src[_LATTICESIZE_ * fluidIdx + stencilIdx] * stencil_[_DIMS_*stencilIdx+0];
         v[1] += dstrb_src[_LATTICESIZE_ * fluidIdx + stencilIdx] * stencil_[_DIMS_*stencilIdx+1];
         v[2] += dstrb_src[_LATTICESIZE_ * fluidIdx + stencilIdx] * stencil_[_DIMS_*stencilIdx+2];
#endif
      }

      // add contribution of gravity to momentum
      v[0] += 0.5*f[0];
      v[1] += 0.5*f[1];
      v[2] += 0.5*f[2];

      // convert momentum to velocity
      Pdf invRho = 1.0/rho;
      v[0] *= invRho;
      v[1] *= invRho;
      v[2] *= invRho;

      Pdf vdotv = _INVCS2_ * (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
      Pdf fdotv = _INVCS2_ * (f[0] * v[0] + f[1] * v[1] + f[2] * v[2]);

      for (int stencilIdx = 0; stencilIdx < _LATTICESIZE_; stencilIdx++)
      {
#ifdef USE_SYCL
		Pdf cdotv = _INVCS2_ * (stencil_d_[_DIMS_*stencilIdx+0] * v[0] +
                                 stencil_d_[_DIMS_*stencilIdx+1] * v[1] +
                                 stencil_d_[_DIMS_*stencilIdx+2] * v[2] );
         Pdf cdotf = _INVCS2_ * (stencil_d_[_DIMS_*stencilIdx+0] * f[0] +
                                 stencil_d_[_DIMS_*stencilIdx+1] * f[1] +
                                 stencil_d_[_DIMS_*stencilIdx+2] * f[2] );
         Pdf eq = weight_d_[stencilIdx] * rho * (1.0 + cdotv + 0.5 * cdotv * cdotv - 0.5 * vdotv);
         Pdf fdist = weight_d_[stencilIdx] * (cdotf - fdotv + cdotv * cdotf);
         int streamLoc = adjacency_d_[_LATTICESIZE_ * fluidIdx + stencilIdx];
#else
         Pdf cdotv = _INVCS2_ * (stencil_[_DIMS_*stencilIdx+0] * v[0] +
                                 stencil_[_DIMS_*stencilIdx+1] * v[1] +
                                 stencil_[_DIMS_*stencilIdx+2] * v[2] );
         Pdf cdotf = _INVCS2_ * (stencil_[_DIMS_*stencilIdx+0] * f[0] +
                                 stencil_[_DIMS_*stencilIdx+1] * f[1] +
                                 stencil_[_DIMS_*stencilIdx+2] * f[2] );
         Pdf eq = weight_[stencilIdx] * rho * (1.0 + cdotv + 0.5 * cdotv * cdotv - 0.5 * vdotv);
         Pdf fdist = weight_[stencilIdx] * (cdotf - fdotv + cdotv * cdotf);
         int streamLoc = geometry_.adjacency_[_LATTICESIZE_ * fluidIdx + stencilIdx];
#endif
         dstrb_tgt[streamLoc] =
            (1.0-_OMEGA_) * dstrb_src[_LATTICESIZE_ * fluidIdx + stencilIdx] +
            _OMEGA_ * eq +
            (1.0-0.5*_OMEGA_) * fdist;
      }
   }
#ifdef USE_SYCL
);
#endif

   return;
}
#endif
/******************************************************************************/
