// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.
#ifndef HEMELB_LB_STREAMERS_NASHZEROTHORDERPRESSUREDELEGATE_H
#define HEMELB_LB_STREAMERS_NASHZEROTHORDERPRESSUREDELEGATE_H

#include "util/utilityFunctions.h"
#include "lb/streamers/LinkStreamer.h"

namespace hemelb::lb::streamers
{
    template<typename CollisionImpl>
    class NashZerothOrderPressureDelegate
    {
    public:
        using CollisionType = CollisionImpl;
        using LatticeType = typename CollisionType::CKernel::LatticeType;

          NashZerothOrderPressureDelegate(CollisionType& delegatorCollider,
                                          InitParams& initParams) :
              collider(delegatorCollider), iolet(*initParams.boundaryObject)
          {
          }

        void StreamLink(const LbmParameters* lbmParams,
                        geometry::FieldData& latticeData,
                        const geometry::Site<geometry::FieldData>& site,
                        HydroVars<typename CollisionType::CKernel>& hydroVars,
                        const Direction& direction)
        {
            int boundaryId = site.GetIoletId();

            // Set the density at the "ghost" site to be the density of the iolet.
            distribn_t ghostDensity = iolet.GetBoundaryDensity(boundaryId);

            // Calculate the velocity at the ghost site, as the component normal to the iolet.
            auto ioletNormal = iolet.GetLocalIolet(boundaryId)->GetNormal().template as<float>();

            // Note that the division by density compensates for the fact that v_x etc have momentum
            // not velocity.
            distribn_t component = Dot(hydroVars.momentum, ioletNormal) / hydroVars.density;

            // TODO it's ugly that we have to do this.
            // TODO having to give 0 as an argument is also ugly.
            // TODO it's ugly that we have to give hydroVars a nonsense distribution vector
            // that doesn't get used.
            HydroVars<typename CollisionType::CKernel> ghostHydrovars(site);

            ghostHydrovars.density = ghostDensity;
            ghostHydrovars.momentum = ioletNormal * component * ghostDensity;

            collider.kernel.CalculateFeq(ghostHydrovars, 0);

            Direction unstreamed = LatticeType::INVERSEDIRECTIONS[direction];

            *latticeData.GetFNew(site.GetIndex() * LatticeType::NUMVECTORS + unstreamed) =
                ghostHydrovars.GetFEq()[unstreamed];
        }

        void PostStepLink(geometry::FieldData& latticeData,
                          const geometry::Site<geometry::FieldData>& site,
                          const Direction& direction)
        {
            // Nothing to do :)
        }

    protected:
        CollisionType& collider;
        BoundaryValues& iolet;
    };
}

#endif // HEMELB_LB_STREAMERS_NASHZEROTHORDERPRESSUREDELEGATE_H
