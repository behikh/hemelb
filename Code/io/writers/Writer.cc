// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#include "io/writers/Writer.h"

namespace hemelb::io
{

      Writer& Writer::operator<<(enum Writer::Separator const & value)
      {
        writeRecordSeparator();
        return *this;
      }

}
