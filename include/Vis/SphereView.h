/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#ifndef FACE_TOOLS_SPHERE_VIEW_H
#define FACE_TOOLS_SPHERE_VIEW_H

#include <ModelViewer.h>
#include <VtkScalingActor.h>    // RVTK
#include <vtkSphereSource.h>
#include <vtkCaptionActor2D.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT SphereView
{
public:
    SphereView( const cv::Vec3f& centre=cv::Vec3f(0,0,0), double radius=1.0, bool pickable=true, bool fixedScale=false);
    SphereView( const SphereView&);
    SphereView& operator=( const SphereView&);
    virtual ~SphereView();

    void setResolution( int);   // Default 8
    int resolution() const;

    void setPickable( bool);
    bool pickable() const;

    void setFixedScale( bool);
    bool fixedScale() const;

    void setScaleFactor( double);
    double scaleFactor() const;

    void setCentre( const cv::Vec3f&);
    const cv::Vec3f& centre() const;

    void setRadius( double);                            // Set radius
    double radius() const;                              // Get radius

    void setOpacity( double);
    double opacity() const;

    void setColour( double r, double g, double b);      // Set colour as rgb components in [0,1].
    void setColour( const double[3]);
    const double* colour() const;                       // Return a 3-tuple array for the rgb components.

    void setCaption( const std::string&);               // Set caption used when highlighting.
    void setCaption( const QString&);
    std::string caption() const;

    void setCaptionColour( const QColor&);

    void setHighlighted( bool);                         // Show the caption (only if already visible).
    bool highlighted() const;

    void setVisible( bool, ModelViewer*);               // Set visibility of actors.
    bool visible() const { return _visible;}

    const vtkProp* prop() const;     

private:
    bool _visible;
    RVTK::VtkScalingActor* _actor;
    vtkNew<vtkSphereSource> _source;
    vtkNew<vtkCaptionActor2D> _caption;
    void init();
};  // end class

}}   // end namespaces

#endif
