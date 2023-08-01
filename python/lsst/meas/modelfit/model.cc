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

#include "lsst/meas/modelfit/Model.h"
#include "lsst/meas/modelfit/Prior.h"
#include "lsst/meas/modelfit/UnitSystem.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace meas {
namespace modelfit {

using PyModel = py::class_<Model>;

void wrapModel(lsst::cpputils::python::WrapperCollection &wrappers) {
    wrappers.wrapType(PyModel(wrappers.module, "Model"), [](auto &mod, auto &cls) {
        py::enum_<Model::CenterEnum>(cls, "CenterEnum")
                .value("FIXED_CENTER", Model::FIXED_CENTER)
                .value("SINGLE_CENTER", Model::SINGLE_CENTER)
                .value("MULTI_CENTER", Model::MULTI_CENTER)
                .export_values();

        cls.def_static("make", (std::shared_ptr<Model>(*)(Model::BasisVector, Model::NameVector const &,
                                                          Model::CenterEnum)) &
                               Model::make,
                       "basisVector"_a, "prefixes"_a, "center"_a);
        cls.def_static("make", (std::shared_ptr<Model>(*)(std::shared_ptr<shapelet::MultiShapeletBasis>,
                                                          Model::CenterEnum)) &
                               Model::make,
                       "basis"_a, "center"_a);
        cls.def_static("makeGaussian", &Model::makeGaussian, "center"_a, "radius"_a = 1.0);
        cls.def("getNonlinearDim", &Model::getNonlinearDim);
        cls.def("getAmplitudeDim", &Model::getAmplitudeDim);
        cls.def("getFixedDim", &Model::getFixedDim);
        cls.def("getBasisCount", &Model::getBasisCount);
        cls.def("getNonlinearNames", &Model::getNonlinearNames, py::return_value_policy::copy);
        cls.def("getAmplitudeNames", &Model::getAmplitudeNames, py::return_value_policy::copy);
        cls.def("getFixedNames", &Model::getFixedNames, py::return_value_policy::copy);
        cls.def("getBasisVector", &Model::getBasisVector, py::return_value_policy::copy);
        cls.def("makeShapeletFunction", &Model::makeShapeletFunction);
        cls.def("adaptPrior", &Model::adaptPrior);
        cls.def("makeEllipseVector", &Model::makeEllipseVector);
        cls.def("writeEllipses",
                (Model::EllipseVector (Model::*)(ndarray::Array<Scalar const, 1, 1> const &,
                                                 ndarray::Array<Scalar const, 1, 1> const &) const) &
                        Model::writeEllipses,
                "nonlinear"_a, "fixed"_a);
        cls.def("readEllipses",
                (void (Model::*)(Model::EllipseVector const &, ndarray::Array<Scalar, 1, 1> const &,
                                 ndarray::Array<Scalar, 1, 1> const &) const) &
                        Model::readEllipses,
                "ellipses"_a, "nonlinear"_a, "fixed"_a);
        cls.def("transformParameters", &Model::transformParameters, "transform"_a, "nonlinear"_a, "amplitudes"_a,
                "fixed"_a);
    });
}

}  // namespace modelfit
}  // namespace meas
}  // namespace lsst
