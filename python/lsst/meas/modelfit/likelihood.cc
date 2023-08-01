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

#include "lsst/meas/modelfit/Likelihood.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace meas {
namespace modelfit {

using PyLikelihood = py::class_<Likelihood>;

void wrapLikelihood(lsst::cpputils::python::WrapperCollection &wrappers) {
    // py::module::import("lsst.meas.modelfit.model");
    wrappers.wrapType(PyLikelihood(wrappers.module, "Likelihood"), [](auto &mod, auto &cls) {
        cls.def("getDataDim", &Likelihood::getDataDim);
        cls.def("getAmplitudeDim", &Likelihood::getAmplitudeDim);
        cls.def("getNonlinearDim", &Likelihood::getNonlinearDim);
        cls.def("getFixedDim", &Likelihood::getFixedDim);
        cls.def("getFixed", &Likelihood::getFixed);
        cls.def("getData", &Likelihood::getData);
        cls.def("getUnweightedData", &Likelihood::getUnweightedData);
        cls.def("getWeights", &Likelihood::getWeights);
        cls.def("getVariance", &Likelihood::getVariance);
        cls.def("getModel", &Likelihood::getModel);
        cls.def("computeModelMatrix", &Likelihood::computeModelMatrix, "modelMatrix"_a, "nonlinear"_a,
                "doApplyWeights"_a = true);
    });
}

}  // namespace modelfit
}  // namespace meas
}  // namespace lsst
