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

#include "lsst/meas/modelfit/optimizer.h"
#include "lsst/meas/modelfit/Model.h"
#include "lsst/meas/modelfit/Likelihood.h"
#include "lsst/meas/modelfit/Prior.h"
#include "lsst/afw/table/Catalog.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace meas {
namespace modelfit {
namespace {

using PyOptimizerObjective = py::class_<OptimizerObjective>;
using PyOptimizerControl = py::class_<OptimizerControl>;
using PyOptimizerHistoryRecorder =
        py::class_<OptimizerHistoryRecorder>;
using PyOptimizer = py::class_<Optimizer>;

PyOptimizerObjective declareOptimizerObjective(lsst::cpputils::python::WrapperCollection &wrappers) {
    return wrappers.wrapType(PyOptimizerObjective(wrappers.module, "OptimizerObjective"), [](auto &mod, auto &cls) {
        // Class is abstract, so no constructor.
        cls.def_readonly("dataSize", &OptimizerObjective::dataSize);
        cls.def_readonly("parameterSize", &OptimizerObjective::parameterSize);
        cls.def_static("makeFromLikelihood", &OptimizerObjective::makeFromLikelihood, "likelihood"_a,
                       "prior"_a = nullptr);
        // class is abstract and not subclassable in Python, so we don't wrap the ctor
        cls.def("fillObjectiveValueGrid", &OptimizerObjective::fillObjectiveValueGrid, "parameters"_a,
                "output"_a);
        cls.def("computeResiduals", &OptimizerObjective::computeResiduals, "parameters"_a, "residuals"_a);
        cls.def("differentiateResiduals", &OptimizerObjective::differentiateResiduals, "parameters"_a,
                "derivatives"_a);
        cls.def("hasPrior", &OptimizerObjective::hasPrior);
        cls.def("computePrior", &OptimizerObjective::computePrior, "parameters"_a);
        cls.def("differentiatePrior", &OptimizerObjective::differentiatePrior, "parameters"_a, "gradient"_a,
                "hessian"_a);
    });
}

PyOptimizerControl declareOptimizerControl(lsst::cpputils::python::WrapperCollection &wrappers) {
    return wrappers.wrapType(PyOptimizerControl(wrappers.module, "OptimizerControl"), [](auto &mod, auto &cls) {
        LSST_DECLARE_CONTROL_FIELD(cls, OptimizerControl, noSR1Term);
        LSST_DECLARE_CONTROL_FIELD(cls, OptimizerControl, skipSR1UpdateThreshold);
        LSST_DECLARE_CONTROL_FIELD(cls, OptimizerControl, minTrustRadiusThreshold);
        LSST_DECLARE_CONTROL_FIELD(cls, OptimizerControl, gradientThreshold);
        LSST_DECLARE_CONTROL_FIELD(cls, OptimizerControl, numDiffRelStep);
        LSST_DECLARE_CONTROL_FIELD(cls, OptimizerControl, numDiffAbsStep);
        LSST_DECLARE_CONTROL_FIELD(cls, OptimizerControl, numDiffTrustRadiusStep);
        LSST_DECLARE_CONTROL_FIELD(cls, OptimizerControl, stepAcceptThreshold);
        LSST_DECLARE_CONTROL_FIELD(cls, OptimizerControl, trustRegionInitialSize);
        LSST_DECLARE_CONTROL_FIELD(cls, OptimizerControl, trustRegionGrowReductionRatio);
        LSST_DECLARE_CONTROL_FIELD(cls, OptimizerControl, trustRegionGrowStepFraction);
        LSST_DECLARE_CONTROL_FIELD(cls, OptimizerControl, trustRegionGrowFactor);
        LSST_DECLARE_CONTROL_FIELD(cls, OptimizerControl, trustRegionShrinkReductionRatio);
        LSST_DECLARE_CONTROL_FIELD(cls, OptimizerControl, trustRegionShrinkFactor);
        LSST_DECLARE_CONTROL_FIELD(cls, OptimizerControl, trustRegionSolverTolerance);
        LSST_DECLARE_CONTROL_FIELD(cls, OptimizerControl, maxInnerIterations);
        LSST_DECLARE_CONTROL_FIELD(cls, OptimizerControl, maxOuterIterations);
        LSST_DECLARE_CONTROL_FIELD(cls, OptimizerControl, doSaveIterations);
        cls.def(py::init<>());
    });
}

PyOptimizerHistoryRecorder declareOptimizerHistoryRecorder(lsst::cpputils::python::WrapperCollection &wrappers) {
    return wrappers.wrapType(PyOptimizerHistoryRecorder(wrappers.module, "OptimizerHistoryRecorder"), [](auto &mod, auto &cls) {
        cls.def(py::init<afw::table::Schema &, std::shared_ptr<Model>, bool>(), "schema"_a, "model"_a,
                "doRecordDerivatives"_a);
        cls.def(py::init<afw::table::Schema const &>(), "schema"_a);
        cls.def("apply", &OptimizerHistoryRecorder::apply, "outerIterCount"_a, "innerIterCount"_a, "history"_a,
                "optimizer"_a);
        cls.def("unpackDerivatives",
                (void (OptimizerHistoryRecorder::*)(ndarray::Array<Scalar const, 1, 1> const &,
                                                    ndarray::Array<Scalar, 1, 1> const &,
                                                    ndarray::Array<Scalar, 2, 2> const &) const) &
                        OptimizerHistoryRecorder::unpackDerivatives,
                "nested"_a, "gradient"_a, "hessian"_a);
        cls.def("unpackDerivatives", (void (OptimizerHistoryRecorder::*)(
                        afw::table::BaseRecord const &, ndarray::Array<Scalar, 1, 1> const &,
                        ndarray::Array<Scalar, 2, 2> const &) const) &
                        OptimizerHistoryRecorder::unpackDerivatives,
                "record"_a, "gradient"_a, "hessian"_a);
        // Other unpackDerivatives overloads do the same thing but with Eigen types,
        // which makes them redundant in Python where it's all just NumPy.
        cls.def("fillObjectiveModelGrid", &OptimizerHistoryRecorder::fillObjectiveModelGrid, "record"_a,
                "parameters"_a, "output"_a);
        cls.def_readonly("outer", &OptimizerHistoryRecorder::outer);
        cls.def_readonly("inner", &OptimizerHistoryRecorder::inner);
        cls.def_readonly("state", &OptimizerHistoryRecorder::state);
        cls.def_readonly("objective", &OptimizerHistoryRecorder::objective);
        cls.def_readonly("prior", &OptimizerHistoryRecorder::prior);
        cls.def_readonly("trust", &OptimizerHistoryRecorder::trust);
        cls.def_readonly("parameters", &OptimizerHistoryRecorder::parameters);
        cls.def_readonly("derivatives", &OptimizerHistoryRecorder::derivatives);
    });
}

PyOptimizer declareOptimizer(lsst::cpputils::python::WrapperCollection &wrappers) {
    return wrappers.wrapType(PyOptimizer(wrappers.module, "Optimizer"), [](auto &mod, auto &cls) {
        // StateFlags enum is used as bitflag, so we wrap values as int class attributes.
        cls.attr("CONVERGED_GRADZERO") = py::cast(int(Optimizer::CONVERGED_GRADZERO));
        cls.attr("CONVERGED_TR_SMALL") = py::cast(int(Optimizer::CONVERGED_TR_SMALL));
        cls.attr("CONVERGED") = py::cast(int(Optimizer::CONVERGED));
        cls.attr("FAILED_MAX_INNER_ITERATIONS") = py::cast(int(Optimizer::FAILED_MAX_INNER_ITERATIONS));
        cls.attr("FAILED_MAX_OUTER_ITERATIONS") = py::cast(int(Optimizer::FAILED_MAX_OUTER_ITERATIONS));
        cls.attr("FAILED_MAX_ITERATIONS") = py::cast(int(Optimizer::FAILED_MAX_ITERATIONS));
        cls.attr("FAILED_EXCEPTION") = py::cast(int(Optimizer::FAILED_EXCEPTION));
        cls.attr("FAILED_NAN") = py::cast(int(Optimizer::FAILED_NAN));
        cls.attr("FAILED") = py::cast(int(Optimizer::FAILED));
        cls.attr("STATUS_STEP_REJECTED") = py::cast(int(Optimizer::STATUS_STEP_REJECTED));
        cls.attr("STATUS_STEP_ACCEPTED") = py::cast(int(Optimizer::STATUS_STEP_ACCEPTED));
        cls.attr("STATUS_STEP") = py::cast(int(Optimizer::STATUS_STEP));
        cls.attr("STATUS_TR_UNCHANGED") = py::cast(int(Optimizer::STATUS_TR_UNCHANGED));
        cls.attr("STATUS_TR_DECREASED") = py::cast(int(Optimizer::STATUS_TR_DECREASED));
        cls.attr("STATUS_TR_INCREASED") = py::cast(int(Optimizer::STATUS_TR_INCREASED));
        cls.attr("STATUS_TR") = py::cast(int(Optimizer::STATUS_TR));
        cls.attr("STATUS") = py::cast(int(Optimizer::STATUS));
        cls.def(py::init<std::shared_ptr<Optimizer::Objective const>, ndarray::Array<Scalar const, 1, 1> const &,
                        Optimizer::Control>(),
                "objective"_a, "parameters"_a, "ctrl"_a);
        cls.def("getObjective", &Optimizer::getObjective);
        cls.def("getControl", &Optimizer::getControl, py::return_value_policy::copy);
        cls.def("step", (bool (Optimizer::*)()) &Optimizer::step);
        cls.def("step", (bool (Optimizer::*)(Optimizer::HistoryRecorder const &, afw::table::BaseCatalog &)) &
                        Optimizer::step,
                "recorder"_a, "history"_a);
        cls.def("run", (int (Optimizer::*)()) &Optimizer::run);
        cls.def("run", (int (Optimizer::*)(Optimizer::HistoryRecorder const &, afw::table::BaseCatalog &)) &
                        Optimizer::run,
                "recorder"_a, "history"_a);
        cls.def("getState", &Optimizer::getState);
        cls.def("getObjectiveValue", &Optimizer::getObjectiveValue);
        cls.def("getParameters", &Optimizer::getParameters);
        cls.def("getResiduals", &Optimizer::getResiduals);
        cls.def("getGradient", &Optimizer::getGradient);
        cls.def("getHessian", &Optimizer::getHessian);
        cls.def("removeSR1Term", &Optimizer::removeSR1Term);
    });
}
}

void wrapOptimizer(lsst::cpputils::python::WrapperCollection &wrappers) {
    auto clsObjective = declareOptimizerObjective(wrappers);
    auto clsControl = declareOptimizerControl(wrappers);
    auto clsHistoryRecorder = declareOptimizerHistoryRecorder(wrappers);
    auto cls = declareOptimizer(wrappers);
    cls.attr("Objective") = clsObjective;
    cls.attr("Control") = clsControl;
    cls.attr("HistoryRecorder") = clsHistoryRecorder;

    wrappers.module.def("solveTrustRegion", &solveTrustRegion, "x"_a, "F"_a, "g"_a, "r"_a, "tolerance"_a);
}

}  // namespace modelfit
}  // namespace meas
}  // namespace lsst
