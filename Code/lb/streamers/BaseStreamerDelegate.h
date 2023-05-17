// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_LB_STREAMERS_BASESTREAMERDELEGATE_H
#define HEMELB_LB_STREAMERS_BASESTREAMERDELEGATE_H

#include "geometry/Domain.h"
#include "lb/kernels/BaseKernel.h"

namespace hemelb::lb::streamers
{
      /**
       * Base class for Streamer delegates.
       *
       * Sets out the interface that streamers should implement and typedefs
       * that they should make.
       *
       * Unfortunately C++ template rules cause a difficulty: since the base
       * class is a dependent name (i.e. only know on instantiation) and
       * unqualified names are assumed to be non-dependent we cannot use
       * CollisionType and LatticeType in subclasses without using the (rather
       * lond) name of the appropriate instantiation of this template.
       *
       * Subclasses should therefore redeclare CollisionType and LatticeType
       * for their convenience.
       *
       * Since the methods that do the work are NOT polymorphic (and marked as
       * inline) the compiler can inline them quite straightforwardly---I
       * checked the assembly output on gcc with -O2 and this is in fact
       * done. The overhead should be pretty low.
       *
       */
      template<class CollisionImpl>
      class BaseStreamerDelegate
      {
        public:
          using CollisionType = CollisionImpl;
          using LatticeType = typename CollisionType::CKernel::LatticeType;

        protected:
          /**
           * Protected default ctor to make life a bit easier for subclasses.
           */
          BaseStreamerDelegate() = default;

        public:
          /**
           * Perform the streaming operation from site along direction
           *
           * hydroVars must be post-collision
           *
           * @param latticeData
           * @param site
           * @param hydroVars
           * @param direction
           */
          void StreamLink(const LbmParameters* lbmParams,
                                 geometry::FieldData& latticeData,
                                 const geometry::Site<geometry::Domain>& site,
                                 HydroVars<typename CollisionType::CKernel>& hydroVars,
                                 const Direction& direction)
          {
          }

          /**
           * Perform any post-step operations for the link from site along direction
           *
           * @param latticeData
           * @param site
           * @param direction
           */
          void PostStepLink(geometry::FieldData& latticeData,
                                   const geometry::Site<geometry::FieldData>& site,
                                   const Direction& direction)
          {
          }
      };

}

#endif /* HEMELB_LB_STREAMERS_BASESTREAMERDELEGATE_H */
