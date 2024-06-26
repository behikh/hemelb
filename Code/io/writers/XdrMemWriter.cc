// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#include "io/writers/XdrMemWriter.h"

namespace hemelb::io
{

    XdrMemWriter::XdrMemWriter(char* dataBuffer, unsigned int dataLength) :
            XdrMetaWriter<char*>(dataBuffer, dataBuffer + dataLength)
    {
    }

}
