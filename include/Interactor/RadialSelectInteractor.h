/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#ifndef FACE_TOOLS_RADIAL_SELECT_INTERACTOR_H
#define FACE_TOOLS_RADIAL_SELECT_INTERACTOR_H

#include "FaceViewInteractor.h"
#include <LoopSelectVisualisation.h>
#include <ObjModelRegionSelector.h> // RFeatures

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT RadialSelectInteractor : public FaceViewInteractor
{ Q_OBJECT
public:
    RadialSelectInteractor( Vis::LoopSelectVisualisation&, const FM*);

    // Return the model this region selector is for.
    const FM* model() const { return _model;}

    // Use after initialisation (centre and radius managed internally thereafter).
    void set( const cv::Vec3f& centre, double radius);

    double radius() const;
    cv::Vec3f centre() const;
    size_t selectedFaces( IntSet&) const;

protected:
    void enterProp( Vis::FV*, const vtkProp*) override;
    void leaveProp( Vis::FV*, const vtkProp*) override;

    bool leftButtonDown() override;
    bool leftButtonUp() override;
    bool leftDrag() override;

    bool mouseWheelForward() override;
    bool mouseWheelBackward() override;

private:
    Vis::LoopSelectVisualisation &_vis;
    bool _onReticule, _move;
    const FM* _model;   // The model the region selector is for.
    double _radiusChange;
    RFeatures::ObjModelRegionSelector::Ptr _rsel;

    void _showHover(bool);
    void _updateVis();
};  // end class

}}   // end namespaces

#endif
