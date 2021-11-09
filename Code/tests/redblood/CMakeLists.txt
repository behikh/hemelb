# This file is part of HemeLB and is Copyright (C)
# the HemeLB team and/or their institutions, as detailed in the
# file AUTHORS. This software is provided under the terms of the
# license in the file LICENSE.
target_sources(hemelb-tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/Fixtures.cc

  ${CMAKE_CURRENT_SOURCE_DIR}/BendingTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/BordersTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/CellArmyTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/CellIOTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/CellInserterTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/CellCellInteractionTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/CellCellInteractionWithGridTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/CellForceSpreadTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/CellTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/CellVelocityInterpolTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/DivideConquerTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/EnergyTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/FacetTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/FaderCellTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/FlowExtensionTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/GradientKernTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/GradientTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/InterpolationTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/LoadDeformedCellTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/LoadingTimmMeshTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/Node2NodeTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/NodeIntegrationTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/RedBloodMeshDataIOTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/RedBloodMeshTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/RedBloodMeshVTKDataIOTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/StencilTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/TopologyTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/VertexBagTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/WallCellPairIteratorTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/WallNodeDnCTests.cc

  ${CMAKE_CURRENT_SOURCE_DIR}/buffer/BufferTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/buffer/ColumnsTests.cc

  ${CMAKE_CURRENT_SOURCE_DIR}/parallel/NodeParallelizationTests.cc

  # Longer-running integration tests
  ${CMAKE_CURRENT_SOURCE_DIR}/CellIntegrationTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/FadeInOutIntegrationTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/SadCellTest.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/SadCellIntegrationTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/MulticellBenchmarkTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/FedosovValidationTests.cc
)

target_sources(mpi_redblood_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/Fixtures.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/parallel/ParallelFixtures.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/parallel/ParallelFixtureTests.cc

  ${CMAKE_CURRENT_SOURCE_DIR}/parallel/CellParallelizationTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/parallel/MPISpreadForcesTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/parallel/MPIIntegrateVelocities.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/parallel/MPILockStepTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/parallel/GraphCommsTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/parallel/RBCInserterWithPerturbationParallelTests.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/parallel/MPIParallelIntegrationTests.cc
)