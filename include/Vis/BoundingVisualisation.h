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

#ifndef FACE_TOOLS_VIS_BOUNDING_VISUALISATION_H
#define FACE_TOOLS_VIS_BOUNDING_VISUALISATION_H

#include "BaseVisualisation.h"
#include "BoundingView.h"

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT BoundingVisualisation : public BaseVisualisation
{ Q_OBJECT
public:
    ~BoundingVisualisation() override;

    void apply( FV*, const QPoint* mc=nullptr) override;
    bool purge( FV*, Action::Event) override;

    void setVisible( FV*, bool) override;
    bool isVisible( const FV*) const override;

    void syncActorsToData( const FV*, const cv::Matx44d& d=cv::Matx44d::eye()) override;

private:
    std::unordered_map<const FV*, BoundingView*> _views;
};  // end class

}}   // end namespace

#endif
