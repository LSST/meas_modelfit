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
#include "lsst/meas/modelfit/Prior.h"
#include "lsst/meas/modelfit/MixturePrior.h"
#include "lsst/meas/modelfit/SemiEmpiricalPrior.h"
#include "lsst/meas/modelfit/SoftenedLinearPrior.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace meas {
namespace modelfit {
namespace {

void declarePrior(lsst::cpputils::python::WrapperCollection &wrappers) {
    using PyPrior = py::class_<Prior>;

    wrappers.wrapType(PyPrior(wrappers.module, "Prior"), [](auto &mod, auto &cls) {
        cls.def("getTag", &Prior::getTag);
        cls.def("evaluate", &Prior::evaluate, "nonlinear"_a, "amplitudes"_a);
        cls.def("evaluateDerivatives", &Prior::evaluateDerivatives, "nonlinear"_a, "amplitudes"_a,
                "nonlinearGradient"_a, "amplitudeGradient"_a, "nonlinearHessian"_a, "amplitudeHessian"_a,
                "crossHessian"_a);
        cls.def("marginalize", &Prior::marginalize, "gradient"_a, "hessian"_a, "nonlinear"_a);
        cls.def("maximize", &Prior::maximize, "gradient"_a, "hessian"_a, "nonlinear"_a, "amplitudes"_a);
        cls.def("drawAmplitudes", &Prior::drawAmplitudes, "gradient"_a, "hessian"_a, "nonlinear"_a, "rng"_a,
                "amplitudes"_a, "weights"_a, "multiplyWeights"_a = false);
    });
}

void declareMixturePrior(lsst::cpputils::python::WrapperCollection &wrappers) {
    using Class = MixturePrior;
    using PyClass = py::class_<Class, Prior>;
    wrappers.wrapType(PyClass(wrappers.module, "MixturePrior"), [](auto &mod, auto &cls) {
        cls.def(py::init<std::shared_ptr<Mixture>, std::string const &>(), "mixture"_a, "tag"_a = "");
        cls.def_static("getUpdateRestriction", &Class::getUpdateRestriction,
                       py::return_value_policy::reference);  // returned object has static duration
        cls.def("getMixture", &Class::getMixture);
        // virtual methods already wrapped by Prior base class
    });
}

void declareSemiEmpiricalPrior(lsst::cpputils::python::WrapperCollection &wrappers) {
    using Class = SemiEmpiricalPrior;
    using Control = SemiEmpiricalPriorControl;
    using PyControl = py::class_<Control>;
    using PyClass = py::class_<Class, Prior>;

    static auto clsControl =
            wrappers.wrapType(PyControl(wrappers.module, "SemiEmpiricalPriorControl"), [](auto &mod, auto &cls) {
                cls.def(py::init<>());
                LSST_DECLARE_CONTROL_FIELD(cls, Control, ellipticitySigma);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, ellipticityCore);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, logRadiusMinOuter);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, logRadiusMinInner);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, logRadiusMu);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, logRadiusSigma);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, logRadiusNu);
                cls.def("validate", &Control::validate);
            });

    wrappers.wrapType(PyClass(wrappers.module, "SemiEmpiricalPrior"), [](auto &mod, auto &cls) {
        cls.def(py::init<Control>(), "ctrl"_a);
        cls.attr("Control") = clsControl;
    });
    // virtual methods already wrapped by Prior base class
}

void declareSoftenedLinearPrior(lsst::cpputils::python::WrapperCollection &wrappers) {
    using Class = SoftenedLinearPrior;
    using Control = SoftenedLinearPriorControl;
    using PyControl = py::class_<Control>;
    using PyClass = py::class_<Class, Prior>;

    static auto clsControl =
            wrappers.wrapType(PyControl(wrappers.module, "SoftenedLinearPriorControl"), [](auto &mod, auto &cls) {
                cls.def(py::init<>());
                LSST_DECLARE_CONTROL_FIELD(cls, Control, ellipticityMaxOuter);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, ellipticityMaxInner);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, logRadiusMinOuter);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, logRadiusMinInner);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, logRadiusMaxOuter);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, logRadiusMaxInner);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, logRadiusMinMaxRatio);
            });

    wrappers.wrapType(PyClass(wrappers.module, "SoftenedLinearPrior"), [](auto &mod, auto &cls) {
        cls.def(py::init<Control>(), "ctrl"_a);
        cls.def("getControl", &Class::getControl, py::return_value_policy::copy);
        cls.attr("Control") = clsControl;
        // virtual methods already wrapped by Prior base class
    });
}
}  // namespace

void wrapPriors(lsst::cpputils::python::WrapperCollection &wrappers) {
    // py::module::import("lsst.meas.modelfit.mixture");

    declarePrior(wrappers);
    declareMixturePrior(wrappers);
    declareSemiEmpiricalPrior(wrappers);
    declareSoftenedLinearPrior(wrappers);
}

}  // namespace modelfit
}  // namespace meas
}  // namespace lsst
