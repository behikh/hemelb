// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_NET_MIXINS_STORINGNET_H
#define HEMELB_NET_MIXINS_STORINGNET_H
#include "net/BaseNet.h"
#include "log/Logger.h"
#include "net/ProcComms.h"
namespace hemelb
{
  namespace net
  {
    class StoringNet : public virtual BaseNet
    {
      public:
        StoringNet(const MpiCommunicator& comms);

        void RequestSendImpl(void const* pointer, int count, proc_t rank, MPI_Datatype type) override;
        void RequestReceiveImpl(void* pointer, int count, proc_t rank, MPI_Datatype type) override;

        void RequestGatherVSendImpl(void const* buffer, int count, proc_t toRank, MPI_Datatype type) override;
        void RequestGatherReceiveImpl(void* buffer, MPI_Datatype type) override;

        void RequestGatherSendImpl(void* buffer, proc_t toRank, MPI_Datatype type) override;
        void RequestGatherVReceiveImpl(void* buffer, int * displacements, int *counts,
                                       MPI_Datatype type) override;

        virtual void RequestAllToAllReceiveImpl(void * buffer, int count, MPI_Datatype type) override;
        virtual void RequestAllToAllSendImpl(void * buffer, int count, MPI_Datatype type) override;

      protected:
        /**
         * Struct representing all that's needed to successfully communicate with another processor.
         */

        std::map<proc_t, ProcComms<true>> sendProcessorComms;
        std::map<proc_t, ProcComms<false>> receiveProcessorComms;

        std::map<proc_t, ProcComms<true>> gatherVSendProcessorComms;
        GatherVReceiveProcComms gatherVReceiveProcessorComms;

        std::map<proc_t, GatherProcComms> gatherSendProcessorComms;
        GatherProcComms gatherReceiveProcessorComms;

        AllToAllProcComms allToAllReceiveProcComms;
        AllToAllProcComms allToAllSendProcComms;

    };
  }
}
#endif
