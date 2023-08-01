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
#include "pybind11/eigen.h"
#include "pybind11/stl.h"

#include "ndarray/pybind11.h"

#include "lsst/meas/modelfit/TruncatedGaussian.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace meas {
namespace modelfit {
namespace {

using Sampler = TruncatedGaussianSampler;
using Evaluator = TruncatedGaussianEvaluator;
using LogEvaluator = TruncatedGaussianLogEvaluator;

using PyTruncatedGaussian = py::class_<TruncatedGaussian>;
using PySampler = py::class_<Sampler>;
using PyEvaluator = py::class_<Evaluator>;
using PyLogEvaluator = py::class_<LogEvaluator>;

// Shared wrapper code for the TruncatedGaussianLogEvaluator and
// TruncatedGaussianEvaluator classes, which have the exact same interface.
// 'name' should be one of ("Evaluator", "LogEvaluator").
template<typename Class, typename PyClass>
PyClass declareEvaluator(lsst::cpputils::python::WrapperCollection &wrappers, std::string const &name) {
        return wrappers.wrapType(
                PyClass(wrappers.module, ("TruncatedGaussian" + name).c_str()),
                [](auto &mod, auto &cls) {
                    cls.def(py::init<TruncatedGaussian const &>(), "parent"_a);
                    cls.def("__call__",
                            (Scalar (Class::*)(
                                    ndarray::Array<Scalar const, 1, 1> const &) const) &Class::operator(),
                            "alpha"_a);
                    cls.def("__call__", (void (Class::*)(ndarray::Array<Scalar const, 2, 1> const &,
                                                         ndarray::Array<Scalar, 1, 1> const &) const) &
                                    Class::operator(),
                            "alpha"_a, "output"_a);
                    // Third overload of operator() is just an Eigen version of the ndarray one, so it's
                    // redundant in Python.
                });
}

PySampler declareSampler(lsst::cpputils::python::WrapperCollection &wrappers) {
    return wrappers.wrapType(PySampler(wrappers.module, "TruncatedGaussianSampler"), [](auto &mod, auto &cls) {
        cls.def(py::init<TruncatedGaussian const &, TruncatedGaussian::SampleStrategy>(), "parent"_a,
                       "strategy"_a);
        cls.def("__call__",
                       (Scalar (Sampler::*)(afw::math::Random &, ndarray::Array<Scalar, 1, 1> const &) const) &
                               Sampler::operator(),
                       "rng"_a, "alpha"_a);
        cls.def("__call__", (void (Sampler::*)(afw::math::Random &, ndarray::Array<Scalar, 2, 1> const &,
                                                      ndarray::Array<Scalar, 1, 1> const &, bool) const) &
                               Sampler::operator(),
                       "rng"_a, "alpha"_a, "weights"_a, "multiplyWeights"_a = false);
    });
}

using PySampleStrategy = py::enum_<TruncatedGaussian::SampleStrategy>;

PySampleStrategy declareSampleStrategy(lsst::cpputils::python::WrapperCollection &wrappers) {
    return wrappers.wrapType(PySampleStrategy(wrappers.module, "SampleStrategy"), [](auto &mod, auto &enm) {
        enm.value("DIRECT_WITH_REJECTION", TruncatedGaussian::DIRECT_WITH_REJECTION);
        enm.value("ALIGN_AND_WEIGHT", TruncatedGaussian::ALIGN_AND_WEIGHT);
        enm.export_values();
    });
}

PyTruncatedGaussian declareTruncatedGaussian(lsst::cpputils::python::WrapperCollection &wrappers) {
    return wrappers.wrapType(
            PyTruncatedGaussian(wrappers.module, "TruncatedGaussian"), [](auto &mod, auto &cls) {
                cls.def_static("fromSeriesParameters", &TruncatedGaussian::fromSeriesParameters, "q0"_a, "gradient"_a,
                               "hessian"_a);
                cls.def_static("fromStandardParameters", &TruncatedGaussian::fromStandardParameters, "mean"_a,
                               "covariance"_a);
                cls.def("sample", (Sampler (TruncatedGaussian::*)(TruncatedGaussian::SampleStrategy) const) &
                                TruncatedGaussian::sample,
                        "strategy"_a);
                cls.def("sample", (Sampler (TruncatedGaussian::*)(Scalar) const) &TruncatedGaussian::sample,
                        "minRejectionEfficiency"_a = 0.1);
                cls.def("evaluateLog", &TruncatedGaussian::evaluateLog);
                cls.def("evaluate", &TruncatedGaussian::evaluate);
                cls.def("getDim", &TruncatedGaussian::getDim);
                cls.def("maximize", &TruncatedGaussian::maximize);
                cls.def("getUntruncatedFraction", &TruncatedGaussian::getUntruncatedFraction);
                cls.def("getLogPeakAmplitude", &TruncatedGaussian::getLogPeakAmplitude);
                cls.def("getLogIntegral", &TruncatedGaussian::getLogIntegral);
            });
}

}

void wrapTruncatedGaussian(lsst::cpputils::python::WrapperCollection &wrappers) {
    declareSampleStrategy(wrappers);
    auto clsTruncatedGaussian = declareTruncatedGaussian(wrappers);
    clsTruncatedGaussian.attr("LogEvaluator") = declareEvaluator<LogEvaluator, PyLogEvaluator>(wrappers, "LogEvaluator");
    clsTruncatedGaussian.attr("Evaluator") = declareEvaluator<Evaluator, PyEvaluator>(wrappers, "Evaluator");
    clsTruncatedGaussian.attr("Sampler") = declareSampler(wrappers);
}

}  // namespace modelfit
}  // namespace meas
}  // namespace lsst
