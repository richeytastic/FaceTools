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

#ifndef FACE_TOOLS_DETECT_FACE_FINDER_2D_H
#define FACE_TOOLS_DETECT_FACE_FINDER_2D_H

#include "FaceTools_Export.h"
#include <opencv2/opencv.hpp>
typedef unsigned char byte;

namespace FaceTools {
namespace Detect {

class FaceTools_EXPORT FaceFinder2D
{
public:
    // FeaturesDetector::initialise must have been called already!
    bool find( const cv::Mat_<byte> lightMap);  // Looks for face and eyes only

    // Get the absolute positions in the view (as proportions of the view size)
    // of the respective face elements.
    const cv::RotatedRect& getLeftEyeBox() const { return _leye;}
    const cv::RotatedRect& getRightEyeBox() const { return _reye;}

    cv::Point2f getLEyeCentre() const { return _leye.center;}
    cv::Point2f getREyeCentre() const { return _reye.center;}

    // Draw debug output on given image and return it.
    cv::Mat_<cv::Vec3b> drawDebug( cv::Mat_<cv::Vec3b>) const;

private:
    // All feature boxes are stored as proportions of the view
    cv::RotatedRect _faceBox;
    cv::RotatedRect _leye, _reye;  // Left and right eye (from viewer's perspective)
    cv::RotatedRect _interPupilSpace;
    bool findEyes( const cv::Mat_<byte>);
};  // end class

}   // end namespace
}   // end namespace

#endif