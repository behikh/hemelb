// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#include <string>
#include <cstdio>

#include <catch2/catch.hpp>

#include "io/FILE.h"
#include "io/formats/extraction.h"
#include "io/readers/XdrMemReader.h"
#include "extraction/PropertyOutputFile.h"
#include "extraction/OutputField.h"
#include "extraction/WholeGeometrySelector.h"
#include "extraction/LocalPropertyOutput.h"

#include "tests/helpers/HasCommsTestFixture.h"
#include "tests/extraction/DummyDataSource.h"

namespace hemelb
{
  namespace tests
  {
    namespace {
      const char* tempXtrFileName = "simple.xtr";
      const char* tempOffFileName = "simple.off";
      // Power of 2 for simple binary
      constexpr double REFERENCE_PRESSURE_mmHg = 8.0;

      constexpr double epsilon = 1e-5;
      template <typename T>
      Approx apprx(T&& x) {
	return Approx(std::forward<T>(x)).epsilon(epsilon);
      }

      void CheckDataWriting(DummyDataSource* datasource, uint64_t timestep, io::FILE& file) {
	// The file should have an entry for each lattice point, consisting
	// of 3D grid coords, pressure (with an offset of 80) and 3D velocity.
	// This gives 3*4 + 4 + 3*4 = 28 bytes per site.
	long siteCount = 0;
	datasource->Reset();
	while (datasource->ReadNext()) {
	  ++siteCount;
	}

	// We also have the iteration number, a long
	size_t expectedSize = 8 + 28 * siteCount;

	// Attempt to read one extra byte, to make sure we aren't under-reading
	auto contentsBuffer = std::make_unique<char[]>(expectedSize);
	size_t nRead = file.read(contentsBuffer.get(), 1, expectedSize + 1);

	REQUIRE(expectedSize == nRead);

	// Create an XDR buffer reader to process the file for comparison
	io::XdrMemReader reader(contentsBuffer.get(), expectedSize);

	// The timestep should be correct
	uint64_t readTimestep;
	reader.read(readTimestep);

	REQUIRE(timestep == readTimestep);

	
	// Now iterate over the sites.
	datasource->Reset();
	while (datasource->ReadNext()) {
	  // Read the grid, which should be the same
	  LatticeVector grid = datasource->GetPosition();
	  unsigned x, y, z;
	  reader.read(x);
	  reader.read(y);
	  reader.read(z);

	  REQUIRE(grid == LatticeVector{x, y, z});

	  // Read the pressure, which should be an offset of the
	  // reference pressure away.
	  float pressure;
	  reader.read(pressure);

	  REQUIRE(Approx(datasource->GetPressure()).epsilon(epsilon) == (REFERENCE_PRESSURE_mmHg + double{pressure}));

	  // Read the velocity and compare
	  auto velocity = datasource->GetVelocity();
	  float vx, vy, vz;
	  reader.read(vx);
	  reader.read(vy);
	  reader.read(vz);

	  REQUIRE(apprx(velocity.x()) == vx);
          REQUIRE(apprx(velocity.y()) == vy);
	  REQUIRE(apprx(velocity.z()) == vz);
	}

      }
    }

    TEST_CASE_METHOD(helpers::HasCommsTestFixture, "LocalPropertyOutput") {
      //extraction::LocalPropertyOutput* propertyWriter = nullptr;
      
      char writtenMainHeader[io::formats::extraction::MainHeaderLength];
      constexpr size_t fieldHeaderLength = 0x34;
      char writtenFieldHeader[fieldHeaderLength];

      // The code won't overwrite any existing file
      std::remove(tempXtrFileName);
      std::remove(tempOffFileName);

      auto simpleOutFile = extraction::PropertyOutputFile{tempXtrFileName, 100, util::make_clone_ptr<extraction::WholeGeometrySelector>()};

      extraction::OutputField pressure{"Pressure", extraction::source::Pressure{}, float{0}, 1, {REFERENCE_PRESSURE_mmHg}};
      simpleOutFile.fields.push_back(pressure);

      extraction::OutputField velocity{"Velocity", extraction::source::Velocity{}, float{0}, 0};
      simpleOutFile.fields.push_back(velocity);

      auto simpleDataSource = std::make_unique<DummyDataSource>();

      SECTION("StringWrittenLength") {
	// Check the zero length string
	std::string s0;
	REQUIRE(4U == io::formats::extraction::GetStoredLengthOfString(s0));

	// This should have no padding
	std::string s1("Fish");
	REQUIRE(8U == io::formats::extraction::GetStoredLengthOfString(s1));

	// This must be padded up to 8 bytes
	std::string s2("A");
	REQUIRE(8U == io::formats::extraction::GetStoredLengthOfString(s2));

      }

      SECTION("Header length calculation") {
	auto hl = [](extraction::OutputField const& f) {
	  return io::formats::extraction::GetFieldHeaderLength(f.name, f.noffsets, extraction::code::type_to_enum(f.typecode));
	};

	int const hlen = hl(pressure) + hl(velocity);
	REQUIRE(hlen == fieldHeaderLength);
      }

      SECTION("Write") {
	// Create the writer object; this should write the headers.
	auto propertyWriter = std::make_unique<extraction::LocalPropertyOutput>(*simpleDataSource, simpleOutFile, Comms());
	// Open the file
	auto writtenFile = io::FILE::open(simpleOutFile.filename, "r");
	
	// Assert that the file is there
	//REQUIRE(writtenFile != nullptr);

	// Read the main header.
	size_t nRead = writtenFile.read(writtenMainHeader,
				  1,
				  io::formats::extraction::MainHeaderLength);
	// Check we read enough
	REQUIRE(size_t{io::formats::extraction::MainHeaderLength} == nRead);

	// Check it matches
	const char expectedMainHeader[] =
	  "\x68\x6C\x62\x21"
	  "\x78\x74\x72\x04"
	  "\x00\x00\x00\x05"
	  "\x3F\x33\xA9\x2A"
	  "\x30\x55\x32\x61"
	  "\x3F\xA1\x68\x72"
	  "\xB0\x20\xC4\x9C"
	  "\x3F\x50\x62\x4D"
	  "\xD2\xF1\xA9\xFC"
	  "\x3F\xB2\xF1\xA9"
	  "\xFB\xE7\x6C\x8B"
	  "\x00\x00\x00\x00"
	  "\x00\x00\x00\x40"
	  "\x00\x00\x00\x02"
	  "\x00\x00\x00\x34";
	for (int i = 0; i < io::formats::extraction::MainHeaderLength; ++i) {
	  INFO(i);
	  REQUIRE(expectedMainHeader[i] == writtenMainHeader[i]);
	}

	// Read the field header.
	nRead = writtenFile.read(writtenFieldHeader, 1, fieldHeaderLength);

	// Check we read enough
	REQUIRE(fieldHeaderLength == nRead);

	const char expectedFieldHeader[] =
          "\x00\x00\x00\x08"  // name length
	  "Pressure" // name
	  "\x00\x00\x00\x01"  // n values
	  "\x00\x00\x00\x00"  // type code
	  "\x00\x00\x00\x01"  // n offsets
	  "\x41\x00\x00\x00"  // offset

	  "\x00\x00\x00\x08"  // name length
	  "Velocity" // name
	  "\x00\x00\x00\x03"  // n values
	  "\x00\x00\x00\x00"  // type code
	  "\x00\x00\x00\x00"  // n offsets
	  ;

	// +1 for null terminator
	REQUIRE(sizeof(expectedFieldHeader) == fieldHeaderLength + 1);

	for (size_t i = 0; i < fieldHeaderLength; ++i) {
	  CAPTURE(i);
	  REQUIRE(expectedFieldHeader[i] == writtenFieldHeader[i]);
	}

	// Now going to write the body.
	// Create some pseudo-random data
	simpleDataSource->FillFields();
	// Write it
	propertyWriter->Write(0, 9999);

	CheckDataWriting(simpleDataSource.get(), 0, writtenFile);

	// Get some new data
	simpleDataSource->FillFields();
	// This should NOT write
	propertyWriter->Write(10, 9999);
	// This SHOULD write
	propertyWriter->Write(100, 9999);

	// The previous call to CheckDataWriting() sets the EOF indicator in writtenFile,
	// the previous call to Write() ought to unset it but it isn't working properly in
	// my current version of libc.
	writtenFile.clearerr();

	CheckDataWriting(simpleDataSource.get(), 100, writtenFile);
      }


      // tearDown

      // remove temporary files
      std::remove(tempXtrFileName);
      std::remove(tempOffFileName);
      }

  }
}

