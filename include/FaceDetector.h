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

#ifndef FACE_TOOLS_FACE_DETECTOR_H
#define FACE_TOOLS_FACE_DETECTOR_H

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "Landmarks.h"
#include "ObjMetaData.h"
#include "ModelViewer.h"
#include "FaceTools_Export.h"


namespace FaceTools
{

class FaceTools_EXPORT FaceDetector
{
public:
    explicit FaceDetector( const std::string& faceShapeLandmarksDat);
    ~FaceDetector();

    int findLandmarks( ObjMetaData::Ptr);

private:
    ModelViewer *_viewer;   // Offscreen scene rendering
    struct Impl;
    Impl *_impl;

    FaceDetector( const FaceDetector&);     // NO COPY
    void operator=( const FaceDetector&);   // NO COPY
};  // end class


}   // end namespace

#endif
