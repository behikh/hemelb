# This file is part of HemeLB and is Copyright (C)
# the HemeLB team and/or their institutions, as detailed in the
# file AUTHORS. This software is provided under the terms of the
# license in the file LICENSE.
hemelb_dependency(MPWide find)

macro(hemelb_add_target_dependency_mpwide tgt)
  target_include_directories(${tgt} PRIVATE ${MPWide_INCLUDE_DIR})
  target_compile_definitions(${tgt} PRIVATE -DHEMELB_BUILD_MULTISCALE)
  target_link_libraries(${tgt} PRIVATE ${MPWide_LIBRARIES})
endmacro()
