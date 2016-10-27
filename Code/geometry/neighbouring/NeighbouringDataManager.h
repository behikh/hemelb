
// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_GEOMETRY_NEIGHBOURING_NEIGHBOURINGDATAMANAGER_H
#define HEMELB_GEOMETRY_NEIGHBOURING_NEIGHBOURINGDATAMANAGER_H

#include "geometry/LatticeData.h"
#include "geometry/neighbouring/NeighbouringLatticeData.h"
#include "geometry/neighbouring/RequiredSiteInformation.h"
#include "net/IteratedAction.h"
#include <vector>
#include <map>

namespace hemelb
{
  namespace geometry
  {
    class LatticeData;

    namespace neighbouring
    {

      class NeighbouringDataManager : public net::IteratedAction
      {
        public:
          NeighbouringDataManager(const LatticeData & localLatticeData,
                                  NeighbouringLatticeData & neighbouringLatticeData,
                                  comm::Async::Ptr cq);
          // Initially, the required site information will not be used -- we just transfer everything.
          // This considerably simplifies matters.
          // Nevertheless, we provide the interface here in its final form
          void RegisterNeededSite(site_t globalId,
                                  RequiredSiteInformation requirements = RequiredSiteInformation(true));
	  
	  // This is collective across the communicator used by the
	  // communication queue given to the constructor
          void ShareNeeds();
          std::vector<site_t> &GetNeedsForProc(proc_t proc)
          {
            return needsEachProcHasFromMe[proc];
          }
          std::vector<site_t> & GetNeededSites()
          {
            return neededSites;
          }
          void TransferNonFieldDependentInformation();
          void TransferFieldDependentInformation();
          // NB this is virtual so that the class can be tested.
          virtual proc_t ProcForSite(site_t site);
        protected:
          void RequestComms();
        private:
          const LatticeData & localLatticeData;
          NeighbouringLatticeData & neighbouringLatticeData;
	  comm::Async::Ptr commQ;
          typedef std::vector<site_t> IdVec;
          typedef std::map<int, int> CountMap;
          typedef std::map<proc_t, IdVec > IdsMap;
          IdVec neededSites;
          IdsMap needsEachProcHasFromMe;

          bool needsHaveBeenShared;

      };

    }
  }
}

#endif
