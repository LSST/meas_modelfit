#!/usr/bin/env python

#
# LSST Data Management System
# Copyright 2008-2013 LSST Corporation.
#
# This product includes software developed by the
# LSST Project (http://www.lsst.org/).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the LSST License Statement and
# the GNU General Public License along with this program.  If not,
# see <http://www.lsstcorp.org/LegalNotices/>.
#

import unittest
import os
import numpy
try:
    import scipy.integrate
except ImportError:
    scipy = None

import lsst.utils.tests
import lsst.meas.multifit

numpy.random.seed(500)

class IntegralsTestCase(lsst.utils.tests.TestCase):

    def mcIntegrate(self, g, f, n):
        cov = numpy.linalg.pinv(f)
        factor = numpy.linalg.det(f/(2*numpy.pi))**0.5
        mu = -numpy.dot(cov, g)
        z = numpy.random.multivariate_normal(mu, cov, size=n)
        return float(numpy.sum((z > 0).all(axis=1))) / (n*factor)

    def quadIntegrate(self, g, f):
        def func(y, x):
            z = numpy.array([x,y])
            return numpy.exp(-numpy.dot(g, z) - 0.5*numpy.dot(z, numpy.dot(f, z)))
        return scipy.integrate.dblquad(func, 0.0, float("inf"), lambda x: 0.0, lambda x: float("inf"),
                                       epsrel=1E-10, epsabs=1E-10)

    def testBVN(self):
        data = numpy.loadtxt(os.path.join("tests", "reference", "bvn.txt"), delimiter=',')
        for h, k, r, p1 in data:
            p2 = lsst.meas.multifit.bvnu(h, k, r)
            self.assertClose(p1, p2, rtol=1E-14)

    def test2d(self):
        if scipy is None:
            return
        for n in range(8):
            m = numpy.random.randn(3,2)
            y = numpy.random.randn(3)
            g = numpy.dot(m.transpose(), y)
            f = numpy.dot(m.transpose(), m)
            check, err = self.quadIntegrate(g, f)
            value = lsst.meas.multifit.integrateGaussian(g, f)
            self.assertClose(numpy.exp(-value), check, atol=err, rtol=1E-8)

            m[:,1] = m[:,0]   # make model degenerate
            g = numpy.dot(m.transpose(), y)
            f = numpy.dot(m.transpose(), m)
            check, err = self.quadIntegrate(g, f)
            value = lsst.meas.multifit.integrateGaussian(g, f)
            self.assertClose(numpy.exp(-value), check, atol=err, rtol=1E-8)

            for p in range(5):
                m[:,1] = m[:,0] + (numpy.random.randn(3) * 1E-5 * 10**p) # make model approach degenerate
                g = numpy.dot(m.transpose(), y)
                f = numpy.dot(m.transpose(), m)
                check, err = self.quadIntegrate(g, f)
                value = lsst.meas.multifit.integrateGaussian(g, f)
                self.assertClose(numpy.exp(-value), check, atol=err, rtol=1E-5)

def suite():
    """Returns a suite containing all the test cases in this module."""

    lsst.utils.tests.init()

    suites = []
    suites += unittest.makeSuite(IntegralsTestCase)
    suites += unittest.makeSuite(lsst.utils.tests.MemoryTestCase)
    return unittest.TestSuite(suites)

def run(shouldExit=False):
    """Run the tests"""
    lsst.utils.tests.run(suite(), shouldExit)

if __name__ == "__main__":
    run(True)
