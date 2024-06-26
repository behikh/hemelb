// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_REDBLOOD_DIVIDECONQUER_H
#define HEMELB_REDBLOOD_DIVIDECONQUER_H

#include <vector>
#include <map>
#include <cmath>
#include <type_traits>
#include "units.h"
#include "util/Vector3D.h"

namespace hemelb::redblood
{
    namespace details
    {
      // Short-hand to get type of the base of Divide and Conquer class
      // Also aggregates key type and compare functor
      template<class T>
      struct DnCBase
      {
          //! Key of the divide and conquer mapping
          using key_type = LatticeVector;
          //! Comparison of two divide and conquer keys
          struct CompareKeys
          {
              bool operator()(key_type const &a, key_type const &b) const
              {
                if (a.x() > b.x())
                {
                  return false;
                }
                else if (a.x() < b.x())
                {
                  return true;
                }

                if (a.y() > b.y())
                {
                  return false;
                }
                else if (a.y() < b.y())
                {
                  return true;
                }

                return a.z() < b.z();
              }
          };
          //! Base type for Divide and Conquer class
          using type = std::multimap<key_type, T, CompareKeys>;
      };
      static_assert(std::is_trivial_v<DnCBase<int>> && std::is_standard_layout_v<DnCBase<int>>, "Can be a struct");
      static_assert(std::is_trivial_v<DnCBase<int>::CompareKeys> && std::is_standard_layout_v<DnCBase<int>::CompareKeys>, "Can be a struct");
    }

    //! \brief Multimap for divide and conquer algorithms
    //! \details Items at a position x are mapped into boxes of a given size.
    template<class T>
    class DivideConquer : public details::DnCBase<T>::type
    {
        using base_type = typename details::DnCBase<T>::type;

      public:
        using key_type = typename base_type::key_type;
        using value_type = typename base_type::value_type;
        using reference = typename base_type::reference;
        using const_reference = typename base_type::const_reference;
        using iterator = typename base_type::iterator;
        using const_iterator = typename base_type::const_iterator;
        using range = std::pair<iterator, iterator>;
        using const_range = std::pair<const_iterator, const_iterator>;

        //! Constructor sets size of cutoff
        DivideConquer(LatticeDistance boxsize) :
            base_type(), boxsize(boxsize)
        {
        }
        //! Insert into divide and conquer container
        iterator insert(LatticePosition const &pos, T const &value)
        {
          return base_type::insert(value_type(DowngradeKey(pos), value));
        }
        //! Insert into divide and conquer container
        iterator insert(key_type const &pos, T const &value)
        {
          return base_type::insert(value_type(pos, value));
        }
        //! All objects in a single divide and conquer box
        range equal_range(LatticePosition const &pos)
        {
          return DivideConquer<T>::equal_range(DowngradeKey(pos));
        }
        //! All objects in a single divide and conquer box
        range equal_range(key_type const &pos)
        {
          return base_type::equal_range(pos);
        }
        //! All objects in a single divide and conquer box
        const_range equal_range(LatticePosition const &pos) const
        {
          return DivideConquer<T>::equal_range(DowngradeKey(pos));
        }
        //! All objects in a single divide and conquer box
        const_range equal_range(key_type const &pos) const
        {
          return base_type::equal_range(pos);
        }

        //! Length of each box
        LatticeDistance GetBoxSize() const
        {
          return boxsize;
        }

        //! Converts from position to box index
        key_type DowngradeKey(LatticePosition const &pos) const
        {
          return key_type(static_cast<LatticeCoordinate>(std::floor(pos.x() / boxsize)),
                          static_cast<LatticeCoordinate>(std::floor(pos.y() / boxsize)),
                          static_cast<LatticeCoordinate>(std::floor(pos.z() / boxsize)));
        }
        //! No conversion since in box index type already
        key_type DowngradeKey(key_type const &pos) const
        {
          return pos;
        }

      protected:
        LatticeDistance boxsize;
    };
} // hemelb::redblood

#endif
