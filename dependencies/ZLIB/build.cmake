# This file is part of HemeLB and is Copyright (C)
# the HemeLB team and/or their institutions, as detailed in the
# file AUTHORS. This software is provided under the terms of the
# license in the file LICENSE.
include_guard()

find_file(ZLIB_TARBALL zlib-1.2.6.tar.gz
  DOC "Path to download ZLIB (can be url http://)"
  PATHS ${HEMELB_DEPENDENCIES_PATH}/distributions
  )
if(NOT ZLIB_TARBALL)
  message("No zlib source found, will download.")
  set(ZLIB_TARBALL http://sourceforge.net/projects/libpng/files/zlib/1.2.6/zlib-1.2.6.tar.gz
    CACHE STRING "Path to download ZLIB (can be local file://)" FORCE)
endif()
ExternalProject_Add(
  dep_ZLIB
  INSTALL_DIR ${HEMELB_DEPENDENCIES_INSTALL_PREFIX}
  URL ${ZLIB_TARBALL}
  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
  BUILD_COMMAND make -j${HEMELB_SUBPROJECT_MAKE_JOBS}
  BUILD_IN_SOURCE 1
)
