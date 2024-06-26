// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_LB_KERNELS_TRUNCATEDPOWERLAWRHEOLOGYMODEL_H
#define HEMELB_LB_KERNELS_TRUNCATEDPOWERLAWRHEOLOGYMODEL_H

#include "lb/kernels/AbstractRheologyModel.h"

namespace hemelb::lb
{
    struct InitParams;

    class TruncatedPowerLawRheologyModel : public AbstractRheologyModel<
            TruncatedPowerLawRheologyModel>
    {
    public:
        /*
         * GAMMA_{ZERO,INF} are computed with the power-law formula: nu = m * gamma^{n-1} for
         * given values of nu_{max,min}, m, and n.
         */
        static constexpr double GAMMA_ZERO = 6.25e-4; // s^{-1}, gamma=6.25e-4 for m=BLOOD_VISCOSITY_Pa_s, n=0.5 and eta_max=0.16 Pa_s
        static constexpr double GAMMA_INF = 1.3061; // s^{-1}, gamma=1.3061 for m=BLOOD_VISCOSITY_Pa_s, n=0.5 and eta_min=0.0035 Pa_s

        static constexpr double N_CONSTANT = 0.5; // dimensionless, n<1 shear thinning, n>1 shear thickening, n=1 Newtonian with nu=M_CONSTANT

	    TruncatedPowerLawRheologyModel(InitParams& initParams);

        /*
         *  Compute nu for a given shear rate according to the truncated power-law:
         *
         *        { M_CONSTANT * GAMMA_ZERO^{N_CONSTANT - 1},  if iShearRate<GAMMA_ZERO
         *  eta = { M_CONSTANT *  GAMMA_INF^{N_CONSTANT - 1},  if iShearRate>GAMMA_INF
         *        { M_CONSTANT * iShearRate^{N_CONSTANT - 1},  otherwise
         *
         *  @param iShearRate local shear rate value (s^{-1}).
         *  @param iDensity local density. TODO at the moment this value is not used
         *         in any subclass.
         *
         *  @return dynamic viscosity (Pa s).
         */
        double CalculateViscosityForShearRate(const double &iShearRate,
                                              const distribn_t &iDensity) const;
    private:
        PhysicalDynamicViscosity M_CONSTANT;
    };
}

#endif /* HEMELB_LB_KERNELS_TRUNCATEDPOWERLAWRHEOLOGYMODEL_H */
