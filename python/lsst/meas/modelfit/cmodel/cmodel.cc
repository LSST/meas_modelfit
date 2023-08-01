// -*- lsst-c++ -*-
/*
 * LSST Data Management System
 * Copyright 2008-2013 LSST Corporation.
 *
 * This product includes software developed by the
 * LSST Project (http://www.lsst.org/).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the LSST License Statement and
 * the GNU General Public License along with this program.  If not,
 * see <http://www.lsstcorp.org/LegalNotices/>.
 */

#include "pybind11/pybind11.h"
#include "lsst/cpputils/python.h"

#include "ndarray/pybind11.h"

#include "lsst/pex/config/python.h"
#include "lsst/meas/modelfit/CModel.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace meas {
namespace modelfit {
namespace {

using PyCModelStageControl = py::class_<CModelStageControl>;
using PyCModelControl = py::class_<CModelControl>;
using PyCModelStageResult = py::class_<CModelStageResult>;
using PyCModelResult = py::class_<CModelResult>;
using PyCModelAlgorithm = py::class_<CModelAlgorithm>;

PyCModelStageControl declareCModelStageControl(lsst::cpputils::python::WrapperCollection &wrappers) {
    return wrappers.wrapType(PyCModelStageControl(wrappers.module, "CModelStageControl"), [](auto &mod, auto &cls) {
        cls.def(py::init<>());
        cls.def("getProfile", &CModelStageControl::getProfile);
        cls.def("getModel", &CModelStageControl::getModel);
        cls.def("getPrior", &CModelStageControl::getPrior);
        LSST_DECLARE_CONTROL_FIELD(cls, CModelStageControl, profileName);
        LSST_DECLARE_CONTROL_FIELD(cls, CModelStageControl, priorSource);
        LSST_DECLARE_CONTROL_FIELD(cls, CModelStageControl, priorName);
        LSST_DECLARE_NESTED_CONTROL_FIELD(cls, CModelStageControl, linearPriorConfig);
        LSST_DECLARE_NESTED_CONTROL_FIELD(cls, CModelStageControl, empiricalPriorConfig);
        LSST_DECLARE_CONTROL_FIELD(cls, CModelStageControl, nComponents);
        LSST_DECLARE_CONTROL_FIELD(cls, CModelStageControl, maxRadius);
        LSST_DECLARE_CONTROL_FIELD(cls, CModelStageControl, usePixelWeights);
        LSST_DECLARE_CONTROL_FIELD(cls, CModelStageControl, weightsMultiplier);
        LSST_DECLARE_NESTED_CONTROL_FIELD(cls, CModelStageControl, optimizer);
        LSST_DECLARE_CONTROL_FIELD(cls, CModelStageControl, doRecordHistory);
        LSST_DECLARE_CONTROL_FIELD(cls, CModelStageControl, doRecordTime);
    });
}

PyCModelControl declareCModelControl(lsst::cpputils::python::WrapperCollection &wrappers) {
    return wrappers.wrapType(PyCModelControl(wrappers.module, "CModelControl"), [](auto &mod, auto &cls) {
        cls.def(py::init<>());
        LSST_DECLARE_CONTROL_FIELD(cls, CModelControl, psfName);
        LSST_DECLARE_NESTED_CONTROL_FIELD(cls, CModelControl, region);
        LSST_DECLARE_NESTED_CONTROL_FIELD(cls, CModelControl, initial);
        LSST_DECLARE_NESTED_CONTROL_FIELD(cls, CModelControl, exp);
        LSST_DECLARE_NESTED_CONTROL_FIELD(cls, CModelControl, dev);
        LSST_DECLARE_CONTROL_FIELD(cls, CModelControl, minInitialRadius);
        LSST_DECLARE_CONTROL_FIELD(cls, CModelControl, fallbackInitialMomentsPsfFactor);
    });
}

// Custom wrapper for views to std::bitset.
template <int N>
class BitSetView {
public:
    explicit BitSetView(std::bitset<N> const *target) : _target(target) {}

    bool operator[](int i) const { return (*_target)[i]; }

    constexpr std::size_t size() const { return N; }

    template <typename PyParent>
    static void declare(PyParent &parent) {
        py::class_<BitSetView<N>> cls(parent, ("BitSetView" + std::to_string(N)).c_str());
        cls.def("__getitem__", &BitSetView<N>::operator[]);
        cls.def("__len__", &BitSetView<N>::size);
    }

private:
    std::bitset<N> const *_target;
};

PyCModelStageResult declareCModelStageResult(lsst::cpputils::python::WrapperCollection &wrappers) {
    return wrappers.wrapType(PyCModelStageResult(wrappers.module, "CModelStageResult"), [](auto &mod, auto &cls) {
        cls.def(py::init<>());
        cls.attr("FAILED") = py::cast(int(CModelStageResult::FAILED));
        cls.attr("TR_SMALL") = py::cast(int(CModelStageResult::TR_SMALL));
        cls.attr("MAX_ITERATIONS") = py::cast(int(CModelStageResult::MAX_ITERATIONS));
        cls.attr("NUMERIC_ERROR") = py::cast(int(CModelStageResult::NUMERIC_ERROR));
        cls.attr("BAD_REFERENCE") = py::cast(int(CModelStageResult::BAD_REFERENCE));
        cls.attr("NO_FLUX") = py::cast(int(CModelStageResult::NO_FLUX));
        cls.attr("N_FLAGS") = py::cast(int(CModelStageResult::N_FLAGS));

        // Data members are intentionally read-only from the Python side;
        // they should only be set by the C++ algorithm code that uses
        // this class to communicate its outputs.
        cls.def_readonly("model", &CModelStageResult::model);
        cls.def_readonly("prior", &CModelStageResult::prior);
        cls.def_readonly("objfunc", &CModelStageResult::objfunc);
        cls.def_readonly("likelihood", &CModelStageResult::likelihood);
        cls.def_readonly("instFlux", &CModelStageResult::instFlux);
        cls.def_readonly("instFluxErr", &CModelStageResult::instFluxErr);
        cls.def_readonly("instFluxInner", &CModelStageResult::instFluxInner);
        cls.def_readonly("objective", &CModelStageResult::objective);
        cls.def_readonly("time", &CModelStageResult::time);
        cls.def_readonly("ellipse", &CModelStageResult::ellipse);
        cls.def_readonly("nonlinear", &CModelStageResult::nonlinear);
        cls.def_readonly("amplitudes", &CModelStageResult::amplitudes);
        cls.def_readonly("fixed", &CModelStageResult::fixed);

        // Declare wrappers for a view class for the flags attribute
        BitSetView<CModelStageResult::N_FLAGS>::declare(cls);
        // Wrap the flag. attributes
        cls.def_property_readonly(
                "flags",
                [](CModelStageResult const &self) { return BitSetView<CModelStageResult::N_FLAGS>(&self.flags); },
                py::return_value_policy::reference_internal);
    });
}

PyCModelResult declareCModelResult(lsst::cpputils::python::WrapperCollection &wrappers) {
    return wrappers.wrapType(PyCModelResult(wrappers.module, "CModelResult"), [](auto &mod, auto &cls) {
        cls.def(py::init<>());

        cls.attr("FAILED") = py::cast(int(CModelResult::FAILED));
        cls.attr("REGION_MAX_AREA") = py::cast(int(CModelResult::REGION_MAX_AREA));
        cls.attr("REGION_MAX_BAD_PIXEL_FRACTION") = py::cast(int(CModelResult::REGION_MAX_BAD_PIXEL_FRACTION));
        cls.attr("REGION_USED_FOOTPRINT_AREA") = py::cast(int(CModelResult::REGION_USED_FOOTPRINT_AREA));
        cls.attr("REGION_USED_PSF_AREA") = py::cast(int(CModelResult::REGION_USED_PSF_AREA));
        cls.attr("REGION_USED_INITIAL_ELLIPSE_MIN") =
                py::cast(int(CModelResult::REGION_USED_INITIAL_ELLIPSE_MIN));
        cls.attr("REGION_USED_INITIAL_ELLIPSE_MAX") =
                py::cast(int(CModelResult::REGION_USED_INITIAL_ELLIPSE_MAX));
        cls.attr("NO_FLUX") = py::cast(int(CModelResult::NO_FLUX));

        // Data members are intentionally read-only from the Python side;
        // they should only be set by the C++ algorithm code that uses
        // this class to communicate its outputs.
        cls.def_readonly("instFlux", &CModelResult::instFlux);
        cls.def_readonly("instFluxErr", &CModelResult::instFluxErr);
        cls.def_readonly("instFluxInner", &CModelResult::instFluxInner);
        cls.def_readonly("fracDev", &CModelResult::fracDev);
        cls.def_readonly("objective", &CModelResult::objective);
        cls.def_readonly("initial", &CModelResult::initial);
        cls.def_readonly("exp", &CModelResult::exp);
        cls.def_readonly("dev", &CModelResult::dev);
        cls.def_readonly("initialFitRegion", &CModelResult::initialFitRegion);
        cls.def_readonly("finalFitRegion", &CModelResult::finalFitRegion);
        cls.def_readonly("fitSysToMeasSys", &CModelResult::fitSysToMeasSys);

        // Declare wrappers for a view class for the flags attribute
        BitSetView<CModelResult::N_FLAGS>::declare(cls);
        // Wrap the flag. attributes
        cls.def_property_readonly(
                "flags", [](CModelResult const &self) { return BitSetView<CModelResult::N_FLAGS>(&self.flags); },
                py::return_value_policy::reference_internal);
    });
}

PyCModelAlgorithm declareCModelAlgorithm(lsst::cpputils::python::WrapperCollection &wrappers) {
    return wrappers.wrapType(PyCModelAlgorithm(wrappers.module, "CModelAlgorithm"), [](auto &mod, auto &cls) {
        cls.def(py::init<std::string const &, CModelControl const &, afw::table::Schema &>(), "name"_a, "ctrl"_a,
                "schema"_a);
        cls.def(py::init<std::string const &, CModelControl const &, afw::table::SchemaMapper &>(), "name"_a,
                "ctrl"_a, "schemaMapper"_a);
        cls.def(py::init<CModelControl const &>(), "ctrl"_a);
        cls.def("getControl", &CModelAlgorithm::getControl);
        cls.def("apply", &CModelAlgorithm::apply, "exposure"_a, "psf"_a, "center"_a, "moments"_a,
                "approxFlux"_a = -1, "kronRadius"_a = -1, "footprintArea"_a = -1);
        cls.def("applyForced", &CModelAlgorithm::applyForced, "exposure"_a, "psf"_a, "center"_a, "reference"_a,
                "approxFlux"_a = -1);
        cls.def("measure", (void (CModelAlgorithm::*)(afw::table::SourceRecord &,
                                                      afw::image::Exposure<Pixel> const &) const) &
                        CModelAlgorithm::measure,
                "measRecord"_a, "exposure"_a);
        cls.def("measure",
                (void (CModelAlgorithm::*)(afw::table::SourceRecord &, afw::image::Exposure<Pixel> const &,
                                           afw::table::SourceRecord const &) const) &
                        CModelAlgorithm::measure,
                "measRecord"_a, "exposure"_a, "refRecord"_a);
        cls.def("fail", &CModelAlgorithm::fail, "measRecord"_a, "error"_a);
        cls.def("writeResultToRecord", &CModelAlgorithm::writeResultToRecord, "result"_a, "record"_a);
    });
}
}  // namespace

void wrapCmodel(lsst::cpputils::python::WrapperCollection &wrappers) {
    declareCModelStageControl(wrappers);
    auto clsControl = declareCModelControl(wrappers);
    declareCModelStageResult(wrappers);
    auto clsResult = declareCModelResult(wrappers);
    auto clsAlgorithm = declareCModelAlgorithm(wrappers);
    clsAlgorithm.attr("Control") = clsControl;
    clsAlgorithm.attr("Result") = clsResult;
}

}  // namespace modelfit
}  // namespace meas
}  // namespace lsst
