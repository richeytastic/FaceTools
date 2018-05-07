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

#include <SphereView.h>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkMapper.h>
using FaceTools::Vis::SphereView;
using FaceTools::ModelViewer;


SphereView::SphereView( const cv::Vec3f& c, double r)
    : _viewer(NULL),
      _source( vtkSmartPointer<vtkSphereSource>::New()),
      _actor( vtkSmartPointer<vtkActor>::New()),
      _caption( vtkSmartPointer<vtkCaptionActor2D>::New()),
      _ishighlighted(false), _isshown(false)
{
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection( _source->GetOutputPort());
    _actor->SetMapper(mapper);

    _caption->BorderOff();
    _caption->GetCaptionTextProperty()->BoldOff();
    _caption->GetCaptionTextProperty()->ItalicOff();
    _caption->GetCaptionTextProperty()->ShadowOff();
    _caption->GetCaptionTextProperty()->SetFontFamilyToCourier();
    _caption->GetCaptionTextProperty()->SetFontSize(4);
    _caption->GetCaptionTextProperty()->SetUseTightBoundingBox(true);
    _caption->SetPickable(false);

    setCentre(c);
    setRadius(r);
}   // end ctor


SphereView::~SphereView()
{
    setVisible( false, NULL);
}   // end dtor


// public
void SphereView::setCaption( const std::string& lname)
{
    _caption->SetCaption( lname.c_str());
}   // end setCaption


// public
void SphereView::setCentre( const cv::Vec3f& pos)
{
    _source->SetCenter( pos[0], pos[1], pos[2]);
    _source->Update();
    double attachPoint[3] = {pos[0], pos[1], pos[2]};
    _caption->SetAttachmentPoint( attachPoint);
}   // end setCentre


// public
cv::Vec3f SphereView::centre() const
{
    double vp[3];
    _source->GetCenter( vp);
    return cv::Vec3f( float(vp[0]), float(vp[1]), float(vp[2]));
}   // end centre


// public
void SphereView::setRadius( double r)
{
    _source->SetRadius(r);
    _source->Update();
}   // end setRadius


// public
double SphereView::radius() const
{
    return _source->GetRadius();
}   // end radius


// public
void SphereView::setVisible( bool enable, ModelViewer* viewer)
{
    if ( _viewer)
    {
        _viewer->remove(_actor);
        _viewer->remove(_caption);
    }   // end if

    _isshown = false;
    _ishighlighted = false;
    _viewer = viewer;
    if ( enable && _viewer)
    {
        _viewer->add(_actor);
        _isshown = true;
    }   // end if
}   // end setVisible


// public
bool SphereView::isVisible() const { return _isshown;}
bool SphereView::isHighlighted() const { return _ishighlighted;}
bool SphereView::isProp( const vtkProp* prop) const { return _actor == prop;}


// public
void SphereView::highlight( bool enable)
{
    double opacity = 1.0;
    _ishighlighted = enable && isVisible();
    if ( _ishighlighted)
        opacity = 0.4;
    _actor->GetProperty()->SetColor( 0.7,1,1);
    _actor->GetProperty()->SetOpacity( opacity);

    _caption->GetCaptionTextProperty()->SetColor( 1,1,1);
    _caption->SetVisibility( true);

    if ( _viewer)
    {
        _viewer->remove(_caption);
        if ( _ishighlighted)
            _viewer->add(_caption);
    }   // end if
}   // end highlight


bool SphereView::pointedAt( const QPoint& p) const
{
    bool pointedAt = false;
    if ( _viewer)
        pointedAt = _viewer->getPointedAt( &p, _actor);
    return pointedAt;
}   // end pointedAt