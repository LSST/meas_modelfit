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

#include "lsst/meas/modelfit/DoubleShapeletPsfApprox.h"
#include "lsst/meas/modelfit/GeneralPsfFitter.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace meas {
namespace modelfit {
namespace {

void declareDoubleShapelet(lsst::cpputils::python::WrapperCollection &wrappers) {
    using Control = DoubleShapeletPsfApproxControl;
    using Algorithm = DoubleShapeletPsfApproxAlgorithm;

    using PyControl = py::class_<Control>;
    using PyAlgorithm = py::class_<Algorithm, meas::base::SimpleAlgorithm>;

    static auto clsControl =
            wrappers.wrapType(PyControl(wrappers.module, "DoubleShapeletPsfApproxControl"), [](auto &mod, auto &cls) {
                cls.def(py::init<>());
                LSST_DECLARE_CONTROL_FIELD(cls, Control, innerOrder);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, outerOrder);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, radiusRatio);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, peakRatio);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, minRadius);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, minRadiusDiff);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, maxRadiusBoxFraction);
                LSST_DECLARE_NESTED_CONTROL_FIELD(cls, Control, optimizer);
            });

    wrappers.wrapType(PyAlgorithm(wrappers.module, "DoubleShapeletPsfApproxAlgorithm"), [](auto &mod, auto &cls) {
        // wrap anonymous enum values as ints because we'll need to use them as ints
        cls.attr("Control") = clsControl;
        cls.attr("FAILURE") = py::cast(Algorithm::FAILURE);
        cls.attr("INVALID_POINT_FOR_PSF") = py::cast(Algorithm::INVALID_POINT_FOR_PSF);
        cls.attr("INVALID_MOMENTS") = py::cast(Algorithm::INVALID_MOMENTS);
        cls.attr("MAX_ITERATIONS") = py::cast(Algorithm::MAX_ITERATIONS);

        cls.def(py::init<Control const &, std::string const &, afw::table::Schema &>(), "ctrl"_a,
                         "name"_a, "schema"_a);
        cls.def_static("initializeResult", &Algorithm::initializeResult, "ctrl"_a);
        cls.def_static("fitMoments", &Algorithm::fitMoments, "result"_a, "ctrl"_a, "psfImage"_a);
        cls.def_static("makeObjective", &Algorithm::makeObjective, "moments"_a, "ctrl"_a, "psfImage"_a);
        cls.def_static("fitProfile", &Algorithm::fitProfile, "result"_a, "ctrl"_a, "psfImage"_a);
        cls.def_static("fitShapelets", &Algorithm::fitShapelets, "result"_a, "ctrl"_a, "psfImage"_a);
        cls.def("measure", &Algorithm::measure, "measRecord"_a, "exposure"_a);
        cls.def("fail", &Algorithm::fail, "measRecord"_a, "error"_a = nullptr);
    });
}

void declareGeneral(lsst::cpputils::python::WrapperCollection &wrappers) {
    using ComponentControl = GeneralPsfFitterComponentControl;
    using Control = GeneralPsfFitterControl;
    using Fitter = GeneralPsfFitter;
    using Algorithm = GeneralPsfFitterAlgorithm;

    using PyComponentControl = py::class_<ComponentControl>;
    using PyControl = py::class_<Control>;
    using PyFitter = py::class_<Fitter>;
    using PyAlgorithm = py::class_<Algorithm, Fitter>;

    wrappers.wrapType(PyComponentControl(wrappers.module, "GeneralPsfFitterComponentControl"), [](auto &mod, auto &cls) {
        cls.def(py::init<int, double>(), "order"_a = 0, "radius"_a = 1.0);
        LSST_DECLARE_CONTROL_FIELD(cls, ComponentControl, order);
        LSST_DECLARE_CONTROL_FIELD(cls, ComponentControl, positionPriorSigma);
        LSST_DECLARE_CONTROL_FIELD(cls ,ComponentControl, ellipticityPriorSigma);
        LSST_DECLARE_CONTROL_FIELD(cls, ComponentControl, radiusFactor);
        LSST_DECLARE_CONTROL_FIELD(cls, ComponentControl, radiusPriorSigma);
    });

    static auto clsControl =
            wrappers.wrapType(PyControl(wrappers.module, "GeneralPsfFitterControl"), [](auto &mod, auto &cls) {
                cls.def(py::init<>());
                LSST_DECLARE_NESTED_CONTROL_FIELD(cls, Control, inner);
                LSST_DECLARE_NESTED_CONTROL_FIELD(cls, Control, primary);
                LSST_DECLARE_NESTED_CONTROL_FIELD(cls, Control, wings);
                LSST_DECLARE_NESTED_CONTROL_FIELD(cls, Control, outer);
                LSST_DECLARE_NESTED_CONTROL_FIELD(cls, Control, optimizer);
                LSST_DECLARE_CONTROL_FIELD(cls, Control, defaultNoiseSigma);
            });

    wrappers.wrapType(PyFitter(wrappers.module, "GeneralPsfFitter"), [](auto &mod, auto &cls) {
        cls.def(py::init<Control const &>(), "ctrl"_a);
        cls.def("addFields", &Fitter::addFields, "schema"_a, "prefix"_a);
        cls.def("getModel", &Fitter::getModel);
        cls.def("getPrior", &Fitter::getPrior);
        cls.def("adapt", &Fitter::adapt, "previousFit"_a, "previousModel"_a);
        // We use lambdas here because the C++ signature has an optional int*
        // argument not relevant for Python (which confuses pybind11).
        cls.def("apply", [](Fitter const &self, afw::image::Image<Pixel> const &image,
                                  afw::geom::ellipses::Quadrupole const &moments,
                                  Scalar noiseSigma) { return self.apply(image, moments, noiseSigma); },
                      "image"_a, "moments"_a, "noiseSigma"_a = -1);
        cls.def("apply", [](Fitter const &self, afw::image::Image<double> const &image,
                                  afw::geom::ellipses::Quadrupole const &moments,
                                  Scalar noiseSigma) { return self.apply(image, moments, noiseSigma); },
                      "image"_a, "moments"_a, "noiseSigma"_a = -1);
        cls.def("apply", [](Fitter const &self, afw::image::Image<Pixel> const &image,
                                  shapelet::MultiShapeletFunction const &initial,
                                  Scalar noiseSigma) { return self.apply(image, initial, noiseSigma); },
                      "image"_a, "initial"_a, "noiseSigma"_a = -1);
        cls.def("apply", [](Fitter const &self, afw::image::Image<double> const &image,
                                  shapelet::MultiShapeletFunction const &initial,
                                  Scalar noiseSigma) { return self.apply(image, initial, noiseSigma); },
                      "image"_a, "initial"_a, "noiseSigma"_a = -1);
    });

    wrappers.wrapType(PyAlgorithm(wrappers.module, "GeneralPsfFitterAlgorithm"), [](auto &mod, auto &cls) {
        cls.attr("Control") = clsControl;
        cls.attr("FAILURE") = py::cast(Algorithm::FAILURE);
        cls.attr("MAX_INNER_ITERATIONS") = py::cast(Algorithm::MAX_INNER_ITERATIONS);
        cls.attr("MAX_OUTER_ITERATIONS") = py::cast(Algorithm::MAX_OUTER_ITERATIONS);
        cls.attr("EXCEPTION") = py::cast(Algorithm::EXCEPTION);
        cls.attr("CONTAINS_NAN") = py::cast(Algorithm::CONTAINS_NAN);
        cls.def(py::init<Control const &, afw::table::Schema &, std::string const &>(), "ctrl"_a,
                         "schema"_a, "prefix"_a);
        cls.def("getKey", &Algorithm::getKey);
        cls.def("measure",
                         (void (Algorithm::*)(afw::table::SourceRecord &, afw::image::Image<double> const &,
                                              shapelet::MultiShapeletFunction const &) const) &
                                 Algorithm::measure,
                         "measRecord"_a, "image"_a, "initial"_a);
        cls.def("measure",
                         (void (Algorithm::*)(afw::table::SourceRecord &, afw::image::Image<double> const &,
                                              afw::geom::ellipses::Quadrupole const &) const) &
                                 Algorithm::measure,
                         "measRecord"_a, "image"_a, "moments"_a);
        cls.def("fail", &Algorithm::fail, "measRecord"_a, "error"_a = nullptr);
    });
    // MultiShapeletPsfLikelihood intentionally not exposed to Python.
}

}  // namespace

void wrapPsf(lsst::cpputils::python::WrapperCollection &wrappers) {
    declareDoubleShapelet(wrappers);
    declareGeneral(wrappers);
}

}  // namespace modelfit
}  // namespace emeas
}  // namespace lsst:
