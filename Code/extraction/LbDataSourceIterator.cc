// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#include "extraction/LbDataSourceIterator.h"

namespace hemelb::extraction
{
    LbDataSourceIterator::LbDataSourceIterator(const lb::MacroscopicPropertyCache& propertyCache,
                                               const geometry::FieldData& data, int rank_,
                                               std::shared_ptr<util::UnitConverter> converter) :
            propertyCache(propertyCache), data(data), rank(rank_), converter(std::move(converter)), position(-1)
    {
    }

    bool LbDataSourceIterator::ReadNext()
    {
      ++position;

      if (position >= data.GetDomain().GetLocalFluidSiteCount())
      {
        return false;
      }

      return true;
    }

    util::Vector3D<site_t> LbDataSourceIterator::GetPosition() const
    {
      return data.GetSite(position).GetGlobalSiteCoords();
    }

    FloatingType LbDataSourceIterator::GetPressure() const
    {
      return converter->ConvertPressureToPhysicalUnits(propertyCache.densityCache.Get(position)
          * Cs2);
    }

    util::Vector3D<FloatingType> LbDataSourceIterator::GetVelocity() const
    {
      return converter->ConvertVelocityToPhysicalUnits(propertyCache.velocityCache.Get(position).as<float>());
    }

    FloatingType LbDataSourceIterator::GetShearStress() const
    {
      return converter->ConvertStressToPhysicalUnits(propertyCache.wallShearStressMagnitudeCache.Get(position));
    }

    FloatingType LbDataSourceIterator::GetVonMisesStress() const
    {
      return converter->ConvertStressToPhysicalUnits(propertyCache.vonMisesStressCache.Get(position));
    }

    FloatingType LbDataSourceIterator::GetShearRate() const
    {
      return converter->ConvertShearRateToPhysicalUnits(propertyCache.shearRateCache.Get(position));
    }

    util::Matrix3D LbDataSourceIterator::GetStressTensor() const
    {
      return converter->ConvertFullStressTensorToPhysicalUnits(propertyCache.stressTensorCache.Get(position));
    }

    util::Vector3D<PhysicalStress> LbDataSourceIterator::GetTraction() const
    {
      return converter->ConvertTractionToPhysicalUnits(propertyCache.tractionCache.Get(position),
                                                      data.GetSite(position).GetWallNormal());
    }

    util::Vector3D<PhysicalStress> LbDataSourceIterator::GetTangentialProjectionTraction() const
    {
      return converter->ConvertStressToPhysicalUnits(propertyCache.tangentialProjectionTractionCache.Get(position));
    }

    const distribn_t* LbDataSourceIterator::GetDistribution() const
    {
      auto site = data.GetSite(position);
      return site.GetFOld(data.GetDomain().GetLatticeInfo().GetNumVectors());
    }

    void LbDataSourceIterator::Reset()
    {
      position = -1;
    }

    bool LbDataSourceIterator::IsValidLatticeSite(const util::Vector3D<site_t>& location) const
    {
      return data.GetDomain().IsValidLatticeSite(location);
    }

    bool LbDataSourceIterator::IsAvailable(const util::Vector3D<site_t>& location) const
    {
      return data.GetDomain().GetProcIdFromGlobalCoords(location) == rank;
    }

    PhysicalDistance LbDataSourceIterator::GetVoxelSize() const
    {
      return converter->GetVoxelSize();
    }

    const PhysicalPosition& LbDataSourceIterator::GetOrigin() const
    {
      return converter->GetLatticeOrigin();
    }

    bool LbDataSourceIterator::IsWallSite(const util::Vector3D<site_t>& location) const
    {
      site_t localSiteId = data.GetDomain().GetContiguousSiteId(location);

      return data.GetSite(localSiteId).IsWall();
    }

    unsigned LbDataSourceIterator::GetNumVectors() const
    {
      return data.GetDomain().GetLatticeInfo().GetNumVectors();
    }
}
