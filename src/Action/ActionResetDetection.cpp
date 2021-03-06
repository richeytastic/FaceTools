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

#include <Action/ActionResetDetection.h>
#include <FaceModel.h>
#include <QMessageBox>
using FaceTools::Action::ActionResetDetection;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using MS = FaceTools::ModelSelect;
using QMB = QMessageBox;


ActionResetDetection::ActionResetDetection( const QString& dn, const QIcon& icon)
    : FaceAction(dn, icon)
{
    addRefreshEvent( Event::MASK_CHANGE);
}   // end ctor


bool ActionResetDetection::isAllowed( Event)
{
    const FM *fm = MS::selectedModel();
    return fm && fm->hasLandmarks();
}   // end isAllowed


bool ActionResetDetection::doBeforeAction( Event)
{
    QWidget* prnt = static_cast<QWidget*>(parent());
    static const QString msg = tr("Really reset the detection? This will affect all assessments!");
    return QMB::Yes == QMB::warning( prnt, displayName(), QString("<p align='center'>%1</p>").arg(msg),
                                      QMB::Yes | QMB::No, QMB::No);
}   // end doBeforeAction


void ActionResetDetection::doAction( Event)
{
    storeUndo( this, Event::MASK_CHANGE | Event::LANDMARKS_CHANGE);

    FM* fm = MS::selectedModel();
    fm->lockForWrite();
    fm->setMask( nullptr);
    static const Landmark::LandmarkSet emptyLmks;
    const IntSet aids = fm->assessmentIds();
    for ( int aid : aids)
        fm->assessment(aid)->setLandmarks( emptyLmks);
    fm->remakeBounds();
    fm->unlock();
}   // end doAction


Event ActionResetDetection::doAfterAction( Event)
{
    MS::showStatus("Correspondence mask and landmarks removed!", 5000);
    return Event::MASK_CHANGE | Event::LANDMARKS_CHANGE | Event::VIEW_CHANGE;
}   // end doAfterAction

