/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_ACTION_RADIAL_SELECT_H
#define FACE_TOOLS_ACTION_RADIAL_SELECT_H

/**
 * A visualisation with an internally constructed interactor to adjust
 * the radially visualised area on a set of views for a FaceModel.
 */

#include "ActionVisualise.h"
#include <FaceTools/Interactor/RadialSelectHandler.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionRadialSelect : public ActionVisualise
{ Q_OBJECT
public:
    ActionRadialSelect( const QString&, const QIcon&, Interactor::RadialSelectHandler::Ptr);

    QString toolTip() const override { return "Select a radially bounded region of the model.";}

protected:
    bool checkState( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;

private:
    Interactor::RadialSelectHandler::Ptr _handler;
};  // end class

}}   // end namespace

#endif