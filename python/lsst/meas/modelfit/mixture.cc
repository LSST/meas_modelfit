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

#include <sstream>  // Python.h must come before even system headers

#include "ndarray/pybind11.h"
#include "ndarray/eigen.h"


#include "lsst/meas/modelfit/Mixture.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace meas {
namespace modelfit {
namespace {

using PyMixtureComponent = py::class_<MixtureComponent>;
using PyMixtureUpdateRestriction =
        py::class_<MixtureUpdateRestriction>;
using PyMixture = py::class_<Mixture>;

PyMixtureComponent declareMixtureComponent(lsst::cpputils::python::WrapperCollection &wrappers) {
    return wrappers.wrapType(
            PyMixtureComponent(wrappers.module, "MixtureComponent"), [](auto &mod, auto &cls) {
                cls.def("getDimension", &MixtureComponent::getDimension);
                cls.def_readwrite("weight", &MixtureComponent::weight);
                cls.def("getMu", &MixtureComponent::getMu);
                cls.def("setMu", &MixtureComponent::setMu);
                cls.def("getSigma", &MixtureComponent::getSigma);
                cls.def("setSigma", &MixtureComponent::setSigma);
                cls.def("project", (MixtureComponent (MixtureComponent::*)(int) const) &MixtureComponent::project,
                        "dim"_a);
                cls.def("project", (MixtureComponent (MixtureComponent::*)(int, int) const) &MixtureComponent::project,
                        "dim1"_a, "dim2"_a);
                cls.def(py::init<int>(), "dim"_a);
                cls.def(py::init<Scalar, Vector const &, Matrix const &>(), "weight"_a, "mu"_a, "sigma"_a);
                utils::python::addOutputOp(cls, "__str__");
                utils::python::addOutputOp(cls, "__repr__");
            });
}

PyMixtureUpdateRestriction declareMixtureUpdateRestriction(lsst::cpputils::python::WrapperCollection &wrappers) {
    return wrappers.wrapType(PyMixtureUpdateRestriction(
            wrappers.module, "MixtureUpdateRestriction"), [](auto &mod, auto &cls) {
        cls.def("getDimension", &MixtureUpdateRestriction::getDimension);
        cls.def(py::init<int>(), "dim"_a);
        // The rest of this interface isn't usable in Python, and doesn't need to be.
    });
}

PyMixture declareMixture(lsst::cpputils::python::WrapperCollection &wrappers) {
    return wrappers.wrapType(PyMixture(wrappers.module, "Mixture"), [](auto &mod, auto &cls) {
        afw::table::io::python::addPersistableMethods<Mixture>(cls);
        cls.def("__iter__", [](Mixture &self) { return py::make_iterator(self.begin(), self.end()); },
                py::keep_alive<0, 1>());
        cls.def("__getitem__",
                [](Mixture &self, std::ptrdiff_t i) { return self[utils::python::cppIndex(self.size(), i)]; },
                py::return_value_policy::reference_internal);
        cls.def("__len__", &Mixture::size);
        cls.def("getComponentCount", &Mixture::getComponentCount);
        cls.def("project", (std::shared_ptr<Mixture> (Mixture::*)(int) const) &Mixture::project, "dim"_a);
        cls.def("project", (std::shared_ptr<Mixture> (Mixture::*)(int, int) const) &Mixture::project, "dim1"_a,
                "dim2"_a);
        cls.def("getDimension", &Mixture::getDimension);
        cls.def("normalize", &Mixture::normalize);
        cls.def("shift", &Mixture::shift, "dim"_a, "offset"_a);
        cls.def("clip", &Mixture::clip, "threshold"_a = 0.0);
        cls.def("getDegreesOfFreedom", &Mixture::getDegreesOfFreedom);
        cls.def("setDegreesOfFreedom", &Mixture::setDegreesOfFreedom,
                "df"_a = std::numeric_limits<Scalar>::infinity());
        cls.def("evaluate",
                [](Mixture const &self, MixtureComponent const &component,
                   ndarray::Array<Scalar, 1, 0> const &array) -> Scalar {
                    return self.evaluate(component, ndarray::asEigenMatrix(array));
                },
                "component"_a, "x"_a);
        cls.def("evaluate",
                [](Mixture const &self, ndarray::Array<Scalar, 1, 0> const &array) -> Scalar {
                    return self.evaluate(ndarray::asEigenMatrix(array));
                },
                "x"_a);
        cls.def("evaluate", (void (Mixture::*)(ndarray::Array<Scalar const, 2, 1> const &,
                                               ndarray::Array<Scalar, 1, 0> const &) const) &
                        Mixture::evaluate,
                "x"_a, "p"_a);
        cls.def("evaluateComponents", &Mixture::evaluateComponents, "x"_a, "p"_a);
        cls.def("evaluateDerivatives",
                py::overload_cast<ndarray::Array<Scalar const, 1, 1> const &,
                        ndarray::Array<Scalar, 1, 1> const &,
                        ndarray::Array<Scalar, 2, 1> const &>(&Mixture::evaluateDerivatives, py::const_),
                "x"_a, "gradient"_a, "hessian"_a);
        cls.def("draw", &Mixture::draw, "rng"_a, "x"_a);
        cls.def("updateEM", (void (Mixture::*)(ndarray::Array<Scalar const, 2, 1> const &,
                                               ndarray::Array<Scalar const, 1, 0> const &, Scalar, Scalar)) &
                        Mixture::updateEM,
                "x"_a, "w"_a, "tau1"_a = 0.0, "tau2"_a = 0.5);
        cls.def("updateEM", (void (Mixture::*)(ndarray::Array<Scalar const, 2, 1> const &,
                                               ndarray::Array<Scalar const, 1, 0> const &,
                                               MixtureUpdateRestriction const &restriction, Scalar, Scalar)) &
                        Mixture::updateEM,
                "x"_a, "w"_a, "restriction"_a, "tau1"_a = 0.0, "tau2"_a = 0.5);
        cls.def("updateEM", (void (Mixture::*)(ndarray::Array<Scalar const, 2, 1> const &,
                                               MixtureUpdateRestriction const &restriction, Scalar, Scalar)) &
                        Mixture::updateEM,
                "x"_a, "restriction"_a, "tau1"_a = 0.0, "tau2"_a = 0.5);
        cls.def("clone", &Mixture::clone);
        cls.def(py::init<int, Mixture::ComponentList &, Scalar>(), "dim"_a, "components"_a,
                "df"_a = std::numeric_limits<Scalar>::infinity());
        utils::python::addOutputOp(cls, "__str__");
        utils::python::addOutputOp(cls, "__repr__");
    });
}
} // namespace

void wrapMixture(lsst::cpputils::python::WrapperCollection &wrappers) {
    auto clsMixtureComponent = declareMixtureComponent(wrappers);
    auto clsMixtureUpdateRestriction = declareMixtureUpdateRestriction(wrappers);
    auto clsMixture = declareMixture(wrappers);
    clsMixture.attr("Component") = clsMixtureComponent;
    clsMixture.attr("UpdateRestriction") = clsMixtureUpdateRestriction;
}

}  // namespace modelfit
}  // namespace meas
}  // namespace lsst
