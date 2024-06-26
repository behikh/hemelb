// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#include "tests/helpers/FourCubeLatticeData.h"

#include <cstdlib>
#include "units.h"
#include "geometry/Domain.h"
#include "geometry/GmyReadResult.h"
#include "geometry/LookupTree.h"
#include "io/formats/geometry.h"
#include "util/Vector3D.h"
#include "lb/lattices/D3Q15.h"
#include "net/IOCommunicator.h"

namespace hemelb::tests
{

    TestSiteData::TestSiteData(geometry::SiteData& siteData) :
      geometry::SiteData(siteData)
    {
    }

    void TestSiteData::SetHasWall(Direction direction)
    {
      unsigned newValue = geometry::SiteData::GetWallIntersectionData();
      newValue |= 1U << (direction - 1);
      wallIntersection = newValue;
    }

    void TestSiteData::SetHasIolet(Direction direction)
    {
      unsigned newValue = geometry::SiteData::GetIoletIntersectionData();
      newValue |= 1U << (direction - 1);
      ioletIntersection = newValue;
    }

    void TestSiteData::SetIoletId(int boundaryId)
    {
      ioletId = boundaryId;
    }

    /**
     * The create function makes a 4 x 4 x 4 cube of sites from (0,0,0) to (3,3,3).
     * The plane (x,y,0) is an inlet (boundary 0).
     * The plane (x,y,3) is an outlet (boundary 1).
     * The planes (0,y,z), (3,y,z), (x,0,z) and (x,3,z) are all walls.
     *
     * @return
     */
    std::shared_ptr<geometry::Domain> FourCubeDomain::Create(const net::IOCommunicator& comm, site_t sitesPerBlockUnit, proc_t rankCount)
    {
        using namespace geometry;
        GmyReadResult readResult(Vec16::Ones(),
                                 sitesPerBlockUnit);

        // VoxelSize = 0.01
        // Origin = util::Vector3D<PhysicalDistance>::Zero();
        site_t sitesAlongCube = sitesPerBlockUnit - 2;
        site_t minInd = 1, maxInd = sitesAlongCube;
      
        geometry::BlockReadResult& block = readResult.Blocks[0];
        block.Sites.resize(readResult.GetSitesPerBlock(), geometry::GeometrySite(false));

        site_t index = -1;
        for (site_t i = 0; i < sitesPerBlockUnit; ++i) {
            for (site_t j = 0; j < sitesPerBlockUnit; ++j) {
                for (site_t k = 0; k < sitesPerBlockUnit; ++k) {
                    ++index;

                    if (i < minInd || i > maxInd ||
                        j < minInd || j > maxInd ||
                        k < minInd || k	> maxInd)
                        continue;

	    geometry::GeometrySite& site = block.Sites[index];

	    site.isFluid = true;
	    site.targetProcessor = 0;

	    for (Direction direction = 1; direction < lb::D3Q15::NUMVECTORS; ++direction)
	      {
		site_t neighI = i + lb::D3Q15::CX[direction];
		site_t neighJ = j + lb::D3Q15::CY[direction];
		site_t neighK = k + lb::D3Q15::CZ[direction];

		geometry::GeometrySiteLink link;

		float randomDistance = (float(std::rand() % 10000) / 10000.0);
		using CutType = io::formats::geometry::CutType;
		// The inlet is by the minimal z value.
		if (neighK < minInd)
                  {
                    link.ioletId = 0;
                    link.type = CutType::INLET;
                    link.distanceToIntersection = randomDistance;
                  }
		// The outlet is by the maximal z value.
		else if (neighK > maxInd)
                  {
                    link.ioletId = 0;
                    link.type = CutType::OUTLET;
                    link.distanceToIntersection = randomDistance;
                  }
		// Walls are by extremes of x and y.
		else if (neighI < minInd || neighJ < minInd || neighI > maxInd || neighJ > maxInd)
                  {
                    link.type = CutType::WALL;
                    link.distanceToIntersection = randomDistance;
                  }

		site.links.push_back(link);

	      }

	    /*
	     * For sites at the intersection of two cube faces considered wall (i.e. perpendicular to the x or y
	     * axes), we arbitrarily choose the normal to lie along the y axis. The logic below must be consistent
	     * with Scripts/SimpleGeometryGenerationScripts/four_cube.py
	     */
	    if (i == minInd)
	      {
		site.wallNormalAvailable = true;
		site.wallNormal = util::Vector3D<float>(-1, 0, 0);
	      }
	    if (i == maxInd)
	      {
		site.wallNormalAvailable = true;
		site.wallNormal = util::Vector3D<float>(1, 0, 0);
	      }
	    if (j == minInd)
	      {
		site.wallNormalAvailable = true;
		site.wallNormal = util::Vector3D<float>(0, -1, 0);
	      }
	    if (j == maxInd)
	      {
		site.wallNormalAvailable = true;
		site.wallNormal = util::Vector3D<float>(0, 1, 0);
	      }
                }
            }
        }

        readResult.block_store = std::make_unique<octree::DistributedStore>(
                readResult.GetSitesPerBlock(),
                octree::build_block_tree(
                        readResult.GetBlockDimensions().as<octree::U16>(),
                        {readResult.GetSitesPerBlock()}
                ),
                std::vector{0},
                comm
        );
        auto domain = std::make_shared<FourCubeDomain>(
                lb::D3Q15::GetLatticeInfo(),
                readResult,
                comm
        );

      // First, fiddle with the fluid site count, for tests that require this set.
      domain->fluidSitesOnEachProcessor.resize(rankCount);
        domain->fluidSitesOnEachProcessor[0] = sitesAlongCube * sitesAlongCube
	* sitesAlongCube;
      for (proc_t rank = 1; rank < rankCount; ++rank) {
          domain->fluidSitesOnEachProcessor[rank] = rank * 1000;
      }
        return domain;
    }

    void FourCubeDomain::SetHasWall(site_t site, Direction direction)
    {
      TestSiteData mutableSiteData(siteData[site]);
      mutableSiteData.SetHasWall(direction);
      siteData[site] = geometry::SiteData(mutableSiteData);
    }

    void FourCubeDomain::SetHasIolet(site_t site, Direction direction)
    {
      TestSiteData mutableSiteData(siteData[site]);
      mutableSiteData.SetHasIolet(direction);
      siteData[site] = geometry::SiteData(mutableSiteData);
    }

    void FourCubeDomain::SetIoletId(site_t site, int id)
    {
      TestSiteData mutableSiteData(siteData[site]);
      mutableSiteData.SetIoletId(id);
      siteData[site] = geometry::SiteData(mutableSiteData);
    }

    void FourCubeDomain::SetBoundaryDistance(site_t site, Direction direction, distribn_t distance)
    {
      distanceToWall[ (lb::D3Q15::NUMVECTORS - 1) * site + direction - 1] = distance;
    }

    void FourCubeDomain::SetBoundaryNormal(site_t site, util::Vector3D<distribn_t> boundaryNormal)
    {
      wallNormalAtSite[site] = boundaryNormal;
    }

    FourCubeLatticeData* FourCubeLatticeData::Create(const net::IOCommunicator& comm, site_t sitesPerBlockUnit, proc_t rankCount) {
      return new FourCubeLatticeData{FourCubeDomain::Create(comm, sitesPerBlockUnit, rankCount)};
    }
}
