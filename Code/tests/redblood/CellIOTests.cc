// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#include <catch2/catch.hpp>
#include <tinyxml.h>

#include "configuration/SimConfig.h"
#include "configuration/SimBuilder.h"
#include "redblood/CellControllerBuilder.h"
#include "redblood/MeshIO.h"
#include "redblood/FaderCell.h"

#include "tests/helpers/FolderTestFixture.h"
#include "tests/helpers/SimConfBuildHelp.h"

template <typename To, typename From>
auto dynamic_unique_cast(std::unique_ptr<From>&& src) {
    From* src_raw = src.release();
    To* dest_raw = dynamic_cast<To*>(src_raw);
    if (dest_raw) {
        // Worked!
    } else {
        // Fail - need to do something with the resource, guess we assign back to src
        src.reset(src_raw);
    }
    return std::unique_ptr<To>(dest_raw);
}

namespace hemelb::tests
{
    using namespace redblood;

    TEST_CASE_METHOD(helpers::FolderTestFixture, "CellIOTests", "[redblood]") {
        TiXmlDocument doc;
        LatticeDistance scale;

        auto converter = std::make_shared<util::UnitConverter>(0.5, 0.6, LatticePosition(1, 2, 3), 1000.0, 0.0);

        CopyResourceToTempdir("red_blood_cell.txt");
        CopyResourceToTempdir("empty_for_relative_paths.xml");
        auto config = std::make_unique<UninitSimConfig>("empty_for_relative_paths.xml");
        auto builder = UninitSimBuilder(*config, converter);
        auto cell_builder = CellControllerBuilder(converter);
        {
            // It seems TiXML might take care of deallocation
            auto const parent = new TiXmlElement("parent");
            doc.LinkEndChild(parent);
            auto const cell = new TiXmlElement("cell");
            auto const shape = new TiXmlElement("shape");
            shape->SetAttribute("mesh_path", "red_blood_cell.txt");
            shape->SetAttribute("mesh_format", "Krueger");
            cell->LinkEndChild(shape);
            auto const scaleXML = new TiXmlElement("scale");
            scale = 1.5;
            scaleXML->SetDoubleAttribute("value", scale);
            scaleXML->SetAttribute("units", "m");
            cell->LinkEndChild(scaleXML);

            auto const moduli = new TiXmlElement("moduli");
            auto add_stuff = [moduli](std::string const& name, std::string const& units, Dimensionless value) {
                auto elem = new TiXmlElement(name);
                elem->SetDoubleAttribute("value", value);
                elem->SetAttribute("units", units);
                moduli->LinkEndChild(elem);
            };
            add_stuff("surface", "lattice", 2e0);
            add_stuff("dilation", "lattice", 0.58);
            add_stuff("bending", "Nm", 2e-18);
            cell->LinkEndChild(moduli);

            parent->LinkEndChild(cell);
        }

        auto approx = Approx(0.0).margin(1e-12);

        // Reads cell with minimum stuff
        SECTION("testReadCellWithDefaults") {
            // Remove moduli, so we get default behavior
            auto cellEl = doc.FirstChildElement("parent")->FirstChildElement("cell");
            cellEl->RemoveChild(cellEl->FirstChildElement("moduli"));

            auto cellConf = config->readCell(cellEl);
            auto cell = dynamic_unique_cast<Cell const>(cell_builder.build_cell(cellConf));
            REQUIRE(cell);
            auto const kruegerIO = redblood::KruegerMeshIO{};
            auto const data = kruegerIO.readFile("red_blood_cell.txt", true);
            REQUIRE(static_cast<site_t>(data->vertices.size()) == cell->GetNumberOfNodes());
            REQUIRE(approx(converter->ConvertToLatticeUnits("m", scale)) == cell->GetScale());
            REQUIRE(approx(1e0) == cell->moduli.volume);
            REQUIRE(approx(1e0) == cell->moduli.surface);
            REQUIRE(approx(converter->ConvertToLatticeUnits("Nm", 2e-19)) == cell->moduli.bending);
            REQUIRE(approx(0.75) == cell->moduli.dilation);
            REQUIRE(approx(converter->ConvertToLatticeUnits("N/m", 5e-6)) == cell->moduli.strain);
        }

        SECTION("testReadCellModuli") {
            auto cellConf = config->readCell(doc.FirstChildElement("parent")->FirstChildElement("cell"));
            auto const moduli = cell_builder.build_cell_moduli(cellConf.moduli);
            REQUIRE(approx(1e0) == moduli.volume);
            REQUIRE(approx(2e0) == moduli.surface);
            REQUIRE(approx(converter->ConvertToLatticeUnits("Nm", 2e-18)) == moduli.bending);
            REQUIRE(approx(0.58) == moduli.dilation);
            REQUIRE(approx(converter->ConvertToLatticeUnits("N/m", 5e-6)) == moduli.strain);
        }

        SECTION("testReadMeshTemplates") {
            const char* xml_text = "<parent>"
                                   "  <inlets>"
                                   "   <inlet>"
                                   "     <condition type=\"pressure\" subtype=\"cosine\">"
                                   "       <amplitude value=\"0\" units=\"mmHg\" />"
                                   "       <mean value=\"0\" units=\"mmHg\" />"
                                   "       <phase value=\"0\" units=\"rad\" />"
                                   "       <period value=\"1\" units=\"s\" />"
                                   "     </condition>"
                                   "     <normal units=\"dimensionless\" value=\"(0.0,0.0,1.0)\" />"
                                   "     <position units=\"m\" value=\"(0.0,0.0,-0.024)\" />"
                                   "     <flowextension>"
                                   "       <length units=\"m\" value=\"0.1\" />"
                                   "       <radius units=\"m\" value=\"0.01\" />"
                                   "       <fadelength units=\"m\" value=\"0.05\" />"
                                   "     </flowextension>"
                                   "   </inlet>"
                                   "  </inlets>"
                                   "  <outlets>"
                                   "    <outlet>"
                                   "     <condition type=\"pressure\" subtype=\"cosine\">"
                                   "       <amplitude value=\"0\" units=\"mmHg\" />"
                                   "       <mean value=\"0\" units=\"mmHg\" />"
                                   "       <phase value=\"0\" units=\"rad\" />"
                                   "       <period value=\"1\" units=\"s\" />"
                                   "     </condition>"
                                   "      <normal units=\"dimensionless\" value=\"(0.0,0.0,-1.0)\" />"
                                   "      <position units=\"m\" value=\"(0.0,0.0,0.024)\" />"
                                   "      <flowextension>"
                                   "        <length units=\"m\" value=\"0.1\" />"
                                   "        <radius units=\"m\" value=\"0.01\" />"
                                   "        <fadelength units=\"m\" value=\"0.05\" />"
                                   "      </flowextension>"
                                   "    </outlet>"
                                   "  </outlets>"
                                   "  <redbloodcells>"
                                   "    <cells>"
                                   "      <cell>"
                                   "        <shape mesh_path=\"red_blood_cell.txt\" mesh_format=\"Krueger\" />"
                                   "        <scale units=\"m\" value=\"0.6\"/>"
                                   "      </cell>"
                                   "     <cell name=\"joe\">"
                                   "       <shape mesh_path=\"red_blood_cell.txt\" mesh_format=\"Krueger\" />"
                                   "       <scale units=\"m\" value=\"0.5\"/>"
                                   "     </cell>"
                                   "   </cells>"
                                   "  </redbloodcells>"
                                   "</parent>";
            TiXmlDocument document;
            document.Parse(xml_text);
            auto root = document.FirstChildElement("parent");
            auto tc_conf = config->readTemplateCells(root->FirstChildElement("redbloodcells")->FirstChildElement("cells"));
            auto in_conf = config->DoIOForInOutlets(root->FirstChildElement("inlets"));
            auto out_conf = config->DoIOForInOutlets(root->FirstChildElement("outlets"));
            auto ins = builder.BuildIolets(in_conf);
            auto outs = builder.BuildIolets(out_conf);

            auto cells = cell_builder.build_template_cells(
                    tc_conf, configuration::MakeCountedIoletView(ins), configuration::MakeCountedIoletView(outs)
            );
            REQUIRE(size_t(2) == cells->size());
            REQUIRE(size_t(1) == cells->count("default"));
            REQUIRE(size_t(1) == cells->count("joe"));
            auto const default_ = std::static_pointer_cast<FaderCell>( (*cells)["default"]);
            auto const joe = std::static_pointer_cast<FaderCell>( (*cells)["joe"]);
            REQUIRE(default_->GetTemplateName() == "default");
            REQUIRE(joe->GetTemplateName() == "joe");
            REQUIRE(Approx(converter->ConvertToLatticeUnits("m", 0.6)).margin(1e-8)
                    == default_->GetScale());
            REQUIRE(Approx(converter->ConvertToLatticeUnits("m", 0.5)).margin(1e-8)
                    == joe->GetScale());
            REQUIRE(static_cast<bool>(default_->GetIOlets()));
            REQUIRE(static_cast<bool>(joe->GetIOlets()));
            REQUIRE(default_->GetIOlets() == joe->GetIOlets());
            REQUIRE(size_t(2) == joe->GetIOlets()->size());
        }
    }
}
