// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_TESTS_HELPERS_LATTICEDATAACCESS_H
#define HEMELB_TESTS_HELPERS_LATTICEDATAACCESS_H

#include <algorithm>
#include <functional>
#include "geometry/Domain.h"
#include "geometry/FieldData.h"
#include "lb/lattices/D3Q15.h"

namespace hemelb::tests::helpers
{

    // Empty FOld distribution
    void zeroOutFOld(geometry::Domain * const latDat);

    // FNew at given site
    template<class Lattice>
    distribn_t const * GetFNew(geometry::FieldData& latDat, LatticeVector const &_pos);
    distribn_t const * GetFNew(geometry::FieldData& latDat, site_t const &index);

    // Population i set to some distribution
    template<class LATTICE>
    void setUpDistribution(geometry::Domain *latDat, size_t _i,
                           std::function<Dimensionless(PhysicalVelocity const &)> _function);

    // Class to setup/manipulate lattice data
    class LatticeDataAccess
    {
    public:
        LatticeDataAccess(geometry::FieldData* const latDat);

        // Empty distribution
        void ZeroOutFOld() const;
        // Empty Forces
        void ZeroOutForces() const;
        // Set FOld for a given site and direction
        template<class LATTICE>
        void SetFOld(site_t _x, site_t _y, site_t _z, site_t _dir, distribn_t _value) const
        {
            SetFOld<LATTICE>(LatticeVector(_x, _y, _z), _dir, _value);
        }
        template<class LATTICE>
        void SetFOld(LatticeVector const &_pos, site_t _dir, distribn_t _value) const;

        // Get FNew for a given site and direction
        template<class LATTICE>
        distribn_t const * GetFNew(site_t _x, site_t _y, site_t _z) const
        {
            return GetFNew<LATTICE>(LatticeVector(_x, _y, _z));
        }
        template<class LATTICE>
        distribn_t const * GetFNew(LatticeVector const &_pos) const;
        distribn_t const * GetFNew(site_t index) const;

        void SetMinWallDistance(PhysicalDistance _mindist);
        void SetWallDistance(PhysicalDistance _mindist);

        // Sets up linear distribution f_i(coeffs) = (1, x, y, z) * coeffs
        // Where * is the dot product
        template<class LATTICE>
        void SetUpDistribution(size_t _i,
                               std::function<Dimensionless(PhysicalVelocity const &)> _function);

    protected:
        // Reference to the wrapped lattice data.
        geometry::FieldData* latDat;
        geometry::Domain* domain;
    };

    inline void LatticeDataAccess::ZeroOutFOld() const
    {
        std::fill(latDat->m_currentDistributions.begin(), latDat->m_currentDistributions.end(), 0);
    }
    inline void LatticeDataAccess::ZeroOutForces() const
    {
        std::fill(latDat->m_force.begin(), latDat->m_force.end(), LatticeForceVector::Zero());
    }

    template<class LATTICE>
    void LatticeDataAccess::SetFOld(LatticeVector const &_pos, site_t _dir,
                                    distribn_t _value) const
    {
        // Figure location of distribution in memory using const access to FOld.
        // This way, access is resilient versus (some) changes in memory layout.
        geometry::Site<geometry::FieldData> const site(latDat->GetSite(_pos));
        auto siteFOld = site.GetFOld<LATTICE>();
        distribn_t const * const firstFOld = &latDat->m_currentDistributions[0];
        size_t const indexFOld(siteFOld.data() - firstFOld);
        latDat->m_currentDistributions[indexFOld + _dir] = _value;
    }

    template<class LATTICE>
    distribn_t const *
    LatticeDataAccess::GetFNew(LatticeVector const &_pos) const
    {
        // Figure location of distribution in memory using const access to FOld.
        // This way, access is resilient versus (some) changes in memory layout.
        auto site = latDat->GetSite(_pos);
        auto siteFOld = site.GetFOld<LATTICE>();
        distribn_t const * const firstFOld = &latDat->m_currentDistributions[0];
        size_t const indexFOld = siteFOld.data() - firstFOld;
        return latDat->GetFNew(indexFOld);
    }

    inline void ZeroOutFOld(geometry::FieldData* const latDat)
    {
        LatticeDataAccess(latDat).ZeroOutFOld();
    }
    inline void ZeroOutForces(geometry::FieldData* const latDat)
    {
        LatticeDataAccess(latDat).ZeroOutForces();
    }
    inline void ZeroOutForces(geometry::FieldData& latDat)
    {
        return ZeroOutForces(&latDat);
    }

    template<class LATTICE>
    void LatticeDataAccess::SetUpDistribution(
            size_t _i, std::function<Dimensionless(PhysicalVelocity const &)> _function)
    {
        auto&& dom = latDat->GetDomain();
        for (site_t i = 0; i < dom.GetLocalFluidSiteCount(); ++i)
        {
            auto site = latDat->GetSite(i);
            LatticeVector const pos = site.GetGlobalSiteCoords();
            LatticePosition const pos_real(pos[0], pos[1], pos[2]);
            distribn_t const * const siteFOld = site.GetFOld<LATTICE>().data();
            distribn_t const * const firstFOld = &latDat->m_currentDistributions[0];
            size_t const indexFOld(siteFOld - firstFOld);
            latDat->m_nextDistributions[indexFOld + _i] = latDat->m_currentDistributions[indexFOld + _i] = _function(pos_real);
        }
    }

    // Population i set to some distribution
    template<class LATTICE>
    void setUpDistribution(geometry::FieldData* latDat, size_t _i,
                           std::function<Dimensionless(PhysicalVelocity const &)> _function)
    {
        LatticeDataAccess(latDat).SetUpDistribution<LATTICE>(_i, _function);
    }

    // Initializes a lattice to empty distributions and forces, execpt at one
    // place
    template<class LATTICE>
    void allZeroButOne(geometry::FieldData& latDat, LatticeVector const &_pos)
    {
        LatticeDataAccess manip(&latDat);
        manip.ZeroOutFOld();
        manip.ZeroOutForces();
        for (size_t i(0); i < LATTICE::NUMVECTORS; ++i)
            manip.SetFOld<LATTICE>(_pos, i, 0.5 + i);
        latDat.GetSite(_pos).SetForce(LatticeForceVector(1, 2, 3));
    }

    template<class LATTICE>
    distribn_t const * GetFNew(geometry::FieldData& latDat, LatticeVector const &_pos)
    {
        return LatticeDataAccess(&latDat).GetFNew<LATTICE>(_pos);
    }

    template<class LATTICE = lb::D3Q15>
    std::tuple<Dimensionless, PhysicalVelocity,
            std::function<Dimensionless(PhysicalVelocity const &)>,
            std::function<Dimensionless(PhysicalVelocity const &)> > makeLinearProfile(
            size_t _cubeSize, geometry::FieldData * const latDat, PhysicalVelocity const &_grad)
    {

        PhysicalVelocity const gradient = _grad.GetNormalised();
        /* any number big enough to avoid negative populations */
        Dimensionless const non_neg_pop(_cubeSize * 3);
        auto linear = [non_neg_pop, gradient](PhysicalVelocity const &_v)
        {
            return non_neg_pop + Dot(_v, gradient);
        };
        auto linear_inv = [non_neg_pop, gradient](PhysicalVelocity const &_v)
        {
            return 2.0 * non_neg_pop - Dot(_v, gradient);
        };
        setUpDistribution<LATTICE>(latDat, 0, linear);
        setUpDistribution<LATTICE>(latDat, 1, linear_inv);

        return std::make_tuple(non_neg_pop, gradient, linear, linear_inv);
    }

}
#endif
