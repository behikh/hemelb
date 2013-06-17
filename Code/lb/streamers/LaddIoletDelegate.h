// 
// Copyright (C) University College London, 2007-2012, all rights reserved.
// 
// This file is part of HemeLB and is CONFIDENTIAL. You may not work 
// with, install, use, duplicate, modify, redistribute or share this
// file, or any part thereof, other than as allowed by any agreement
// specifically made by you with University College London.
// 

#ifndef HEMELB_LB_STREAMERS_LADDIOLETDELEGATE_H
#define HEMELB_LB_STREAMERS_LADDIOLETDELEGATE_H

#include "lb/streamers/SimpleBounceBackDelegate.h"

namespace hemelb
{
  namespace lb
  {
    namespace streamers
    {

      template<typename CollisionImpl>
      class LaddIoletDelegate : public SimpleBounceBackDelegate<CollisionImpl>
      {
        public:
          typedef CollisionImpl CollisionType;
          typedef typename CollisionType::CKernel::LatticeType LatticeType;

          LaddIoletDelegate(CollisionType& delegatorCollider, kernels::InitParams& initParams) :
            SimpleBounceBackDelegate<CollisionType> (delegatorCollider, initParams), bValues(initParams.boundaryObject)
          {
          }

          inline void StreamLink(const LbmParameters* lbmParams,
                                 geometry::LatticeData* const latticeData,
                                 const geometry::Site<geometry::LatticeData>& site,
                                 kernels::HydroVars<typename CollisionType::CKernel>& hydroVars,
                                 const Direction& ii)
          {
            // Translating from Ladd, J. Fluid Mech. "Numerical simulations
            // of particulate suspensions via a discretized Boltzmann
            // equation. Part 1. Theoretical foundation", 1994
            // Eq (3.2) -- simple bounce-back -- becomes:
            //   f_i'(r, t+1) = f_i(r, t*)
            // Eq (3.3) --- modified BB -- becomes:
            //   f_i'(r, t+1) = f_i(r, t*) - 2 a1_i \rho u . c_i
            // where u is the velocity of the boundary half way along the
            // link and a1_i = w_1 / cs2

            int boundaryId = site.GetIoletId();
            iolets::InOutLetParabolicVelocity* iolet =
                dynamic_cast<iolets::InOutLetParabolicVelocity*> (bValues->GetLocalIolet(boundaryId));
            LatticePosition sitePos(site.GetGlobalSiteCoords());

            LatticePosition halfWay(sitePos);
            halfWay.x += 0.5 * LatticeType::CX[ii];
            halfWay.y += 0.5 * LatticeType::CY[ii];
            halfWay.z += 0.5 * LatticeType::CZ[ii];

            LatticeVelocity wallMom(iolet->GetVelocity(halfWay, bValues->GetTimeStep()));

            if (CollisionType::CKernel::LatticeType::IsLatticeCompressible())
            {
              wallMom *= hydroVars.density;
            }

            distribn_t correction = 2. * LatticeType::EQMWEIGHTS[ii] * (wallMom.x
                * LatticeType::CX[ii] + wallMom.y * LatticeType::CY[ii] + wallMom.z * LatticeType::CZ[ii]) / Cs2;

            * (latticeData->GetFNew(SimpleBounceBackDelegate<CollisionImpl>::GetBBIndex(site.GetIndex(), ii)))
                = hydroVars.GetFPostCollision()[ii] - correction;
          }
        private:
          iolets::BoundaryValues* bValues;
      };

    }
  }
}

#endif /* HEMELB_LB_STREAMERS_LADDIOLETDELEGATE_H */