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

#include <LndMrk/LandmarkSet.h>
#include <LndMrk/LandmarksManager.h>
#include <FaceTools.h>
#include <FaceModel.h>
#include <r3d/SurfacePointFinder.h>
#include <cassert>
using FaceTools::Landmark::LandmarkSet;
using FaceTools::Landmark::Landmark;
using FaceTools::Landmark::LmkList;
using SLmk = FaceTools::Landmark::SpecificLandmark;
using FaceTools::FaceSide;
using FaceTools::Vec3f;
using FaceTools::Mat4f;
using FaceTools::MatX3f;
using LDMRK_PAIR = std::pair<int, Vec3f>;
using LMAN = FaceTools::Landmark::LandmarksManager;


namespace  {
void addLandmarks( const std::unordered_map<int, Vec3f>& vs, FaceSide lat, LandmarkSet &mlmks)
{
    for ( const auto& p : vs)
    {
        const int lmid = p.first;
        Vec3f v = Vec3f::Zero();
        if ( mlmks.has(lmid, lat))
            v = mlmks.pos(lmid, lat);
        v += p.second;
        mlmks.set( lmid, v, lat);
    }   // end for
}   // end addLandmarks


void setAverage( std::unordered_map<int, Vec3f>& lat, int n)
{
    for ( auto& p : lat)
        lat[p.first] = p.second / n;
}   // end setAverage

}   // end namespace


LandmarkSet::LandmarkSet( const std::unordered_set<const LandmarkSet*>& lms)
{
    int n = 0;
    for ( const LandmarkSet* lmks : lms)
    {
        if ( lmks->empty())
            continue;

        n++;
        const std::unordered_map<int, Vec3f>& llat = lmks->lateral(LEFT);
        const std::unordered_map<int, Vec3f>& mlat = lmks->lateral(MID);
        const std::unordered_map<int, Vec3f>& rlat = lmks->lateral(RIGHT);

        addLandmarks( llat, LEFT, *this);
        addLandmarks( mlat, MID, *this);
        addLandmarks( rlat, RIGHT, *this);
    }   // end for

    if ( n > 0)
    {
        setAverage( _lmksL, n);
        setAverage( _lmksM, n);
        setAverage( _lmksR, n);
    }   // end if
}   // end ctor


bool LandmarkSet::has( int id, FaceSide lat) const
{
    if ( !has(id))
        return false;

    bool hasLmk = false;
    switch (lat)
    {
        case LEFT:
            hasLmk = _lmksL.count(id) > 0;
            break;
        case MID:
            hasLmk = _lmksM.count(id) > 0;
            break;
        case RIGHT:
            hasLmk = _lmksR.count(id) > 0;
            break;
    }   // end switch
    return hasLmk;
}   // end has


bool LandmarkSet::has( const SLmk& p) const { return has(p.id, p.lat);}


const LandmarkSet::LDMRKS& LandmarkSet::lateral( FaceSide lat) const
{
    const LDMRKS* lmks = nullptr;
    switch (lat)
    {
        case LEFT:
            lmks = &_lmksL;
            break;
        case MID:
            lmks = &_lmksM;
            break;
        case RIGHT:
            lmks = &_lmksR;
            break;
    }   // end switch
    return *lmks;
}   // end lateral


LandmarkSet::LDMRKS& LandmarkSet::_lateral( FaceSide lat)
{
    const LandmarkSet* me = this;
    return const_cast<LDMRKS&>( me->lateral(lat));
}   // end _lateral


bool LandmarkSet::set( int id, const Vec3f& v, FaceSide lat)
{
    Landmark* lmk = LMAN::landmark(id);
    if ( !lmk)
        return false;

    if ( !lmk->isBilateral())   // Ignore specified lateral if landmark is not bilateral
        lat = MID;

    _lateral(lat)[id] = v;
    _ids.insert(id);
    return true;
}   // end set


bool LandmarkSet::set( const QString& lmcode, const Vec3f& v, FaceSide lat)
{
    Landmark* lmk = LMAN::landmark(lmcode);
    if ( !lmk)
        return false;
    return set( lmk->id(), v, lat);
}   // end set


const Vec3f& LandmarkSet::pos( int id, FaceSide lat) const
{
    static const Vec3f ZERO_VEC = Vec3f::Zero();    // Otherwise returning reference to temporary

    if ( LMAN::landmark(id)->isBilateral() && lat == MID)
    {
        std::cerr << "[ERROR] FaceTools::Landmark::LandmarkSet::pos: Requested landmark is bilateral but requested medial!" << std::endl;
        assert(false);
        return ZERO_VEC;
    }   // end if

    assert(has(id, lat));
    if (!has(id, lat))
        return ZERO_VEC;

    return lateral(lat).at(id);
}   // end pos


const Vec3f& LandmarkSet::pos( const QString& lmcode, FaceSide lat) const
{
    return pos( LMAN::landmark(lmcode)->id(), lat);
}   // end pos


const Vec3f& LandmarkSet::pos( const SLmk& sl) const { return pos( sl.id, sl.lat);}


Vec3f LandmarkSet::toPoint( const LmkList &ll, const Mat4f& T, const Mat4f& iT) const
{
    Vec3f v = Vec3f::Zero();
    for ( const SLmk &slmk : ll)
        v += slmk.prop.cwiseProduct( r3d::transform( iT, pos(slmk)));
    return r3d::transform( T, v);
}   // end toPoint


r3d::Mesh::Ptr LandmarkSet::toMesh() const
{
    std::vector<int> lmids( ids().begin(), ids().end());
    std::sort( lmids.begin(), lmids.end());

    r3d::Mesh::Ptr mesh = r3d::Mesh::create();
    for ( int id : lmids)
    {
        if ( LMAN::landmark(id)->isBilateral())
        {
            mesh->addVertex( pos( id, LEFT));
            mesh->addVertex( pos( id, RIGHT));
        }   // end if
        else
            mesh->addVertex( pos( id, MID));
    }   // end for

    return mesh;
}   // end toMesh


Vec3f LandmarkSet::eyeVec() const
{
    Vec3f v = Vec3f::Zero();
    if ( has( LMAN::codeId(P)))
    {
        const int id = LMAN::landmark(P)->id(); // Get the id of the pupil landmark
        v = pos( id, RIGHT) - pos( id, LEFT);
    }   // end if
    return v;
}   // end eyeVec


Vec3f LandmarkSet::midEyePos() const
{
    Vec3f v = Vec3f::Zero();
    if ( has( LMAN::codeId(P)))
    {
        const int id = LMAN::landmark(P)->id(); // Get the id of the pupil landmark
        v = 0.5f*(pos( id, RIGHT) + pos( id, LEFT));
    }   // end if
    return v;
}   // end midEyePos

namespace {

Vec3f getMeanOfSet( const IntSet &ms, const std::unordered_map<int, Vec3f> &lmks)
{
    int cnt = 0;
    Vec3f p = Vec3f::Zero();
    for ( int lid : ms)
    {
        if ( lmks.count(lid) > 0)
        {
            p += lmks.at(lid);
            cnt++;
        }   // end if
    }   // end for
    if ( cnt > 0)
        p /= cnt;
    return p;
}   // end getMeanOfSet

}   // end namespace


Vec3f LandmarkSet::medialMean() const { return getMeanOfSet( LMAN::medialAlignmentSet(), _lmksM);}


namespace {

void testSetExtreme( Vec3f &minc, Vec3f &maxc, const Vec3f &v)
{
    minc[0] = std::min( minc[0], v[0]);
    maxc[0] = std::max( maxc[0], v[0]);
    minc[1] = std::min( minc[1], v[1]);
    maxc[1] = std::max( maxc[1], v[1]);
    minc[2] = std::min( minc[2], v[2]);
    maxc[2] = std::max( maxc[2], v[2]);
}   // end testSetExtreme

}   // end namespace


r3d::Bounds::Ptr LandmarkSet::makeBounds( const Mat4f &T, const Mat4f &iT) const
{
    Vec3f minc( FLT_MAX, FLT_MAX, FLT_MAX);
    Vec3f maxc = -minc;

    for ( const auto& p : _lmksL)
        testSetExtreme( minc, maxc, r3d::transform( iT, p.second));
    for ( const auto& p : _lmksM)
        testSetExtreme( minc, maxc, r3d::transform( iT, p.second));
    for ( const auto& p : _lmksR)
        testSetExtreme( minc, maxc, r3d::transform( iT, p.second));

    const Vec3f cen = r3d::transform( iT, medialMean());

    static const float X_FACTOR = 1.0f;
    static const float Yt_FACTOR = 1.0f;
    static const float Yb_FACTOR = 1.0f;
    static const float Z_FACTOR = 1.0f;
    minc[0] = cen[0] - X_FACTOR * fabsf(minc[0] - cen[0]);
    maxc[0] = cen[0] + X_FACTOR * fabsf(maxc[0] - cen[0]);
    minc[1] = cen[1] - Yb_FACTOR * fabsf(minc[1] - cen[1]);
    maxc[1] = cen[1] + Yt_FACTOR * fabsf(maxc[1] - cen[1]);
    minc[2] = cen[2] - Z_FACTOR * fabsf(minc[2] - cen[2]);
    
    r3d::Bounds::Ptr bnds = r3d::Bounds::create( minc, maxc);
    bnds->setTransformMatrix( T);
    return bnds;
}   // end makeBounds


float LandmarkSet::sqRadius() const
{
    float sqDist = 0.0f;
    const Vec3f mean = medialMean();
    for ( const auto& p : _lmksL)
        sqDist = std::max( (p.second - mean).squaredNorm(), sqDist);
    for ( const auto& p : _lmksM)
        sqDist = std::max( (p.second - mean).squaredNorm(), sqDist);
    for ( const auto& p : _lmksR)
        sqDist = std::max( (p.second - mean).squaredNorm(), sqDist);
    return sqDist;
}   // end sqRadius


namespace {

void nearestFromSet( FaceSide side, const std::unordered_map<int, Vec3f> &lmksSet,
                     const SLmk &slmk, const Vec3f &slmkPos, float &sqDist, SLmk &nlmk)
{
    for ( const auto& p : lmksSet)
    {
        if ( p.first == slmk.id && slmk.lat == side)
            continue;
        const float sqd = (p.second - slmkPos).squaredNorm();
        if ( sqd < sqDist)
        {
            sqDist = sqd;
            nlmk.id = p.first;
            nlmk.lat = side;
        }   // end if
    }   // end for
}   // end nearestFromSet

}   // end namespace


SLmk LandmarkSet::nearest( const SLmk &slmk) const
{
    const Vec3f &lmkPos = pos(slmk);
    float sqDist = FLT_MAX;
    SLmk nlmk;
    nearestFromSet( LEFT,  _lmksL, slmk, lmkPos, sqDist, nlmk);
    nearestFromSet( MID,   _lmksM, slmk, lmkPos, sqDist, nlmk);
    nearestFromSet( RIGHT, _lmksR, slmk, lmkPos, sqDist, nlmk);
    return nlmk;
}   // end nearest

SLmk LandmarkSet::nearest( int id, FaceSide lat) const { return nearest( SLmk( id, lat));}


void LandmarkSet::swapLaterals()
{
    const auto tmp = _lmksL;
    _lmksL = _lmksR;
    _lmksR = tmp;
}   // end swapLaterals


void LandmarkSet::moveToSurface( const FaceTools::FM* fm)
{
    for ( const auto& p : _lmksL)
        _lmksL.at(p.first) = FaceTools::toSurface( fm->kdtree(), p.second);
    for ( const auto& p : _lmksM)
        _lmksM.at(p.first) = FaceTools::toSurface( fm->kdtree(), p.second);
    for ( const auto& p : _lmksR)
        _lmksR.at(p.first) = FaceTools::toSurface( fm->kdtree(), p.second);
}   // end moveToSurface


void LandmarkSet::transform( const Mat4f& t)
{
    assert( !t.isZero());
    for ( auto& p : _lmksL)
        p.second = r3d::transform( t, p.second);
    for ( auto& p : _lmksM)
        p.second = r3d::transform( t, p.second);
    for ( auto& p : _lmksR)
        p.second = r3d::transform( t, p.second);
}   // end transform


// Write out the landmarks to record.
void LandmarkSet::write( PTree& lnodes) const
{
    PTree& llat = lnodes.put("LeftSide", "");
    for ( const auto& p : _lmksL)
        r3d::putNamedVertex( llat, LMAN::landmark(p.first)->code().toStdString(), p.second);
    PTree& mlat = lnodes.put("Medial", "");
    for ( const auto& p : _lmksM)
        r3d::putNamedVertex( mlat, LMAN::landmark(p.first)->code().toStdString(), p.second);
    PTree& rlat = lnodes.put("RightSide", "");
    for ( const auto& p : _lmksR)
        r3d::putNamedVertex( rlat, LMAN::landmark(p.first)->code().toStdString(), p.second);
}   // end write


bool LandmarkSet::_readLateral( const PTree& lats, FaceSide lat)
{
    std::string tag = "Medial";

    if ( lat == LEFT)
    {
        tag = "RightLateral";   // Note swapped in old version!
        if ( lats.count(tag) == 0)
            tag = "LeftSide";
    }   // end if
    else if ( lat == RIGHT)
    {
        tag = "LeftLateral";    // Note swapped in old version!
        if ( lats.count(tag) == 0)
            tag = "RightSide";
    }   // end else if

    if ( lats.count(tag) == 0)
    {
        std::cerr << "[ERROR] FaceTools::Landmark::LandmarkSet::_readLateral: Didn't find landmark lateral tag!" << std::endl;
        return false;
    }   // end if

    // Read in all the landmarks present in the tree
    for ( const PTree::value_type& lval : lats.get_child( tag))
    {
        const QString lmcode = lval.first.c_str();
        Landmark* lmk = LMAN::landmark( lmcode);
        if ( !lmk)
        {
#ifndef NDEBUG
            std::cerr << "[WARNING] FaceTools::Landmark::LandmarkSet::_readLateral: Unable to read landmark '"
                      << lmcode.toStdString() << "'; not present in LMAN!" << std::endl;
#endif
            return false;
        }   // end if
        set( lmk->id(), r3d::getVertex(lval.second), lat);
    }   // end for

    return true;
}   // end _readLateral


// Read in the landmarks from record.
bool LandmarkSet::read( const PTree& lnodes)
{
    if ( !_readLateral( lnodes, LEFT))
        return false;
    if ( !_readLateral( lnodes, MID))
        return false;
    if ( !_readLateral( lnodes, RIGHT))
        return false;
    return true;
}   // end read


namespace {
void checkCloser( float &minSqDist, Vec3f &nv, const Vec3f &vm, const Vec3f &v, float snapSqDist)
{
    const float sqdist = (vm - v).squaredNorm();
    if (sqdist < snapSqDist && sqdist < minSqDist)
    {
        minSqDist = sqdist;
        nv = vm;
    }   // end if
}   // end checkCloser
}   // end namespace


Vec3f LandmarkSet::snapTo( const Vec3f &v, float snapSqDist) const
{
    Vec3f nv = v;
    float minSqDist = FLT_MAX;
    for ( int lmid : _ids)
    {
        if ( LMAN::landmark(lmid)->isVisible())
        {
            if ( LMAN::isBilateral(lmid))
            {
                checkCloser( minSqDist, nv, pos(lmid, LEFT), v, snapSqDist);
                checkCloser( minSqDist, nv, pos(lmid, RIGHT), v, snapSqDist);
            }   // end if
            else
                checkCloser( minSqDist, nv, pos(lmid, MID), v, snapSqDist);
        }   // end if
    }   // end for
    return nv;
}   // end snapTo
