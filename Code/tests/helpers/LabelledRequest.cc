// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#include "tests/helpers/LabelledRequest.h"

#include <cstring>
#include <iostream>

namespace hemelb
{
  namespace tests
  {
    namespace net
    {
        template <bool is_const>
      LabelledRequest<is_const>::LabelledRequest(typename Base::ptr pointer, int count, MPI_Datatype type, proc_t rank, const std::string &label) :
	Base(pointer, count, type, rank), Label(label)
      {
      }
      template <bool is_const>
      bool LabelledRequest<is_const>::EnvelopeIdentical(const Base & other)
      {
          auto& self = *this;
	bool this_ok = ( (self.Count == other.Count) && (self.Rank == other.Rank) && (self.Type == other.Type));
	if (!this_ok)
	  {
	    std::cerr << "Envelope different: " << Label << " R: " << self.Rank << " C: " << self.Count << " T: " << " : "
		      << " R" << other.Rank << " C " << other.Count << " T " << std::endl;
	    // uncomment the below for type diagnostics if your compiler supports them
	    /*std::cerr << "Types are:" << static_cast<void*>(Type) << " : " << static_cast<void*>(other.Type) << std::endl;
	      std::cerr << "Type candidates include" << " int: " << static_cast<void*>(MpiDataType<int>())
	      << " unsigned int :" << static_cast<void*>(MpiDataType<unsigned int>())
	      << " site_t: " << static_cast<void*>(MpiDataType<site_t>())
	      << " proc_t: " << static_cast<void*>(MpiDataType<proc_t>())
	      << std::endl;*/
	  }
	/*else{
	  std::cerr << "Envelope same: " << Label << " R: " << Rank << " C: " << Count << " T: " <<  " : "
	  << " R" << other.Rank << " C " << other.Count << " T " << std::endl;
	  }*/
	return this_ok;
      }

      template <bool is_const>
      bool LabelledRequest<is_const>::PayloadIdentical(const Base & other)
      {
          auto& self = *this;
	// reduction
	bool ok = true;
	for (int element = 0; element < self.Count; element++)
	  {
	    int size;

	    MPI_Type_size(other.Type, &size);
	    // The below use of unsigned char is not formally correct (due to the possibility of char not having alignment 1)
	    // But we cannot currently see a better solution to avoid compiler warnings from void* arithmetic.
	    bool this_ok = 0
	      == std::memcmp(static_cast<unsigned char const*>(other.Pointer) + size * element,
			     static_cast<unsigned char const *>(self.Pointer) + size * element,
			     size);
	    if (!this_ok)
              {

                std::cerr << "Unexpected data in request: " << Label << " R " << self.Rank << " C " << self.Count << " : "
			  << std::endl;
                for (int i = 0; i < size; i++)
		  {
		    // The below use of unsigned char is not formally correct (due to the possibility of char not having alignment 1)
		    // But we cannot currently see a better solution to avoid compiler warnings from void* arithmetic.
		    std::cerr << i << " : "
			      << static_cast<int>(* (static_cast<unsigned char const*>(self.Pointer) + size * element + i)) << " "
                      << static_cast<int>(* (static_cast<unsigned char const*>(other.Pointer) + size * element + i)) << std::endl;
		  }
                std::cerr << std::endl;
              }
	    /*//  --- use this block for debugging
              else
              {
	      std::cerr << "Expected data in request: " << Label << " R " << Rank << " C " << Count << " : "
	      << std::endl;
	      for (int i = 0; i < size; i++)
	      {
	      // The below use of unsigned char is not formally correct (due to the possibility of char not having alignment 1)
	      // But we cannot currently see a better solution to avoid compiler warnings from void* arithmetic.
	      std::cerr << i << " : "
	      << static_cast<int>(* (static_cast<unsigned char*>(Pointer) + size * element + i)) << " "
	      << static_cast<int>(* (static_cast<unsigned char*>(other.Pointer) + size * element + i)) << std::endl;
	      }
              }*/
	    ok = ok && this_ok;
	  }
	return ok;
      }

      template <bool is_const>
      void LabelledRequest<is_const>::Unpack(Base& other)
      {
          if constexpr (is_const) {
              throw Exception() << "Calling Unpack on const LabelledRequest!";
          } else {
              for (int element = 0; element < this->Count; element++) {
                  int size;
                  MPI_Type_size(other.Type, &size);

                  // The below use of unsigned char is not formally correct (due to the possibility of char not having alignment 1)
                  // But we cannot currently see a better solution to avoid compiler warnings from void* arithmetic.
                  std::memcpy(static_cast<unsigned char *>(other.Pointer) + size * element,
                              static_cast<unsigned char *>(this->Pointer) + size * element,
                              size);
              }
          }
      }
      template class LabelledRequest<true>;
      template class LabelledRequest<false>;
    }
  }
}
