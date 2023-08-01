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
#include "pybind11/stl.h"

#include "ndarray/pybind11.h"

#include "lsst/pex/config/python.h"
#include "lsst/meas/modelfit/UnitTransformedLikelihood.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace meas {
namespace modelfit {

using PyUnitTransformedLikelihoodControl =
        py::class_<UnitTransformedLikelihoodControl>;

using PyEpochFootprint = py::class_<EpochFootprint>;

using PyUnitTransformedLikelihood =
        py::class_<UnitTransformedLikelihood, Likelihood>;

void wrapUnitTransformedLikelihood(lsst::cpputils::python::WrapperCollection &wrappers) {
    wrappers.wrapType(
            PyUnitTransformedLikelihoodControl(wrappers.module, "UnitTransformedLikelihoodControl"),
            [](auto &mod, auto &cls) {
                LSST_DECLARE_CONTROL_FIELD(cls, UnitTransformedLikelihoodControl, usePixelWeights);
                LSST_DECLARE_CONTROL_FIELD(cls, UnitTransformedLikelihoodControl, weightsMultiplier);
                cls.def(py::init<bool>(), "usePixelWeights"_a = false);
            });
    wrappers.wrapType(PyEpochFootprint(wrappers.module, "EpochFootprint"), [](auto &mod, auto &cls) {
        cls.def(py::init<afw::detection::Footprint const &, afw::image::Exposure<Pixel> const &,
                                      shapelet::MultiShapeletFunction const &>(),
                              "footprint"_a, "exposure"_a, "psf"_a);
        cls.def_readonly("footprint", &EpochFootprint::footprint);
        cls.def_readonly("exposure", &EpochFootprint::exposure);
        cls.def_readonly("psf", &EpochFootprint::psf);
    });

    wrappers.wrapType(PyUnitTransformedLikelihood(wrappers.module, "UnitTransformedLikelihood"),[](auto &mod, auto &cls) {
        cls.def(
                py::init<std::shared_ptr<Model>, ndarray::Array<Scalar const, 1, 1> const &, UnitSystem const &,
                        geom::SpherePoint const &, afw::image::Exposure<Pixel> const &,
                        afw::detection::Footprint const &, shapelet::MultiShapeletFunction const &,
                        UnitTransformedLikelihoodControl const &>(),
                "model"_a, "fixed"_a, "fitSys"_a, "position"_a, "exposure"_a, "footprint"_a, "psf"_a,
                "ctrl"_a);
        cls.def(
                py::init<std::shared_ptr<Model>, ndarray::Array<Scalar const, 1, 1> const &, UnitSystem const &,
                        geom::SpherePoint const &, std::vector<std::shared_ptr<EpochFootprint>> const &,
                        UnitTransformedLikelihoodControl const &>(),
                "model"_a, "fixed"_a, "fitSys"_a, "position"_a, "epochFootprintList"_a, "ctrl"_a);
    });
}

}  // namespace modelfit
}  // namespace meas
}  // namespace lsst
