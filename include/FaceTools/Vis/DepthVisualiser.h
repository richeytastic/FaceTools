/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_VIS_DEPTH_VISUALISER_H
#define FACE_TOOLS_VIS_DEPTH_VISUALISER_H

#include "LineVisualiser.h"

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT DepthVisualiser : public LineVisualiser
{
public:
    void refresh( FV*) override;
};  // end class

}}   // end namespaces

#endif
