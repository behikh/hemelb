// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#include "redblood/Borders.h"

namespace hemelb::redblood
{
    BorderBoxIterator::BorderBoxIterator(size_t const nearness) :
        current(-1, -1, -1), isValid(true),
            doCenter(nearness bitand static_cast<size_t>(Borders::CENTER)),
            doTop(nearness bitand static_cast<size_t>(Borders::TOP)),
            doBottom(nearness bitand static_cast<size_t>(Borders::BOTTOM)),
            doNorth(nearness bitand static_cast<size_t>(Borders::NORTH)),
            doSouth(nearness bitand static_cast<size_t>(Borders::SOUTH)),
            doWest(nearness bitand static_cast<size_t>(Borders::WEST)),
            doEast(nearness bitand static_cast<size_t>(Borders::EAST))
    {
      if (not wannaSee(current))
      {
        try
        {
          operator++();
        }
        catch (...)
        {
          isValid = false;
          throw;
        }
      }
    }

    bool BorderBoxIterator::wannaSee(value_type const &point) const
    {
      return ((point.x() == -1 and doBottom) or (point.x() == 1 and doTop) or point.x() == 0)
          and ((point.y() == -1 and doSouth) or (point.y() == 1 and doNorth) or point.y() == 0)
          and ((point.z() == -1 and doWest) or (point.z() == 1 and doEast) or point.z() == 0)
          and (point.x() != 0 or point.y() != 0 or point.z() != 0 or doCenter);
    }

    BorderBoxIterator& BorderBoxIterator::operator++()
    {
      if (not isValid)
      {
        return *this;
      }
      bool const hasCycled = true;
      auto increment = [](value_type::value_type &x)
      {
        ++x;
        if(x == 2)
        {
          x = -1;
          return hasCycled;
        }
        return not hasCycled;
      };
      do
      {
        bool const cycled0 = increment(current[0]) == hasCycled;
        bool const cycled1 = cycled0 ?
          increment(current[1]) == hasCycled :
          false;
        isValid = cycled1 ?
          increment(current[2]) != hasCycled :
          true;
      }
      while (isValid and not wannaSee(current));
      return *this;
    }
  } // namespace hemelb::redblood
