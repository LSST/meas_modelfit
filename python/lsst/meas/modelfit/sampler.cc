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

#include "lsst/meas/modelfit/Sampler.h"
#include "lsst/afw/table/BaseRecord.h"
#include "lsst/afw/table/BaseTable.h"
#include "lsst/afw/table/Catalog.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace meas {
namespace modelfit {

using PySamplingObjective = py::class_<SamplingObjective>;
using PySampler = py::class_<Sampler>;

void wrapSampler(lsst::cpputils::python::WrapperCollection &wrappers) {
    wrappers.wrapType(PySamplingObjective(wrappers.module, "SamplingObjective"), [](auto &mod, auto &cls) {
        cls.def("getParameterDim", &SamplingObjective::getParameterDim);
        cls.def("__call__", &SamplingObjective::operator(), "parameters"_a, "sample"_a);
    });
    wrappers.wrapType(PySampler(wrappers.module, "Sampler"), [](auto &mod, auto &cls) {
        cls.def("run", &Sampler::run, "objective"_a, "proposal"_a, "samples"_a);
    });
}

}  // namespace modelfit
}  // namespace meas
}  // namespace lsst
