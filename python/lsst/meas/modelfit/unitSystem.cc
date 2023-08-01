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

#include "lsst/meas/modelfit/UnitSystem.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace meas {
namespace modelfit {

using PyUnitSystem = py::class_<UnitSystem>;
using PyLocalUnitTransform = py::class_<LocalUnitTransform>;

void wrapUnitSystem(lsst::cpputils::python::WrapperCollection &wrappers) {
    // Data member wrappers in this module are intentionally read-only;
    // setting them in Python should never be necessary and hence always
    // represents a mistake we want to fail on as early as possible.
    wrappers.wrapType(PyUnitSystem(wrappers.module, "UnitSystem"), [](auto &mod, auto &cls) {
        cls.def_readonly("wcs", &UnitSystem::wcs);
        cls.def_readonly("photoCalib", &UnitSystem::photoCalib);
        cls.def(
                py::init<geom::SpherePoint const &, std::shared_ptr<afw::image::PhotoCalib const>, double>(),
                "position"_a, "calibIn"_a, "flux"_a);
        cls.def(py::init<geom::SpherePoint const &, Scalar>(), "position"_a, "mag"_a);
        cls.def(
                py::init<std::shared_ptr<afw::geom::SkyWcs const>, std::shared_ptr<afw::image::PhotoCalib const>>(),
                "wcs"_a, "photoCalib"_a);
        cls.def(py::init<afw::image::Exposure<float> const &>(), "exposure"_a);
        cls.def(py::init<afw::image::Exposure<double> const &>(), "exposure"_a);
    });

    wrappers.wrapType(PyLocalUnitTransform(wrappers.module, "LocalUnitTransform"), [](auto &mod, auto &cls) {
        cls.def_readonly("geometric", &LocalUnitTransform::geometric);
        cls.def_readonly("flux", &LocalUnitTransform::flux);
        cls.def_readonly("sb", &LocalUnitTransform::sb);
        cls.def(py::init<geom::Point2D const &, UnitSystem const &, UnitSystem const &>(),
                                  "sourcePixel"_a, "source"_a, "destination"_a);
        cls.def(py::init<>());
    });
}

}  // namespace modelfit
}  // namespace meas
}  // namespace lsst
