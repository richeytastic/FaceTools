/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#include <FaceDetector.h>
#include <FaceShapeLandmarks2DDetector.h>
#include <FeaturesDetector.h>
#include <FaceTools.h>
#include <NoseFinder.h>
#include <FaceFinder2D.h>
#include <FaceOrienter.h>
#include <MiscFunctions.h>
#include <FeatureUtils.h>   // RFeatures

using FaceTools::Detect::FaceDetector;
using FaceTools::Detect::FaceFinder2D;
using FaceTools::Detect::FaceOrienter;
using FaceTools::Detect::NoseFinder;
using FaceTools::Detect::FaceShapeLandmarks2DDetector;
using FaceTools::Detect::FeaturesDetector;
using FaceTools::LandmarkSet;

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cmath>
#include <cassert>
#include <VtkTools.h>           // RVTK
#include <ImageGrabber.h>       // RVTK
#include <VtkActorCreator.h>    // RVTK
#include <RendererPicker.h>     // RVTK
using RFeatures::ObjModel;
using RFeatures::Orientation;
using RFeatures::ObjModelKDTree;
using RVTK::ImageGrabber;

#ifndef NDEBUG
#define SHOW_DEBUG
#endif

namespace {

cv::Mat_<cv::Vec3b> snapshot( RVTK::Viewer::Ptr v) { return ImageGrabber(v).colour();}


cv::Mat_<cv::Vec3b> snapshot( RVTK::Viewer::Ptr viewer, auto fpts)
{
    cv::Mat_<cv::Vec3b> cmap = snapshot(viewer);
    std::vector<cv::Point> pts;
    std::for_each( std::begin(fpts), std::end(fpts), [&](auto f){ pts.push_back( cv::Point(f.x*cmap.cols, f.y*cmap.rows));});
    RFeatures::drawPoly( pts, cmap, cv::Scalar(255,100,100));
    return cmap;
}   // end snapshot


std::string getEyePoints( vtkRenderer* ren, cv::Point2f& f0, cv::Vec3f& v0,
                                            cv::Point2f& f1, cv::Vec3f& v1,
                                                             cv::Vec3f& mp)
{
    RVTK::RendererPicker rpicker( ren, RVTK::RendererPicker::TOP_LEFT);
    cv::Point2f fmid = (f1 + f0) * 0.5f;
    // While the given image positions for the eyes aren't returning an actor, move the points closer
    // in towards the expected position of the nose. Picking points under these image positions can
    // fail if there are holes in the model on the eyes. Small holes over the eyes can happen because
    // some photogrammetric techniques are less robust to highly reflective surfaces.
    //
#ifdef SHOW_DEBUG
    std::cerr << "Initial eye detection 2D points:" << std::endl;
    std::cerr << "Left eye:  " << f0 << std::endl;
    std::cerr << "Right eye: " << f1 << std::endl;
    std::cerr << "Mid-point: " << fmid << std::endl;
#endif

    // If haven't found valid points in space within MAX_CHECK tries, the model is too full of holes and we give up.
    static const int MAX_CHECK = 20;
    int checkCount = 0;
    while ( !rpicker.pickActor(f0) && checkCount < MAX_CHECK)
    {
        f0.x += 0.005;
        checkCount++;
    }   // end while

    if ( checkCount == MAX_CHECK)
        return "Failed to pick 3D position from 2D left eye position!";

    checkCount = 0;
    while ( !rpicker.pickActor(f1) && checkCount < MAX_CHECK)
    {
        f1.x -= 0.005;
        checkCount++;
    }   // end while

    if ( checkCount == MAX_CHECK)
        return "Failed to pick 3D position from 2D right eye position!";

    checkCount = 0;
    while ( !rpicker.pickActor(fmid) && checkCount < MAX_CHECK)   // Very unlikely to ever be the case
    {
        fmid.y -= 0.005;
        checkCount++;
    }   // end while

    if ( checkCount == MAX_CHECK)
        return "Failed to pick 3D position from 2D eye mid-point position!";

    v0 = rpicker.pickWorldPosition( f0);
    v1 = rpicker.pickWorldPosition( f1);
    mp = rpicker.pickWorldPosition( fmid);

#ifdef SHOW_DEBUG
    std::cerr << "Final eye detection 2D points:" << std::endl;
    std::cerr << "Left eye:  " << f0   << " --> " << v0 << std::endl;
    std::cerr << "Right eye: " << f1   << " --> " << v1 << std::endl;
    std::cerr << "Mid-point: " << fmid << " --> " << mp << std::endl;
#endif
    return "";
}   // end getEyePoints


// Detect the initial oriention points for the eyes and nosetip.
std::string findOrientationPoints( RVTK::Viewer::Ptr viewer, const ObjModelKDTree::Ptr kdtree,
                                   cv::Vec3f& v0, cv::Vec3f& v1, cv::Vec3f& ntip)
{
    FaceFinder2D faceFinder;
    if ( !faceFinder.find( ImageGrabber(viewer).light()))
        return "Failed to find a candidate face from 2D image!";

    cv::Point2f f0 = faceFinder.getLEyeCentre();    // [0,1]
    cv::Point2f f1 = faceFinder.getREyeCentre();    // [0,1]
    // Get the points in space for the eyes. If model is too full of holes, fail.
    cv::Vec3f mp;   // mp will be the midpoint between the eyes
    std::string errMsg = getEyePoints( viewer->getRenderer(), f0, v0, f1, v1, mp);

#ifdef SHOW_DEBUG
    cv::Mat_<cv::Vec3b> dmat = faceFinder.drawDebug( snapshot(viewer, std::vector<cv::Point2f>({f0,f1})));
#endif
    if ( !errMsg.empty())
    {
#ifdef SHOW_DEBUG
        RFeatures::showImage( dmat, "FAILED: FaceFinder 2D points", false);
#endif
        return errMsg;
    }   // end if

    const ObjModel::Ptr m0 = kdtree->getObject();
#ifdef SHOW_DEBUG
    size_t count = 0;
    for ( int vidx : m0->getVertexIds())
    {
        if ( m0->vtx(vidx)[2] < mp[2])
            count++;
    }   // end for
    std::cerr << "Found " << count << " vertices further back than eye mid-point" << std::endl;
#endif

    // If model is too small around the crop point, fail.
    ObjModel::Ptr m1 = FaceTools::crop( m0, mp, kdtree->find(mp), 70);
    if ( m1->getNumFaces() < 50)
        return "Cropped model around point mid-eye point has < 50 polygons.";

    // Find the vertices of the clean submodel that are closest to the estimated world positions of the eyes
    RFeatures::ObjModelKDTree::Ptr kdm1 = RFeatures::ObjModelKDTree::create(m1);
    const int m1LeftEyeVidx = kdm1->find( v0);
    const int m1RightEyeVidx = kdm1->find( v1);
    RFeatures::ObjModelCurvatureMap::Ptr cmap = RFeatures::ObjModelCurvatureMap::create( m1, m1LeftEyeVidx);
    size_t numSmoothIterations = 10;
    RFeatures::ObjModelSmoother( cmap).smooth( 0.7, numSmoothIterations);
    NoseFinder m1NoseFinder( cmap, m1LeftEyeVidx, m1RightEyeVidx);

    std::string msg;
    if ( m1NoseFinder.find())
    {
        ntip = m1NoseFinder.getNoseTip();
#ifdef SHOW_DEBUG
        std::vector<cv::Point> pts;
        RVTK::RendererPicker rpicker( viewer->getRenderer(), RVTK::RendererPicker::TOP_LEFT);
        pts.push_back( rpicker.projectToImagePlane( m1->vtx(m1LeftEyeVidx)));
        pts.push_back( rpicker.projectToImagePlane( m1->vtx(m1RightEyeVidx)));
        pts.push_back( rpicker.projectToImagePlane( ntip));
        RFeatures::drawPoly( pts, dmat, cv::Scalar(100,100,255));
        RFeatures::showImage( dmat, "Discovered orientation points", false);
#endif
    }   // end if
    else
        msg = "Unable to discover nose-tip from determined 3D eye mid-point.";

    return msg;
}   // end findOrientationPoints

}   // end namespace




// private
bool FaceDetector::findOrientation( ObjModelKDTree::Ptr kdt, Orientation& on, LandmarkSet& lset)  // Nose tip, left and right eye centres set
{
    cv::Vec3f v0, v1, ntip;
    _err = findOrientationPoints( _viewer, kdt, v0, v1, ntip);
    if ( !_err.empty())
        return false;

    using namespace FaceTools::Landmarks;
    lset.set( NASAL_TIP, ntip);
    lset.set( L_EYE_CENTRE, v0);
    lset.set( R_EYE_CENTRE, v1);

    const cv::Vec3f mp = (v0 + v1 + ntip) * 1.0f/3;
    const double cropRad = 2*std::max(cv::norm(mp-v0), std::max(cv::norm(mp-v1), cv::norm(mp-ntip)));
    ObjModel::Ptr cmodel = FaceTools::crop( kdt->getObject(), mp, kdt->find(ntip), cropRad);

    // Smooth the cropped model
    RFeatures::ObjModelKDTree::Ptr kdtree = RFeatures::ObjModelKDTree::create(cmodel);
    int ntid = kdtree->find(ntip);
    RFeatures::ObjModelCurvatureMap::Ptr cmap = RFeatures::ObjModelCurvatureMap::create( cmodel, ntid);
    size_t numSmoothIterations = 10;
    RFeatures::ObjModelSmoother( cmap).smooth( 0.7, numSmoothIterations);

    cv::Vec3d nvec, uvec;
    FaceOrienter orienter( cmap, kdtree);
    orienter( v0, v1, nvec, uvec);
    on.norm() = nvec;
    on.up() = uvec;
    return true;
}   // end findOrientation


// public
bool FaceDetector::detect( ObjModelKDTree::Ptr kdt, Orientation& on, LandmarkSet& lset)
{
    if ( !s_initOk)
    {
        _err = "[ERROR] FaceTools::Detect::FaceDetector::detect: FaceDetector not initialised okay!";
        return false;
    }   // end if

    _err = "";
    _viewer->setCamera( RFeatures::CameraParams( cv::Vec3f( 0, 0, _orng)));    // Default camera for detecting orientation
    RVTK::VtkActorCreator actorCreator;
    _actors.clear();
    actorCreator.generateTexturedActors( kdt->getObject(), _actors);
    std::for_each( std::begin(_actors), std::end(_actors), [this](auto a){ _viewer->addActor(a);});
    _viewer->resetClippingRange();
    _viewer->updateRender();

#ifdef SHOW_DEBUG
    RFeatures::showImage( snapshot(_viewer), "Detection Image Pre Orientation Discovery", false);
#endif
    if ( !findOrientation( kdt, on, lset))
        return cleanUp();

    using namespace FaceTools::Landmarks;
    // Update the view to focus on the identified face centre using the landmark detection range.
    const cv::Vec3f fc = FaceTools::calcFaceCentre( on.up(), lset.pos( L_EYE_CENTRE), lset.pos( R_EYE_CENTRE), lset.pos( NASAL_TIP));
    _viewer->setCamera( RFeatures::CameraParams( _drng * on.norm() + fc, fc, on.up()));
    _viewer->updateRender();

#ifdef SHOW_DEBUG
    RFeatures::showImage( snapshot(_viewer), "Detection Image Post Orientation", false);
#endif
    if ( !FaceShapeLandmarks2DDetector::detect( _viewer, lset))
        return cleanUp( "Complete set of landmarks not found.");

    // VTK's projection of landmarks to model surface is not very accurate, so shift them to be incident with the surface.
    FaceTools::translateLandmarksToSurface( kdt, lset);
    return cleanUp();
}   // end detect


bool FaceDetector::cleanUp( std::string err)
{
    _err = err;
    std::cerr << _err << std::endl;
    std::for_each( std::begin(_actors), std::end(_actors), [this](auto a){ _viewer->removeActor(a);});
    return err.empty();
}   // end cleanUp


bool FaceDetector::cleanUp() { return cleanUp(_err);}


// static initialise
bool FaceDetector::s_initOk(false);


// public static
bool FaceDetector::initialise( const std::string& haarCascadesModelDir, const std::string& faceShapeLandmarksDat)
{
    s_initOk = true;
    if (!FeaturesDetector::initialise( haarCascadesModelDir))
        s_initOk = false;
    if (!FaceShapeLandmarks2DDetector::initialise( faceShapeLandmarksDat))
        s_initOk = false;
    return s_initOk;
}   // end initialise


// public
FaceDetector::FaceDetector( float orng, float drng)
    : _orng(orng), _drng(drng), _err("")
{
    if ( !s_initOk)
    {
        std::cerr << "[ERROR] FaceTools::Detect::FaceDetector: Must have successfully initialised FaceDetector first!" << std::endl;
        return;
    }   // end if

    // Offscreen viewer
    _viewer = RVTK::Viewer::create(true/*offscreen*/);
    _viewer->setSize(600,600);
    // Set flood lights in the viewer
    std::vector<RVTK::Light> lights;
    RVTK::createBoxLights( 600, lights, true);
    RVTK::resetLights( _viewer->getRenderer(), lights);
}   // end ctor

