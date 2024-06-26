// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include "net/MpiCommunicator.h"
#include "net/MpiEnvironment.h"
#include "net/IOCommunicator.h"
#include "log/Logger.h"
#include "debug.h"

#include "tests/helpers/HasCommsTestFixture.h"

int main(int argc, char* argv[]) {
  bool debug = false;

  // Add 
  Catch::Session session;
  using namespace Catch::clara;
  auto cli = session.cli() // Get Catch's composite command line parser
    | Opt(debug, "debug") // bind variable to a new option, with a hint string
    ["-g"] // option name
    ("enable the parallel debugger if HEMELB_USE_DEBUGER is ON (no-op if OFF)"); // help string
  session.cli(cli);

  // Parse the command line.
  int const cli_rc = session.applyCommandLine(argc, argv);
  if (cli_rc != 0) // Indicates a command line error
    return cli_rc;

  // Start MPI and the logger.
  hemelb::net::MpiEnvironment mpi(argc, argv);
  hemelb::log::Logger::Init();

  hemelb::net::MpiCommunicator commWorld = hemelb::net::MpiCommunicator::World();

  // Start the debugger (no-op if HEMELB_USE_DEBUGGER is OFF)
  hemelb::debug::Init(debug, argv[0], commWorld);

  // Initialise the global IOCommunicator.
  hemelb::net::IOCommunicator testCommunicator(commWorld);
  hemelb::tests::helpers::HasCommsTestFixture::Init(testCommunicator);

  int const local_rc = session.run();
  int const total_rc = commWorld.AllReduce(local_rc, MPI_SUM);
  return total_rc;
}
