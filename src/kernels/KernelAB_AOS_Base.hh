#ifndef KERNEL_AB_AOS_BASE_H
#define KERNEL_AB_AOS_BASE_H



#include "KernelAB.hh"

class KernelAB_AOS_Base : public KernelAB
{
   public:
#ifdef USE_SYCL
	KernelAB_AOS_Base(Lattice& lattice, Geometry& geometry,sycl::queue& q): KernelAB(lattice, geometry,q) {};
#else
      KernelAB_AOS_Base(Lattice& lattice, Geometry& geometry): KernelAB(lattice, geometry) {};
#endif
      ~KernelAB_AOS_Base(void) {};

      void setup();
      
#ifdef USE_KOKKOS
   
      void timestepForce(myViewPDF dstrb_src, myViewPDF dstrb_tgt, int startIdx, int countIdx);
#else
    
      void timestepForce(Pdf* dstrb_src, Pdf* dstrb_tgt, int startIdx, int countIdx);
#endif

   protected:



   private:



};
#endif
