// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_DEBUG_LINUX_LINUXDEBUGGER_H
#define HEMELB_DEBUG_LINUX_LINUXDEBUGGER_H

#include <debug/common/ActiveDebugger.h>

namespace hemelb::debug
{

    class LinuxDebugger : public ActiveDebugger
    {
      protected:
        // Platform specific getters
        [[nodiscard]] std::string GetBinaryPath(void) const override;
        [[nodiscard]] std::string GetPlatformInterpreter(void) const override;
        [[nodiscard]] std::string GetPlatformScript(void) const override;

        // C'tor...
        LinuxDebugger(const char* executable, const net::MpiCommunicator& comm);
        // ... which the factory function needs to be able to get at.
        friend class Debugger;

    };

    // Factory. Don't be calling this.
    Debugger* PlatformDebuggerFactory(const char* const executable,
                                      const net::MpiCommunicator& comm);
}
#endif // HEMELB_DEBUG_LINUX_LINUXDEBUGGER_H
