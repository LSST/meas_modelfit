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

#include "lsst/pex/config/python.h"
#include "lsst/meas/modelfit/AdaptiveImportanceSampler.h"
#include "lsst/afw/table/BaseRecord.h"
#include "lsst/afw/table/BaseTable.h"
#include "lsst/afw/table/Catalog.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace meas {
namespace modelfit {

using PyImportanceSamplerControl =
        py::class_<ImportanceSamplerControl>;
using PyAdaptiveImportanceSampler =
        py::class_<AdaptiveImportanceSampler, Sampler>;

void wrapAdaptiveImportanceSampler(lsst::cpputils::python::WrapperCollection &wrappers) {
    wrappers.wrapType(PyImportanceSamplerControl(wrappers.module, "ImportanceSamplerControl"), [](auto &mod, auto &cls) {
        cls.def(py::init<>());
        LSST_DECLARE_CONTROL_FIELD(cls, ImportanceSamplerControl, nSamples);
        LSST_DECLARE_CONTROL_FIELD(cls, ImportanceSamplerControl, nUpdateSteps);
        LSST_DECLARE_CONTROL_FIELD(cls, ImportanceSamplerControl, tau1);
        LSST_DECLARE_CONTROL_FIELD(cls, ImportanceSamplerControl, tau2);
        LSST_DECLARE_CONTROL_FIELD(cls, ImportanceSamplerControl, targetPerplexity);
        LSST_DECLARE_CONTROL_FIELD(cls, ImportanceSamplerControl, maxRepeat);

        PyAdaptiveImportanceSampler clsAdaptiveImportanceSampler(mod, "AdaptiveImportanceSampler");
        clsAdaptiveImportanceSampler.def(py::init<afw::table::Schema &, std::shared_ptr<afw::math::Random>,
                                                 std::map<int, ImportanceSamplerControl> const &, bool>(),
                                         "sampleSchema"_a, "rng"_a, "ctrls"_a, "doSaveIteration"_a = false);
        // virtual run method already wrapped by Sampler base class
        clsAdaptiveImportanceSampler.def("computeNormalizedPerplexity",
                                         &AdaptiveImportanceSampler::computeNormalizedPerplexity);
        clsAdaptiveImportanceSampler.def("computeEffectiveSampleSizeFraction",
                                         &AdaptiveImportanceSampler::computeEffectiveSampleSizeFraction);
    });
}

}  // namespace modelfit
}  // namespace meas
}  // namespace lsst
