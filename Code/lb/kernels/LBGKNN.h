// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_LB_KERNELS_LBGKNN_H
#define HEMELB_LB_KERNELS_LBGKNN_H

#include <cmath>

#include "hassert.h"
#include "units.h"
#include "lb/concepts.h"
#include "lb/HydroVars.h"
#include "lb/LbmParameters.h"
#include "lb/SimulationState.h"

namespace hemelb::lb
{
    template<class tRheologyImplementation>
    class AbstractRheologyModel;

    namespace detail
    {
        template <typename RHEO>
        struct has_calc_visc {
            using prototype = PhysicalDynamicViscosity (RHEO::*)(const PhysicalRate&, const LatticeDensity&) const;
            static constexpr bool value = std::is_same<
                    prototype,
                    decltype(&RHEO::CalculateViscosityForShearRate)
            >::value;
        };
    }

    /**
     * Class extending the original BGK collision operator to support non-Newtonian
     * fluids. Implements support for relaxation time not constant across the domain.
     */
    template<class tRheologyModel, lattice_type L>
    class LBGKNN
    {
	// Eventually these could be made a concept
	//
	// One might like to check these in AbstractRheologyModel, but
	// the derived class is incomplete at that point so can't
	// check its traits there.
	static_assert(std::is_base_of<AbstractRheologyModel<tRheologyModel>, tRheologyModel>::value,
		      "tRheologyModel must inherit AbstractRheologyModel via CRTP");
	static_assert(std::is_constructible<tRheologyModel, const InitParams>::value,
		      "tRheologyModel must be constructable from InitParams");
	static_assert(detail::has_calc_visc<tRheologyModel>::value,
		      "tRheologyModel must have member function "
		      "`PhysicalDynamicViscosity CalculateViscosityForShearRate(const PhysicalRate&, const LatticeDensity&) const`");

    public:
        using LatticeType = L;
        using VarsType = HydroVars<LBGKNN>;

        LBGKNN(InitParams& initParams)
                : mTau(initParams.latDat->GetLocalFluidSiteCount(), initParams.lbmParams->GetTau()),
                  mLbParams(*initParams.lbmParams),
                  mRheo(initParams)
        {
        }

        void CalculateDensityMomentumFeq(VarsType& hydroVars, site_t index)
        {
            LatticeType::CalculateDensityMomentumFEq(hydroVars.f,
                                                     hydroVars.density,
                                                     hydroVars.momentum,
                                                     hydroVars.velocity,
                                                     hydroVars.f_eq);

            for (unsigned int ii = 0; ii < LatticeType::NUMVECTORS; ++ii)
            {
              hydroVars.f_neq[ii] = hydroVars.f[ii] - hydroVars.f_eq[ii];
            }

            // Use the value of tau computed during the previous time step in coming calls to DoCollide
            HASSERT(index < site_t(mTau.size()));
            hydroVars.tau = mTau[index];

            // Compute the local relaxation time that will be used in the next time step
            UpdateLocalTau(mTau[index], hydroVars);
        }

        void CalculateFeq(VarsType& hydroVars, site_t index)
        {
            LatticeType::CalculateFeq(hydroVars.density,
                                      hydroVars.momentum,
                                      hydroVars.f_eq);

            for (unsigned int ii = 0; ii < LatticeType::NUMVECTORS; ++ii)
            {
              hydroVars.f_neq[ii] = hydroVars.f[ii] - hydroVars.f_eq[ii];
            }

            // Use the value of tau computed during the previous time step in coming calls to DoCollide
            HASSERT(index < site_t(mTau.size()));
            hydroVars.tau = mTau[index];

            // Compute the local relaxation time that will be used in the next time step
            UpdateLocalTau(mTau[index], hydroVars);
        }

        void Collide(const LbmParameters* const lbmParams, VarsType& hydroVars)
        {
            double omega = -1.0 / hydroVars.tau;

            for (Direction direction = 0; direction < LatticeType::NUMVECTORS; ++direction)
            {
                hydroVars.SetFPostCollision(direction,
                                            hydroVars.f[direction]
                                            + hydroVars.GetFNeq()[direction] * omega);
            }
        }

        /*
         *  Helper method used in testing in order to access the mTau array after
         *  being set by CalculateDensityMomentumFeq
         */
        const std::vector<distribn_t>& GetTauValues() const
        {
            return mTau;
        }

    private:
        /**
          * Vector containing the current relaxation time for each site in the domain. It will be initialised
          * with the relaxation time corresponding to HemeLB's default Newtonian viscosity and each time step
          * will be updated based on the local hydrodynamic configuration
          */
        std::vector<distribn_t> mTau;

        // Our copy of the base LB parameters
        LbmParameters mLbParams;

        // Our rheology model
        tRheologyModel mRheo;

        /**
         *  Helper method to update the value of local relaxation time (tau) from a given hydrodynamic
         *  configuration. It requires values of f_neq and density at the current time step and it will
         *  compute the value of tau to be used in the next time step.
         *
         *  @param localTau input: tau being used during the current time step,
         *                  output: tau to be used in the following time step
         *  @param hydroVars hydrodynamic configuration for a given lattice site
         */
        void UpdateLocalTau(distribn_t& localTau, VarsType& hydroVars) const
        {
            /*
             * Shear-rate returned by CalculateShearRate is dimensionless and CalculateTauForShearRate
             * wants it in units of s^{-1}
             */
            double shear_rate = LatticeType::CalculateShearRate(localTau,
                                                                hydroVars.f_neq,
                                                                hydroVars.density) / mLbParams.GetTimeStep();

            // Update tau
            localTau = mRheo.CalculateTauForShearRate(shear_rate,
						      hydroVars.density,
						      mLbParams);

            // In some rheology models viscosity tends to infinity as shear rate goes to zero.
            HASSERT(!std::isinf(localTau));
            HASSERT(!std::isnan(localTau));
        }
    };
}

#endif
