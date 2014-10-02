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

#ifndef LSST_MEAS_MULTIFIT_CModelFit_h_INCLUDED
#define LSST_MEAS_MULTIFIT_CModelFit_h_INCLUDED

#include "ndarray.h"

#include "lsst/pex/config.h"
#include "lsst/meas/base/exceptions.h"
#include "lsst/afw/table/Source.h"
#include "lsst/shapelet/RadialProfile.h"
#include "lsst/meas/multifit/Model.h"
#include "lsst/meas/multifit/Prior.h"
#include "lsst/meas/multifit/MixturePrior.h"
#include "lsst/meas/multifit/SoftenedLinearPrior.h"
#include "lsst/meas/multifit/UnitTransformedLikelihood.h"
#include "lsst/meas/multifit/optimizer.h"

namespace lsst { namespace meas { namespace multifit {

/**
 *  @page multifitCModel CModel Magnitudes
 *
 *  The CModel approach to model-fit galaxy photometry - also known as the "Sloan Swindle" - is an
 *  approximation to bulge+disk or Sersic model fitting that follows the following sequence:
 *   - Fit a PSF-convolved elliptical exponential (Sersic n=1) model to the data.
 *   - Fit a PSF-convolved elliptical de Vaucouleurs (Sersic n=4) model to the data.
 *   - Holding the positions and ellipses of both models fixed (only allowing the amplitudes to vary),
 *     fit a linear combination of the two models.
 *  In the limit of pure bulge or pure disk galaxies, this approach yields the same results as a more
 *  principled bugle+disk or Sersic fit.  For galaxies that are a combination of the two components (or
 *  have more complicated morphologies, as of course all real galaxies do), it provides a smooth transition
 *  between the two models, and the fraction of flux in each of the two parameters is correlated with
 *  Sersic index and the true bulge-disk ratio.  Most importantly, this approach yieled good galaxy colors
 *  in the SDSS data processing.
 *
 *  In this implementation of the CModel algorithm, we actually have 4 stages:
 *   - In the "initial" stage, we fit a very approximate PSF-convolved elliptical model, just to provide
 *     a good starting point for the subsequence exponential and de Vaucouleur fits.  Because we use
 *     shapelet/Gaussian approximations to convolved models with the PSF, model evaluation is much faster
 *     when only a few Gaussians are used in the approximation, as is done here.  In the future, we may
 *     also use a simpler PSF approximation in the initial fit, but this is not yet implemented.  We also
 *     have not yet researched how best to make use of the initial fit (i.e. how does the initial best-fit
 *     radius typically relate to the best-fit exponential radius?), or what convergence criteria should
 *     be used in the initial fit.  Following the initial fit, we also revisit the question of which pixels
 *     should be included in the fit (see CModelRegionControl).
 *   - In the "exp" stage, we start with the "initial" fit results, and fit an elliptical exponential
 *     profile.
 *   - In the "dev" stage, we start with the "initial" fit results, and fit an elliptical de Vaucouleur
 *     profile.
 *   - Holding the "exp" and "dev" ellipses fixed, we fit a linear combination of those two profiles.
 *  In all of these steps, the centroid is held fixed at a given input value (take from the slot centroid
 *  when run by the measurement framework).
 *
 *  @section cmodelUnits Units
 *
 *  Unlike most measurement algorithms, CModel requires the Exposure it is given to have both a Wcs and
 *  a Calib.  This is because it makes use of Bayesian priors, and hence it has to know the relationship
 *  between the raw units of the image (pixels and dn) and the global units in which the priors are defined.
 *
 *  In fact, all of the nonlinear fits in CModel are done in a special, local coordinate system, defined
 *  by a Wcs in which the "pixels" have units of arcseconds (because we never create an image in this system,
 *  we don't have to worry about the size of the pixels) and the fluxes should be of order unity.  In
 *  addition to allowing us to use priors, it also ensures that the parameters all have the same order
 *  of magnitude, which improves the behavior of the optimizer.
 *
 *  See @ref multifitUnits for more information.
 *
 *  @section cmodelForced Forced Photometry
 *
 *  In forced photometry, we replace the three nonlinear fits with amplitude-only fits, and then repeat the
 *  final linear fit, using the ellipses from the reference catalog in all casees.  We do allow the relative
 *  amplitudes of the two components to vary in forced mode, though in the future we will add an option to
 *  hold this fixed as well as the ellipses.
 *
 *  @section cmodelPsf Shapelet Approximations to the PSF
 *
 *  The CModel algorithm relies on a multi-shapelet approximation to the PSF to convolve galaxy models.  It
 *  does not compute this approximation directly; for CModelAlgorithm methods that take inputs directly
 *  as arguments, the PSF must be supplied as a shapelet::MultiShapeletFunction instance.  When using
 *  SourceRecords for input/output, CModel assumes that the ShapeletPsfApprox plugin has already been
 *  run (see psf.py), and uses the fields created by that plugin to retrieve the PSF approximation.
 *
 *  @section cmodelOrg Code Organization
 *
 *  The CModel implementation consists of many classes, defined in this file and CModel.cc.  These mostly
 *  fall into four categories:
 *    - Control structs: C++ analogs of Python Config classes, these structs contain the
 *      configuration parameters that control the behavior of the algorithm.  These are nested; the
 *      @ref CModelControl struct contains a @ref CModelRegionControl instance, a
 *      @ref CModelDiagnosticsControl instance, and three @ref CModelStageControl (one for each of "initial",
 *      "exp", and "dev").  The configuration for the final amplitude-only fit goes in @ref CModelControl
 *      itself; because it is a simpler linear fit, it doesn't have much in common with the first
 *      three stages.
 *    - Result structs: while the algorithm has methods to use SourceRecord objects for input/output,
 *      it can also take inputs directly as arguments and return the outputs using these structs.  Like
 *      the Control structs, the master @ref CModelResult struct holds three @ref CModelStageResult classes,
 *      for each of the three nonlinear fits.
 *    - Keys structs: these private classes (defined in an anonymous namespace in CModel.cc) hold the
 *      afw::table::Key and FunctorKey objects that provide a mapping from the Result structs to
 *      Schema fields.  They also provide methods to transfer values from Results to Records, or the
 *      reverse.  These are also split into a master CModelKeys struct, which holds three
 *      CModelStageKeys structs.
 *    - Impl classes: these private classes contain the actual algorithmic code.  Once again, we
 *      have the master implementation class (CModelAlgorithm::Impl) and a class for the nonlinear
 *      fitting stages (CModelStageImpl).
 *  In addition to these categories, we also have the @ref CModelAlgorithm class, which is the C++ public
 *  interface to all of this, and the CModelStageData class, a private class that aggregates per-source
 *  state and makes it easier to pass it around.
 */

/**
 *  Nested control object for CModel that configures one of the three ("initial", "exp", "dev") nonlinear
 *  fitting stages.
 */
struct CModelStageControl {

    CModelStageControl() :
        profileName("lux"),
        priorSource("CONFIG"),
        priorName(),
        nComponents(8),
        maxRadius(0),
        doRecordHistory(true),
        doRecordTime(true)
    {}

    shapelet::RadialProfile const & getProfile() const {
        return shapelet::RadialProfile::get(profileName);
    }

    PTR(Model) getModel() const;

    PTR(Prior) getPrior() const;

    LSST_CONTROL_FIELD(
        profileName, std::string,
        "Name of the shapelet.RadialProfile that defines the model to fit"
    );

    LSST_CONTROL_FIELD(
        priorSource, std::string,
        "One of 'FILE', 'CONFIG', or 'NONE', indicating whether the prior should be loaded from disk "
        "created from the nested prior config/control object, or None"
    );

    LSST_CONTROL_FIELD(
        priorName, std::string,
        "Name of the Prior that defines the model to fit (a filename in $MEAS_MULTIFIT_DIR/data, "
        "with no extension), if priorSource='FILE'.  Ignored for forced fitting."
    );

    LSST_NESTED_CONTROL_FIELD(
        priorConfig, lsst.meas.multifit.multifitLib, SoftenedLinearPriorControl,
        "Configuration for the prior, used if priorSource='CONFIG'."
    );

    LSST_CONTROL_FIELD(nComponents, int, "Number of Gaussian used to approximate the profile");

    LSST_CONTROL_FIELD(
        maxRadius,
        int,
        "Maximum radius used in approximating profile with Gaussians (0=default for this profile)"
    );

    LSST_NESTED_CONTROL_FIELD(
        optimizer, lsst.meas.multifit.multifitLib, OptimizerControl,
        "Configuration for how the objective surface is explored.  Ignored for forced fitting"
    );

    LSST_NESTED_CONTROL_FIELD(
        likelihood, lsst.meas.multifit.multifitLib, UnitTransformedLikelihoodControl,
        "Configuration for how the compound model is evaluated and residuals are weighted in this "
        "stage of the fit"
    );

    LSST_CONTROL_FIELD(
        doRecordHistory, bool,
        "Whether to record the steps the optimizer takes (or just the number, if running as a plugin)"
    );

    LSST_CONTROL_FIELD(
        doRecordTime, bool,
        "Whether to record the time spent in this stage stage"
    );

};

/**
 *  Nested control object for CModel that configures which pixels are used in the fit.
 *
 *  The pixel region is determined from the union of several quantities:
 *   - the Psf model image bounding box.
 *   - the detection Footprint of the source, grown by a configurable number of pixels.
 *   - the best-fit ellipse from the "initial" stage, scaled by a configurable factor (used to update
 *     the fit region following the initial stage.
 *  Masked pixels can also be removed from the fit region.
 *
 *  In addition, if the fit region is too large, or too many of its pixels were masked, the
 *  fit will be aborted early.  This prevents the algorithm from spending too much time fitting
 *  garbage such as bleed trails.
 */
struct CModelRegionControl {

    CModelRegionControl() :
        includePsfBBox(false),
        nGrowFootprint(5),
        nInitialRadii(3),
        maxArea(10000),
        maxBadPixelFraction(0.1)
    {
        badMaskPlanes.push_back("EDGE");
        badMaskPlanes.push_back("SAT");
    }

    LSST_CONTROL_FIELD(
        includePsfBBox, bool,
        "If True, always make the fit region at least the size of the PSF model realization's bounding box"
    );

    LSST_CONTROL_FIELD(
        nGrowFootprint, int,
        "Number of pixels to grow the original footprint by before the initial fit."
    );

    LSST_CONTROL_FIELD(
        nInitialRadii, double,
        "After the initial fit, extend the fit region to include all the pixels within "
        "this many initial-fit radii."
    );

    LSST_CONTROL_FIELD(
        maxArea, int,
        "Abort if the fit region grows beyond this many pixels."
    );

    LSST_CONTROL_FIELD(
        badMaskPlanes, std::vector<std::string>,
        "Mask planes that indicate pixels that should be ignored in the fit."
    );

    LSST_CONTROL_FIELD(
        maxBadPixelFraction, double,
        "Maximum fraction of pixels that may be ignored due to masks; "
        "more than this and we don't even try."
    );

};

/**
 *  Nested control object for CModel that configures debug outputs
 *
 *  CModel has the capability to write optimizer traces to disk for selected objects, to enable
 *  post-mortem debugging of those fits.  This is not implemented in the cleanest possible way
 *  (output locations are not handled by the butler, for instance), but we'd need big changes
 *  to the measurement framework and the butler to clean that up.
 */
struct CModelDiagnosticsControl {

    CModelDiagnosticsControl() : enabled(false), root(""), ids() {}

    LSST_CONTROL_FIELD(
        enabled,  bool,
        "Whether to write diagnostic outputs for post-run debugging"
    );

    LSST_CONTROL_FIELD(
        root, std::string,
        "Root output path for diagnostic outputs"
    );

    LSST_CONTROL_FIELD(
        ids, std::vector<boost::int64_t>,
        "Source IDs for which diagnostic outpust should be produced"
    );

};

/**
 *  The main control object for CModel, containing parameters for the final linear fit and aggregating
 *  the other control objects.
 */
struct CModelControl {

    CModelControl() :
        psfName("DoubleGaussian"),
        minInitialRadius(0.1)
    {
        initial.nComponents = 3; // use very rough model in initial fit
        initial.optimizer.gradientThreshold = 1E-2; // with coarse convergence criteria
        initial.optimizer.minTrustRadiusThreshold = 1E-2;
        dev.profileName = "luv";
    }

    LSST_CONTROL_FIELD(
        psfName,
        std::string,
        "Name of the ShapeletPsfApprox model (one of the keys in the ShapeletPsfApproxConfig.model dict) "
        "used to convolve the galaxy model."
    );

    LSST_NESTED_CONTROL_FIELD(
        region, lsst.meas.multifit.multifitLib, CModelRegionControl,
        "Configuration parameters related to the determination of the pixels to include in the fit."
    );

    LSST_NESTED_CONTROL_FIELD(
        diagnostics, lsst.meas.multifit.multifitLib, CModelDiagnosticsControl,
        "Configuration parameters related to diagnostic outputs for post-run debugging."
    );

    LSST_NESTED_CONTROL_FIELD(
        initial, lsst.meas.multifit.multifitLib, CModelStageControl,
        "An initial fit (usually with a fast, approximate model) used to warm-start the exp and dev fits, "
        "convolved with only the zeroth-order terms in the multi-shapelet PSF approximation."
    );

    LSST_NESTED_CONTROL_FIELD(
        exp, lsst.meas.multifit.multifitLib, CModelStageControl,
        "Independent fit of the exponential component"
    );

    LSST_NESTED_CONTROL_FIELD(
        dev, lsst.meas.multifit.multifitLib, CModelStageControl,
        "Independent fit of the de Vaucouleur component"
    );

    LSST_NESTED_CONTROL_FIELD(
        likelihood, lsst.meas.multifit.multifitLib, UnitTransformedLikelihoodControl,
        "configuration for how the compound model is evaluated and residuals are weighted in the exp+dev "
        "linear combination fit"
    );

    LSST_CONTROL_FIELD(
        minInitialRadius, double,
        "Minimum initial radius in pixels (used to regularize initial moments-based PSF deconvolution)"
    );

};

/**
 *  Result object for a single nonlinear fitting stage of the CModel algorithm
 */
struct CModelStageResult {

    /// Flags for a single CModel stage (note that there are additional flags for the full multi-stage fit)
    enum FlagBit {
        FAILED=0,        ///< General flag, indicating whether the flux for this stage can be trusted.
        TR_SMALL,        ///< Whether convergence was due to the optimizer trust region getting too small
                         ///  (not a failure!)
        MAX_ITERATIONS,  ///< Whether the optimizer exceeded the maximum number of iterations.  Indicates
                         ///  a suspect fit, but not necessarily a bad one (implies FAILED).
        NUMERIC_ERROR,   ///< Optimizer encountered a numerical error (something likely went to infinity).
                         ///  Result will be unusable; implies FAILED.
        N_FLAGS          ///< Non-flag counter to indicate the number of flags
    };

    CModelStageResult();

    PTR(Model) model;    ///< Model object that defines the parametrization (defined fully by Control struct)
    PTR(Prior) prior;    ///< Bayesian priors on the parameters (defined fully by Control struct)
    PTR(OptimizerObjective) objfunc;  ///< Objective class used by the optimizer
    Scalar flux;         ///< Flux measured from just this stage fit.
    Scalar fluxSigma;    ///< Flux uncertainty from just this stage fit.
    Scalar objective;    ///< Value of the objective function at the best fit point: chisq/2 - ln(prior)
    Scalar time;         ///< Time spent in this fit in seconds.
    afw::geom::ellipses::Quadrupole ellipse;  ///< Best fit half-light ellipse in pixel coordinates

    //@{
    /// Flag accessors, to work around Swig's lack of support for std::bitset
    bool getFlag(FlagBit b) const { return flags[b]; }
    void setFlag(FlagBit b, bool value) { flags[b] = value; }
    //}

    ndarray::Array<Scalar const,1,1> nonlinear;  ///< Opaque nonlinear parameters in specialized units
    ndarray::Array<Scalar const,1,1> amplitudes; ///< Opaque linear parameters in specialized units
    ndarray::Array<Scalar const,1,1> fixed;      ///< Opaque fixed parameters in specialized units

    afw::table::BaseCatalog history;  ///< Trace of the optimizer's path, if enabled by diagnostic options
#ifndef SWIG
    std::bitset<N_FLAGS> flags; ///< Array of flags.
#endif
};

/**
 *  Master result object for CModel, containing results for the final linear fit and three nested
 *  CModelStageResult objects for the results of the previous stages.
 */
struct CModelResult {

    /// Flags that apply to all four CModel fits or just the last one.
    enum FlagBit {
        FAILED=0,                ///< General failure flag for the linear fit flux; set if any other
                                 ///  CModel flag is set, or if any of the three previous stages failed.
        MAX_AREA,                ///< Set if we aborted early because the fit region was too large.
        MAX_BAD_PIXEL_FRACTION,  ///< Set if we aborted early because the fit region had too many bad pixels.
        NO_SHAPE,                ///< Set if the input SourceRecord had no valid shape slot with which to
                                 ///  start the fit.
        NO_SHAPELET_PSF,         ///< Set if the Psf shapelet approximation failed.
        N_FLAGS                  ///< Non-flag counter to indicate the number of flags
    };

    CModelResult();

    Scalar flux;       ///< Flux from the final linear fit
    Scalar fluxSigma;  ///< Flux uncertainty from the final linear fit
    Scalar fracDev;    ///< Fraction of flux from the final linear fit in the de Vaucouleur component
                       ///  (always between 0 and 1).
    Scalar objective;  ///< Objective value at the best-fit point (chisq/2)

    //@{
    /// Flag accessors, to work around Swig's lack of support for std::bitset
    bool getFlag(FlagBit b) const { return flags[b]; }
    void setFlag(FlagBit b, bool value) { flags[b] = value; }
    //@}

    CModelStageResult initial; ///< Results from the initial approximate nonlinear fit that feeds the others
    CModelStageResult exp;     ///< Results from the exponential (Sersic n=1) fit
    CModelStageResult dev;     ///< Results from the de Vaucouleur (Sersic n=4) fit

    PTR(afw::detection::Footprint) initialFitRegion;  ///< Pixels used in the initial fit.
    PTR(afw::detection::Footprint) finalFitRegion;    ///< Pixels used in the exp, dev, and linear fits.

#ifndef SWIG
    std::bitset<N_FLAGS> flags; ///< Array of flags.
#endif
};

/**
 *  Main public interface class for CModel algorithm.
 *
 *  See @ref multifitCModel for a full description of the algorithm.
 *
 *  This class provides the methods that actually execute the algorithm, and (depending on how it is
 *  constructed) holds the Key objects necessary to use SourceRecords for input and output.
 */
class CModelAlgorithm {
public:

    typedef CModelControl Control; ///< Typedef to the master Control struct
    typedef CModelResult Result;   ///< Typedef to the master Result struct

    /**
     *  Construct an algorithm instance and add its fields to the Schema.
     *
     *  All fields needed to write the outputs of a regular, non-forced fit will be added to the given
     *  Schema.  In addition, keys needed to retrieve the PSF shapelet approximation (assuming the
     *  ShapeletPsfApprox plugin has been run) will be extracted from the Schema.
     *
     *  @param[in]     name    Name of the algorithm used as a prefix for all fields added to the Schema.
     *  @param[in]     ctrl    Control object that configures the algorithm.
     *  @param[in,out] schema  Schema to which fields will be added, and from which keys for the PSF
     *                         shapelet approximation will be extacted.
     */
    CModelAlgorithm(
        std::string const & name,
        Control const & ctrl,
        afw::table::Schema & schema
    );

    /**
     *  Construct an algorithm instance suitable for forced photometry and add its fields to the Schema.
     *
     *  All fields needed to write the outputs of a forced fit will be added to the given SchemaMapper's
     *  output schema.  Keys needed to retrieve the reference ellipses for the exp and dev fits will be
     *  extracted from the SchemaMapper's input schema.  In addition, keys needed to retrieve the PSF
     *  shapelet approximation (assuming the ShapeletPsfApprox plugin has been run) will be extracted
     *  from the SchemaMapper's output schema (note that the ShapeletPsfApprox plugin must be run in
     *  forced mode as well, to approximate the measurement image's PSF rather than the reference image's
     *  PSF, so its outputs are found in the output schema, not the input schema).
     *
     *  @param[in]     name    Name of the algorithm used as a prefix for all fields added to the Schema.
     *  @param[in]     ctrl    Control object that configures the algorithm.
     *  @param[in,out] schemaMapper  SchemaMapper containing input (reference) and output schemas.
     */
    CModelAlgorithm(
        std::string const & name,
        Control const & ctrl,
        afw::table::SchemaMapper & schemaMapper
    );

    /**
     *  Construct an algorithm instance that cannot use SourceRecords for input/output.
     *
     *  This constructor initializes the algorithm without initializing any of the keys necessary to
     *  operate on SourceRecords.  As a result, only methods that take inputs directly and return Result
     *  objects may be called.
     */
    explicit CModelAlgorithm(Control const & ctrl);

    /// Return the control object the algorithm was constructed with.
    Control const & getControl() const { return _ctrl; }

    /**
     *  @brief Determine the initial fit region for a CModelAlgorithm fit
     *
     *  This routine grows the given footprint by nGrowFootprint, then clips on the bounding box
     *  of the given mask and removes pixels indicated as bad by badMaskPlanes.
     *
     *  @throw meas::base::MeasurementError if the area exceeds CModelRegionControl::maxArea or the fraction
     *         of rejected pixels exceeds CModelRegionControl::maxBadPixelFraction.
     */
    PTR(afw::detection::Footprint) determineInitialFitRegion(
        afw::image::Mask<> const & mask,
        afw::detection::Footprint const & footprint,
        afw::geom::Box2I const & psfBBox
    ) const;

    /**
     *  @brief Determine the final fit region for a CModelAlgorithm fit.
     *
     *  This routine grows the given footprint by nGrowFootprint, then extends it to include
     *  the given ellipse scaled by nInitialRadii.  It then clips on the bounding box of the
     *  given mask and removes pixels indicated as bad by badMaskPlanes.
     *
     *  @throw meas::base::MeasurementError if the area exceeds CModelRegionControl::maxArea or the fraction
     *         of rejected pixels exceeds CModelRegionControl::maxBadPixelFraction.
     */
    PTR(afw::detection::Footprint) determineFinalFitRegion(
        afw::image::Mask<> const & mask,
        afw::detection::Footprint const & footprint,
        afw::geom::Box2I const & psfBBox,
        afw::geom::ellipses::Ellipse const & ellipse
    ) const;

    /**
     *  Run the CModel algorithm on an image, supplying inputs directly and returning outputs in a Result.
     *
     *  @param[in]   exposure     Image to measure.  Must have a valid Psf, Wcs and Calib.
     *  @param[in]   footprint    Detection footprint of the object to be measured, used as a starting point
     *                            for the region of pixels to be fit.
     *  @param[in]   psf          multi-shapelet approximation to the PSF at the position of the source
     *  @param[in]   center       Centroid of the source to be fit.
     *  @param[in]   moments      Non-PSF-corrected moments of the source, used to initialize the model
     *                            parameters
     *  @param[in]   approxFlux   Rough estimate of the flux of the source, used to set the fit coordinate
     *                            system and ensure internal parameters are of order unity.  If less than
     *                            or equal to zero, the sum of the flux within the footprint will be used.
     */
    Result apply(
        afw::image::Exposure<Pixel> const & exposure,
        afw::detection::Footprint const & footprint,
        shapelet::MultiShapeletFunction const & psf,
        afw::geom::Point2D const & center,
        afw::geom::ellipses::Quadrupole const & moments,
        Scalar approxFlux=-1
    ) const;

    /**
     *  Run the CModel algorithm in forced mode on an image, supplying inputs directly and returning
     *  outputs in a Result.
     *
     *  @param[in]   exposure     Image to measure.  Must have a valid Psf, Wcs and Calib.
     *  @param[in]   footprint    Detection footprint of the object to be measured, used as a starting point
     *                            for the region of pixels to be fit.
     *  @param[in]   psf          multi-shapelet approximation to the PSF at the position of the source
     *  @param[in]   center       Centroid of the source to be fit.
     *  @param[in]   reference    Result object from a previous, non-forced run of CModelAlgorithm.
     *  @param[in]   approxFlux   Rough estimate of the flux of the source, used to set the fit coordinate
     *                            system and ensure internal parameters are of order unity.  If less than
     *                            or equal to zero, the sum of the flux within the footprint will be used.
     */
    Result applyForced(
        afw::image::Exposure<Pixel> const & exposure,
        afw::detection::Footprint const & footprint,
        shapelet::MultiShapeletFunction const & psf,
        afw::geom::Point2D const & center,
        Result const & reference,
        Scalar approxFlux=-1
    ) const;

    /**
     *  Run the CModel algorithm on an image, using a SourceRecord for inputs and outputs.
     *
     *  @param[in,out] measRecord  A SourceRecord instance used to provide a Footprint, the centroid and
     *                             shape of the source, a MultiShapeletFunction PSF, and an approximate
     *                             estimate of the (via the PsfFlux slot), and to which all outputs will
     *                             be written.
     *  @param[in]     exposure    Image to be measured.  Must have a valid Psf, Wcs, and Calib.
     *
     *  To run this method, the CModelAlgorithm instance must have been created using the constructor
     *  that takes a Schema argument, and that Schema must match the Schema of the SourceRecord passed here.
     */
     void measure(
        afw::table::SourceRecord & measRecord,
        afw::image::Exposure<Pixel> const & exposure
    ) const;

    /**
     *  Run the CModel algorithm in forced mode on an image, using a SourceRecord for inputs and outputs.
     *
     *  @param[in,out] measRecord  A SourceRecord instance used to provide a Footprint, the centroid of
     *                             the source, a MultiShapeletFunction PSF, and an approximate
     *                             estimate of the (via the PsfFlux slot), and to which all outputs will
     *                             be written.
     *  @param[in]     exposure    Image to be measured.  Must have a valid Psf, Wcs, and Calib.
     *  @param[in]     refRecord   A SourceRecord that contains the outputs of a previous non-forced run
     *                             of CModelAlgorithm (which may have taken place on an image with a
     *                             different Wcs).
     *
     *  To run this method, the CModelAlgorithm instance must have been created using the constructor
     *  that takes a Schema argument, and that Schema must match the Schema of the SourceRecord passed here.
     */
    void measure(
        afw::table::SourceRecord & measRecord,
        afw::image::Exposure<Pixel> const & exposure,
        afw::table::SourceRecord const & refRecord
    ) const;

    /**
     *  Handle an exception thrown by one of the measure() methods, setting the appropriate flag in
     *  the given record.
     *
     *  @param[out]   measRecord   Record on which the flag should be set.
     *  @param[in]    error        Error containing the bit to be set.  If null, only the general
     *                             failure bit will be set.
     */
    void fail(
        afw::table::SourceRecord & measRecord,
        meas::base::MeasurementError * error
    ) const;

    /// Copy values from a Result struct to a BaseRecord object.
    void writeResultToRecord(Result const & result, afw::table::BaseRecord & record) const;

private:

    friend class CModelAlgorithmControl;

    // Actual implementations go here; we use an output argument for the result so we can get partial
    // results to the plugin version when we throw.
    void _applyImpl(
        Result & result,
        afw::image::Exposure<Pixel> const & exposure,
        afw::detection::Footprint const & footprint,
        shapelet::MultiShapeletFunction const & psf,
        afw::geom::Point2D const & center,
        afw::geom::ellipses::Quadrupole const & moments,
        Scalar approxFlux
    ) const;

    // Actual implementations go here; we use an output argument for the result so we can get partial
    // results to the SourceRecord version when we throw.
    void _applyForcedImpl(
        Result & result,
        afw::image::Exposure<Pixel> const & exposure,
        afw::detection::Footprint const & footprint,
        shapelet::MultiShapeletFunction const & psf,
        afw::geom::Point2D const & center,
        Result const & reference,
        Scalar approxFlux
    ) const;

    // gets/checks inputs from SourceRecord that are needed by both apply and applyForced
    template <typename PixelT>
    shapelet::MultiShapeletFunction _processInputs(
        afw::table::SourceRecord & source,
        afw::image::Exposure<PixelT> const & exposure
    ) const;

    class Impl;

    Control _ctrl;
    PTR(Impl) _impl;
};

}}} // namespace lsst::meas::multifit

#endif // !LSST_MEAS_MULTIFIT_CModelFit_h_INCLUDED
