// -*- lsst-c++ -*-
/**
 * @file
 * Declaration of class PointSourceMorphology
 */
#ifndef LSST_MEAS_MULTIFIT_COMPONENTS_POINT_SOURCE_MORPHOLOGY_H
#define LSST_MEAS_MULTIFIT_COMPONENTS_POINT_SOURCE_MORPHOLOGY_H

#include <boost/make_shared.hpp>

#include "lsst/meas/multifit/core.h"
#include "lsst/meas/multifit/components/Morphology.h"

namespace lsst {
namespace meas {
namespace multifit {
namespace components {

/**
 * Derived Morphology component for fitting static point-sources
 */
class PointSourceMorphology : public Morphology {
public:
    enum LinearParameters {FLUX=0, LINEAR_SIZE};
    enum NonlinearParamaters {NONLINEAR_SIZE=0};

    typedef boost::shared_ptr<PointSourceMorphology> Ptr;
    typedef boost::shared_ptr<PointSourceMorphology const> ConstPtr;

    virtual lsst::afw::geom::ellipses::Core::Ptr computeBoundingEllipseCore() const {
        return boost::make_shared<lsst::afw::geom::ellipses::LogShear>();
    }

    virtual MorphologyProjection::Ptr makeProjection(
        lsst::afw::geom::Extent2I const & kernelDimensions,
        lsst::afw::geom::AffineTransform::ConstPtr const & transform
    ) const;

    /**
     * Named PointSourceMorphology constructor    
     */
    static Ptr create(Parameter const & flux) { 
        boost::shared_ptr<ParameterVector const> linear(
            new ParameterVector((ParameterVector(1) << flux).finished())
        );
        boost::shared_ptr<ParameterVector const> nonlinear(
            new ParameterVector()
        );
        return PointSourceMorphology::Ptr(
            new PointSourceMorphology(linear, nonlinear, 0)
        );
    }       

    Parameter const & getFlux() const {return *(_getLinearParameters()->data());}
protected:

    /**
     *  Construct a Morphology object for use inside a ComponentModel.
     *
     *  @sa Morphology::create
     *  @sa FixedMorphology::create
     */
    PointSourceMorphology(
        boost::shared_ptr<ParameterVector const> const & linearParameters,
        boost::shared_ptr<ParameterVector const> const & nonlinearParameters,
        size_t const & start =0
    ) : Morphology(linearParameters, nonlinearParameters, start) {}

    virtual Morphology::Ptr create(
        boost::shared_ptr<ParameterVector const> const & linearParameters,
        boost::shared_ptr<ParameterVector const> const & nonlinearParameters,
        size_t const & start=0 
    ) const {
        return PointSourceMorphology::Ptr(
            new PointSourceMorphology(
                linearParameters, 
                nonlinearParameters, 
                start
            )
        );
    }

    virtual const int getNonlinearParameterSize() const {return NONLINEAR_SIZE;}
};

}}}} // namespace lsst::meas::multifit::components

#endif // !LSST_MEAS_MULTIFIT_COMPONENTS_POINT_SOURCE_MORPHOLOGY_H
