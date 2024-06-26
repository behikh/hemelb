// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_LB_KERNELS_CASSONRHEOLOGYMODEL_H
#define HEMELB_LB_KERNELS_CASSONRHEOLOGYMODEL_H

#include "lb/kernels/AbstractRheologyModel.h"

namespace hemelb::lb
{
    struct InitParams;

    class CassonRheologyModel : public AbstractRheologyModel<CassonRheologyModel>
    {
    public:
            // Casson model constants
            static constexpr double K0 = 0.1937; // Pa^{1/2}
            static constexpr double K1 = 0.055; // (Pa*s)^{1/2}

            static constexpr double CASSON_MAX_VISCOSITY = 0.16; // Pa*s

            // Satisfy RheologyModel concept
            inline CassonRheologyModel(const InitParams&) {}
            /*
             *  Compute nu for a given shear rate according to the Casson model:
             *
             *  eta = (K0 + K1*sqrt(iShearRate))^2 / iShearRate
             *  nu = eta / density
             *
             *  @param iShearRate local shear rate value (s^{-1}).
             *  @param iDensity local density. TODO at the moment this value is not used
             *         in any subclass.
             *
             *  @return dynamic viscosity (Pa s).
             */
            double CalculateViscosityForShearRate(const double &iShearRate,
						  const distribn_t &iDensity) const;
    };
}

#endif /* HEMELB_LB_KERNELS_CASSONRHEOLOGYMODEL_H */
