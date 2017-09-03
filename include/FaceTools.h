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

#ifndef FACE_TOOLS_H
#define FACE_TOOLS_H

#include <stdexcept>
#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <cmath>
#include <ObjModelTools.h>              // RFeatures
#include <DijkstraShortestPathFinder.h> // RFeatures
#include <HaarCascadeDetector.h>        // RFeatures

#include "FacesCohort.h"
#include "FaceTools_Export.h"
#include "FeaturesDetector.h"
#include "FaceDetector.h"
#include "FaceModelManager.h"
#include "FaceOrienter.h"
#include "MiscFunctions.h"
#include "ObjMetaData.h"
#include "RegistrationFace.h"
#include "CorrespondenceFinder.h"
#include "CurvatureSpeedFunctor.h"
#include "CurvatureVariableSpeedFunctor.h"
#include "SurfaceMesher.h"
#include "ModelViewer.h"
#include "Landmarks.h"

namespace FaceTools
{

// Find the boundaries of the given model and create and return a new model being
// the single component that is contiguously connected by polygons to vidx.
// Optional translation offset is applied to the copied model.
FaceTools_EXPORT RFeatures::ObjModel::Ptr getComponent( const RFeatures::ObjModel::Ptr, int vidx, const cv::Vec3d& copyOffset=cv::Vec3d(0,0,0));

// Crop and copy the given model to be the part within radius from centre v.
// Optional translation offset is applied to the copied model.
// The returned object is guaranteed to be fully parsable (i.e., a single connected component).
FaceTools_EXPORT RFeatures::ObjModel::Ptr crop( const RFeatures::ObjModel::Ptr, const cv::Vec3f& v,
                                                double radius, const cv::Vec3d& copyOffset=cv::Vec3d(0,0,0));

// Calculate and return the centre of the face as the point directly behind the nose tip in the plane of the front of the eyes.
// Requires the left and right eye centre landmarks, the nose tip landmark, and orientation vectors for calculation.
FaceTools_EXPORT cv::Vec3f calcFaceCentre( const ObjMetaData::Ptr);

// Return the cropped subregion from around the face centre by G times the distance
// from the face centre to the eye centre (average of both eyes used).
FaceTools_EXPORT RFeatures::ObjModel::Ptr cropAroundFaceCentre( const ObjMetaData::Ptr, double G=2.3);

// Create a vertices only ObjModel from the given row of points.
FaceTools_EXPORT RFeatures::ObjModel::Ptr createFromVertices( const cv::Mat_<cv::Vec3f>& row);

// Create a vertices only ObjModel from the given subset of vertices.
FaceTools_EXPORT RFeatures::ObjModel::Ptr createFromSubset( const RFeatures::ObjModel::Ptr, const IntSet& vidxs);

// Given a source model and a subset of vertices (vidxs), create and return a new
// points only model consisting of the given vertices transformed using matrix T.
// On return, if newVidxsToOld is not null, it is set with vertex mappings of the
// new (returned) object to the given vertex indices on the source model (vidxs).
FaceTools_EXPORT RFeatures::ObjModel::Ptr createFromTransformedSubset( const RFeatures::ObjModel::Ptr source, const IntSet& vidxs,
                                                                       const cv::Matx44d& T, boost::unordered_map<int,int>* newVidxsToOld=NULL);

// Flatten the source face to be in the XY plane, and set entries in provided map (if given) to key the source vertices
// from the new vertices in the returned (flattened) model.
FaceTools_EXPORT RFeatures::ObjModel::Ptr makeFlattened( const RFeatures::ObjModel::Ptr source, boost::unordered_map<int,int>* newVidxsToOld=NULL);

// Clean the given model to ensure it's a triangulated manifold (wraps RFeatures::ObjModelCleaner).
FaceTools_EXPORT void clean( RFeatures::ObjModel::Ptr);


// Returns the number of holes filled. Assumes model is a triangulated mesh.
FaceTools_EXPORT int fillHoles( RFeatures::ObjModel::Ptr);


// Collapse polygons in the given model that have area less than A. Returns # polygons collapsed.
FaceTools_EXPORT int collapseSmallPolygons( RFeatures::ObjModel::Ptr, double A);

// Returns NULL if cannot load or if can't clean (if doClean true).
FaceTools_EXPORT RFeatures::ObjModel::Ptr loadModel( const std::string& fname,
                                                     bool loadTexture=false, bool doClean=false);

// Returns false if unable to load any
FaceTools_EXPORT bool loadModels( const std::vector<std::string>& fnames,
                                  std::vector<RFeatures::ObjModel::Ptr>& models,
                                  bool loadTexture=false, bool doClean=false);

}   // end namespace

#endif


