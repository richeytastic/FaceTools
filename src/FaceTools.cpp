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

#include <FaceTools.h>
#include <Landmarks.h>
using FaceTools::ObjMetaData;
#include <AssetImporter.h>             // RModelIO
#include <ObjModelBoundaryFinder2.h>
using namespace RFeatures;


int FaceTools::findBoundaryLoops( const ObjModel::Ptr model, std::list<std::vector<cv::Vec3f> > &loops)
{
    ObjModelBoundaryFinder2 bfinder(model);
    const int nbs = bfinder.findOrderedBoundaryVertices();
    bfinder.sortBoundaries(true);

    for ( int i = 0; i < nbs; ++i)
    {
        const std::list<int>& blist = bfinder.getBoundary(i);
        //assert( model->getConnectedVertices( blist.back()).count( blist.front()) > 0); // Beginning of boundary must join end
        //std::cerr << "  + boundary has " << blist.size() << " vertices" << std::endl;
        loops.resize( loops.size() + 1);
        std::vector<cv::Vec3f>& lvec = loops.back();
        BOOST_FOREACH ( int b, blist)
            lvec.push_back( model->vtx(b));
    }   // end for
    return nbs;
}   // end findBoundaryLoops


ObjModel::Ptr FaceTools::getComponent( const ObjModel::Ptr model, int svidx)
{
    if ( model->getVertexIds().count(svidx) == 0)
    {
        std::cerr << "[ERROR] FaceTools::getComponent: Invalid starting vertex!" << std::endl;
        return ObjModel::Ptr();
    }   // end if

    ObjModelTriangleMeshParser parser( model);
    parser.parse( *model->getFaceIds( svidx).begin());
    const IntSet& fids = parser.getParsedFaces();

    ObjModelCopier copier( model);
    BOOST_FOREACH ( int fid, fids)
        copier.addTriangle(fid);
    return copier.getCopiedModel();
}   // end getComponent


ObjModel::Ptr FaceTools::crop( const ObjModel::Ptr model, const cv::Vec3f& v, int svidx, double radius)
{
    if ( model->getVertexIds().count(svidx) == 0)
    {
        std::cerr << "[ERROR] FaceTools::crop: Invalid starting vertex!" << std::endl;
        return ObjModel::Ptr();
    }   // end if

    if ( model->getFaceIds( svidx).empty())
    {
        std::cerr << "[ERROR] FaceTools::crop: Given starting vertex is not used in any of the model's polygons!" << std::endl;
        return ObjModel::Ptr();
    }   // end if

    ObjModelCropper::Ptr cropper = ObjModelCropper::create( model, v, svidx);
    cropper->adjustRadius( radius);
    IntSet cfids;
    cropper->getCroppedFaces( cfids);
    return crop( model, cfids);
}   // end crop


ObjModel::Ptr FaceTools::crop( const ObjModel::Ptr model, const IntSet& cfids)
{
    // Copy the subset of faces into a new model
    ObjModelCopier copier( model);
    BOOST_FOREACH ( int fid, cfids)
        copier.addTriangle(fid);
    ObjModel::Ptr cmodel = copier.getCopiedModel();

    // Remove vertices (and attached faces) that connect to 2 or fewer polygons so the boundary is clean.
    ObjModelCleaner cleaner( cmodel);
    cleaner.remove3D();
    cleaner.remove1D();
    int pruned = 0;
    int totPruned = 0;
    do
    {
        pruned = cleaner.pruneVertices(2);
        totPruned += pruned;
    } while ( pruned > 0);

    return getComponent( cmodel, *cmodel->getVertexIds().begin());
}   // end crop


bool FaceTools::hasReqLandmarks( const ObjMetaData::Ptr omd)
{
    return omd &&
           omd->hasLandmark( FaceTools::Landmarks::L_EYE_CENTRE) &&
           omd->hasLandmark( FaceTools::Landmarks::R_EYE_CENTRE) &&
           omd->hasLandmark( FaceTools::Landmarks::NASAL_TIP);
}   // end hasReqLandmarks


cv::Vec3f FaceTools::calcFaceCentre( const ObjMetaData::Ptr omd)
{
    assert(omd);
    if ( !hasReqLandmarks(omd))
    {
        assert(false);
        return cv::Vec3f(0,0,0);
    }   // end if

    cv::Vec3f nvec, uvec;
    if ( !omd->getOrientation( nvec, uvec))
    {
        assert(false);
        return cv::Vec3f(0,0,0);
    }   // end if

    const cv::Vec3f& v0 = omd->getLandmark( FaceTools::Landmarks::L_EYE_CENTRE);
    const cv::Vec3f& v1 = omd->getLandmark( FaceTools::Landmarks::R_EYE_CENTRE);
    const cv::Vec3f& nt = omd->getLandmark( FaceTools::Landmarks::NASAL_TIP);

    const cv::Vec3f midEye = (v0 + v1) * 0.5;
    const cv::Vec3f dvec = nt - midEye;

    cv::Vec3f downVec;
    cv::normalize( -uvec, downVec);
    return midEye + (downVec.dot(dvec) * downVec);
}   // end calcFaceCentre


double FaceTools::calcFaceCropRadius( const ObjMetaData::Ptr omd, double G)
{
    assert(omd);
    if ( !hasReqLandmarks(omd))
        return -1;

    const cv::Vec3f& v0 = omd->getLandmark( FaceTools::Landmarks::L_EYE_CENTRE);
    const cv::Vec3f& v1 = omd->getLandmark( FaceTools::Landmarks::R_EYE_CENTRE);
    const cv::Vec3f fcentre = calcFaceCentre( omd);
    return G * (cv::norm( fcentre - v0) + cv::norm( fcentre - v1))/2;
}   // end calcFaceCropRadius


bool FaceTools::transformToOrigin( ObjMetaData::Ptr omd, const cv::Vec3f* t)
{
    cv::Vec3f fc;
    if ( t)
        fc = *t;
    else
    {
        if ( hasReqLandmarks( omd))
            fc = calcFaceCentre( omd);
        else
            return false;
    }   // end else

    cv::Vec3f nvec, uvec;
    if ( !omd->getOrientation( nvec, uvec))
        return false;

    fc = -fc;   // Subtract to origin

    // Get complimentary axes coordinates for the normal and up vectors
    nvec[0] = -nvec[0];
    nvec[1] = -nvec[1];
    uvec[0] = -uvec[0];
    uvec[2] = -uvec[2];

    RFeatures::ObjModelMover mover( nvec, uvec);
    mover.prependTranslation( fc);
    const cv::Matx44d tmat = mover();
    omd->transform( tmat);

    return true;
}   // end transformToOrigin


ObjModel::Ptr FaceTools::createFromVertices( const cv::Mat_<cv::Vec3f>& vrow)
{
    ObjModel::Ptr omod = ObjModel::create();
    const int npoints = vrow.cols;
    const cv::Vec3f* vptr = vrow.ptr<cv::Vec3f>(0);
    for ( int i = 0; i < npoints; ++i)
        omod->addVertex( vptr[i]);
    return omod;
}   // end createFromVertices


ObjModel::Ptr FaceTools::createFromSubset( const ObjModel::Ptr smod, const IntSet& vidxs)
{
    assert(smod);
    ObjModel::Ptr omod = ObjModel::create( smod->getSpatialPrecision());
    BOOST_FOREACH ( int vidx, vidxs)
        omod->addVertex( smod->vtx(vidx));
    return omod;
}   // end createFromSubset


ObjModel::Ptr FaceTools::createFromTransformedSubset( const ObjModel::Ptr smod, const IntSet& vidxs, const cv::Matx44d& T,
                                                      boost::unordered_map<int,int>* newVidxsToSource)
{
    assert(smod);
    const RFeatures::ObjModelMover transformer(T);
    ObjModel::Ptr omod = ObjModel::create( smod->getSpatialPrecision());
    BOOST_FOREACH ( int vidx, vidxs)
    {
        cv::Vec3f v = smod->vtx(vidx);
        transformer( v);
        const int nvidx = omod->addVertex(v);
        if ( newVidxsToSource)
            (*newVidxsToSource)[nvidx] = vidx;
    }   // end foreach
    return omod;
}   // end createFromTransformedSubset


// Flatten m to XY plane and return it, also setting fmap to be the
// vertex ID mapping from the returned flattened object to the original object m.
ObjModel::Ptr FaceTools::makeFlattened( const ObjModel::Ptr m, boost::unordered_map<int,int>* fmap)
{
    assert(m);
    if ( fmap)
        fmap->clear();
    const IntSet& vidxs = m->getVertexIds();
    int nvidx;
    ObjModel::Ptr fmod = ObjModel::create( m->getSpatialPrecision());
    BOOST_FOREACH ( int vidx, vidxs)
    {
        cv::Vec3f v = m->vtx(vidx);
        v[2] = 0;
        nvidx = fmod->addVertex(v);
        if ( fmap)
            (*fmap)[nvidx] = vidx;
    }   // end foreach
    // fmod is now a flattened version
    return fmod;
}   // end makeFlattened


void FaceTools::clean( ObjModel::Ptr model)
{
    assert(model);
    RFeatures::ObjModelCleaner omc(model);
    const int rem3d = omc.remove3D();
    const int rem1d = omc.remove1D();
    if ( rem3d > 0 || rem1d > 0)
        std::cerr << "[INFO] FaceTools::clean: Removed " << rem3d << " 3D and " << rem1d << " 1D vertices" << std::endl;
    int totRemTV = 0;
    int remTV = 0;
    do
    {
        remTV = RFeatures::ObjModelTetrahedronReplacer( model).removeTetrahedrons();
        totRemTV += remTV;
    } while ( remTV > 0);
    if ( totRemTV > 0)
        std::cerr << "[INFO] FaceTools::clean: Removed/replaced " << totRemTV << " tetrahedron peaks" << std::endl;
}   // end clean


ObjModel::Ptr FaceTools::loadModel( const std::string& fname, bool useTexture, bool doClean)
{
    RModelIO::AssetImporter assetImporter( useTexture);  // Load textures if selected
    const std::string fext = getExtension(fname);
    // If the file extension is not supported, return NULL.
    if ( assetImporter.getAvailable().count(fext) == 0)
        return ObjModel::Ptr();

    if ( !assetImporter.enableFormat(fext))
    {
        std::cerr << "[ERROR] FaceTools::loadModel: Error enabling RModelIO::AssetImporter format!" << std::endl;
        return ObjModel::Ptr();
    }   // end if

    std::cerr << " =====[ Loading Model '" << fname << "' ]=====" << std::endl;
    ObjModel::Ptr model = assetImporter.load( fname);
    if ( !model)
        std::cerr << "Unable to read in object from '" << fname << "'!" << std::endl;
    else
    {
        // Merge materials if more than one - don't want multiple textures (or meshes)
        if ( model->getNumMaterials() > 1)
        {
            std::cerr << " =====[ Combining Textures ]====="<< std::endl;
            model->mergeMaterials();
            assert( model->getNumMaterials() == 1);
        }   // end if

        if ( doClean)
        {
            RFeatures::ObjModelIntegrityChecker ic( model);
            ic.checkIntegrity();
            if ( !ic.is2DManifold())
            {
                std::cerr << " =====[ Cleaning Model ]====="<< std::endl;
                FaceTools::clean(model);
                ic.checkIntegrity();
                assert( ic.is2DManifold());
                if ( !ic.is2DManifold())
                {
                    std::cerr << "[ERROR] FaceTools::loadModel: Unable to make 2D triangulated mesh from '" << fname << "'" << std::endl;
                    model.reset();
                }   // end if
            }   // end if
        }   // end if
    }   // end if

    return model;
}   // end loadModel


bool FaceTools::loadModels( const std::vector<std::string>& fnames, std::vector<RFeatures::ObjModel::Ptr>& models, bool loadTexture, bool doClean)
{
    bool failed = false;
    const int n = (int)fnames.size();
    models.resize(n);
    for ( int i = 0; i < n; ++i)
    {
        models[i] = loadModel( fnames[i], loadTexture, doClean);
        if ( !models[i])
        {
            failed = true;
            break;
        }   // end if
    }   // end for

    return !failed;
}   // end loadModels
