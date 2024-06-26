// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_LB_STREAMERS_VIRTUALSITE_H
#define HEMELB_LB_STREAMERS_VIRTUALSITE_H

#include <vector>

#include <boost/container/flat_map.hpp>

#include "units.h"
#include "util/Vector3D.h"
#include "lb/iolets/InOutLet.h"
#include "lb/lattices/LatticeInfo.h"

namespace hemelb::lb
{

    // Hydrodynamic variables at real sites needed to compute virtual site data.
    struct RSHV
    {
        using Map = boost::container::flat_map<site_t, RSHV>;
        // Time step at which this was last updated
        LatticeTimeStep t;
        // Density at that time
        LatticeDensity rho;
        // Velocity at that time
        LatticeVelocity u;
        // Position of the site in iolet coordinates
        LatticePosition posIolet;
    };

    // Hydrodynamic variables at virtual sites.
    template<lattice_type LatticeType>
    struct VSHV : public RSHV
    {
        typename LatticeType::FArray fPostColl;
    };

    // Forward declaration
    template<class LatticeType>
    class VirtualSite;

    // Extra data attached to each Iolet to enable virtual site BCs.
    template<class LatticeType>
    class VSExtra : public IoletExtraData
    {
    public:
        VSExtra(InOutLet& iolet) :
                IoletExtraData(iolet)
        {

        }
        // TODO: is this correct?
        typename VirtualSite<LatticeType>::Map vSites;
        RSHV::Map hydroVarsCache;
    };

    template<class LatticeType>
    class VirtualSite
    {
    public:
        // TODO: why not a flat_map?
        using Map =  std::map<site_t, VirtualSite<LatticeType> >;

        std::vector<Direction> neighbourDirections;
        std::vector<site_t> neighbourGlobalIds;

        std::vector<LatticeDistance> q;
        distribn_t sumQiSq;
        distribn_t velocityMatrixInv[3][3];
        VSHV<LatticeType> hv;

        // TODO: check why not default
        VirtualSite& operator=(const VirtualSite& rhs)
        {
            neighbourDirections = rhs.neighbourDirections;
            neighbourGlobalIds = rhs.neighbourGlobalIds;
            q = rhs.q;
            sumQiSq = rhs.sumQiSq;
            for (unsigned i = 0; i < 3; ++i)
                for (unsigned j = 0; j < 3; ++j)
                    velocityMatrixInv[i][j] = rhs.velocityMatrixInv[i][j];
            hv = rhs.hv;
            return *this;
        }

        VirtualSite(const VirtualSite& rhs)
        {
            *this = rhs;
        }

        VirtualSite(InitParams& initParams, VSExtra<LatticeType>& extra,
                    const LatticeVector& location) :
                sumQiSq(0.)
        {
            hv.t = 0;
            hv.rho = 1.;
            hv.u = LatticeVelocity::Zero();
            hv.posIolet = extra.WorldToIolet(location);

            auto& lattice = LatticeType::GetLatticeInfo();

            distribn_t velocityMatrix[3][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };

            // For each site index in the neighbourhood
            for (Direction i = 0; i < LatticeType::NUMVECTORS; ++i)
            {
                hv.fPostColl[i] = 0.0;
                const LatticeVector neighbourLocation = location + LatticeType::VECTORS[i];
                site_t neighGlobalIdx;
                // site_t neighLocalIdx;
                proc_t neighbourSiteHomeProc;
                // Figure out if neighbour is fluid.
                bool isNeighbourFluid = false;
                // Check if neighbour is within our bounding box
                if (neighbourLocation.IsInRange(initParams.latDat->GetGlobalSiteMins(),
                                                initParams.latDat->GetGlobalSiteMaxes()))
                {
                    neighGlobalIdx
                            = initParams.latDat->GetGlobalNoncontiguousSiteIdFromGlobalCoords(neighbourLocation);
                    neighbourSiteHomeProc
                            = initParams.latDat->GetProcIdFromGlobalCoords(neighbourLocation);

                    // solid site isn't stored
                    if (neighbourSiteHomeProc != SITE_OR_BLOCK_SOLID)
                        isNeighbourFluid = true;
                }

                if (!isNeighbourFluid)
                {
                    continue;
                }
                else
                {
                    // The neighbour is fluid, but does it have an iolet intersection
                    // in this direction? If not, we must skip it for consistency.

                    // TODO: this really must cope with neigh being off process.
                    if (neighbourSiteHomeProc == initParams.latDat->GetLocalRank())
                    {
                        site_t neighLocalIdx =
                                initParams.latDat->GetLocalContiguousIdFromGlobalNoncontiguousId(neighGlobalIdx);
                        const geometry::Site<const geometry::Domain> neigh =
                                initParams.latDat->GetSite(neighLocalIdx);
                        unsigned inverse = lattice.GetInverseIndex(i);
                        if (!neigh.HasIolet(inverse))
                            continue;
                    }
                    else
                    {
                        throw Exception() << "Need off core site data for VirtualSite neighbour.";
                    }
                }

                // It's fluid, so add to the vSite's neighbour list
                neighbourDirections.push_back(i);
                neighbourGlobalIds.push_back(neighGlobalIdx);

                {
                    LatticePosition xIolet = extra.WorldToIolet(neighbourLocation);

                    // Ensure there's an entry in the hydroVars cache for the site.
                    RSHV::Map::iterator neighPtr = extra.hydroVarsCache.find(neighGlobalIdx);
                    if (neighPtr == extra.hydroVarsCache.end())
                    {
                        RSHV neighHV;
                        neighHV.t = 0;
                        neighHV.rho = 1.0;
                        neighHV.u = LatticeVelocity::Zero();
                        neighHV.posIolet = xIolet;
                        extra.hydroVarsCache.insert(RSHV::Map::value_type(neighGlobalIdx, neighHV));
                    }

                    // Add this site's contribution to the velocity matrix.
                    // Outer product of [x,y,1] . [x,y,1]
                    xIolet.z() = 1;
                    for (int a = 0; a < 3; ++a)
                        for (int b = 0; b < 3; ++b)
                            velocityMatrix[a][b] += xIolet[a] * xIolet[b];
                }


                if (neighbourSiteHomeProc == initParams.latDat->GetLocalRank())
                {
                    // Store the cut distance from neigh to the iolet
                    // I.e along the direction opposite to i
                    site_t neighLocalIdx =
                            initParams.latDat->GetLocalContiguousIdFromGlobalNoncontiguousId(neighGlobalIdx);
                    const geometry::Site<const geometry::Domain> neigh =
                            initParams.latDat->GetSite(neighLocalIdx);
                    AddQ(neigh.GetWallDistance<LatticeType>(lattice.GetInverseIndex(i)));

                    // Local sites don't need communication, so we're done here.
                }
                else
                {
                    // Create a requirements with the info we need.
                    geometry::neighbouring::RequiredSiteInformation requirements(false);

                    requirements.Require(geometry::neighbouring::terms::Density);
                    requirements.Require(geometry::neighbouring::terms::Velocity);

                    initParams.neighbouringDataManager->RegisterNeededSite(neighGlobalIdx,
                                                                           requirements);
                    // Store the cut distance from neigh to the iolet
                    // I.e along the direction opposite to i
                    AddQ(initParams.latDat->GetNeighbouringData().GetCutDistance<LatticeType>(neighGlobalIdx,
                                                                                              lattice.GetInverseIndex(i)));
                }
            }

            distribn_t det = Matrix3DInverse(velocityMatrix, velocityMatrixInv);
            if (det * det < 1e-6)
            {
                // Matrix was close to singular, choose an inverse that instead
                // of fitting, just takes the average velocity.
                for (unsigned i = 0; i < 3; ++i)
                    for (unsigned j = 0; j < 3; ++j)
                        velocityMatrixInv[i][j] = 0.;
                velocityMatrixInv[2][2] = 1.0 / velocityMatrix[2][2];
            }
        }

        static distribn_t Matrix3DInverse(const distribn_t m[3][3], distribn_t out[3][3])
        {
            // c => cofactor
            distribn_t c00 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
            distribn_t c01 = m[2][0] * m[1][2] - m[1][0] * m[2][2];
            distribn_t c02 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

            distribn_t det = m[0][0] * c00 + m[0][1] * c01 + m[0][2] * c02;

            out[0][0] = c00 / det;
            out[0][1] = /* c10 / det */(m[2][1] * m[0][2] - m[0][1] * m[2][2]) / det;
            out[0][2] = /* c20 / det */(m[0][1] * m[1][2] - m[1][1] * m[0][2]) / det;

            out[1][0] = c01 / det;
            out[1][1] = /* c11 / det */(m[0][0] * m[2][2] - m[2][0] * m[0][2]) / det;
            out[1][2] = /* c21 / det */(m[1][0] * m[0][2] - m[0][0] * m[1][2]) / det;

            out[2][0] = c02 / det;
            out[2][1] = /* c12 / det */(m[2][0] * m[0][1] - m[0][0] * m[2][1]) / det;
            out[2][2] = /* c22 / det */(m[0][0] * m[1][1] - m[1][0] * m[0][1]) / det;

            return det;
        }

        void AddQ(LatticeDistance qNew)
        {
            q.push_back(qNew);
            sumQiSq += qNew * qNew;
        }

    };
}

#endif // HEMELB_LB_STREAMERS_VIRTUALSITE_H
