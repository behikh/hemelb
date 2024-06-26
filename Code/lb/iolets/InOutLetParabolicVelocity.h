// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_LB_IOLETS_INOUTLETPARABOLICVELOCITY_H
#define HEMELB_LB_IOLETS_INOUTLETPARABOLICVELOCITY_H
#include "lb/iolets/InOutLetVelocity.h"

namespace hemelb::lb
{
      class InOutLetParabolicVelocity : public InOutLetVelocity
      {
        public:
          InOutLetParabolicVelocity();
          ~InOutLetParabolicVelocity() override = default;
          [[nodiscard]] InOutLet* clone() const override;
          LatticeVelocity GetVelocity(const LatticePosition& x, const LatticeTimeStep t) const override;

          const LatticeSpeed& GetMaxSpeed() const
          {
            return maxSpeed;
          }
          void SetMaxSpeed(const LatticeSpeed& v)
          {
            maxSpeed = v;
          }

          void SetWarmup(unsigned int warmup)
          {
            warmUpLength = warmup;
          }

        protected:
          LatticeSpeed maxSpeed;
          unsigned int warmUpLength;
      };
}
#endif // HEMELB_LB_IOLETS_INOUTLETPARABOLICVELOCITY_H
