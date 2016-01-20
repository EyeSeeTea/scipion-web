/***************************************************************************
 * Authors:     AUTHOR_NAME (jvargas@cnb.csic.es)
 *
 *
 * Unidad de  Bioinformatica of Centro Nacional de Biotecnologia , CSIC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.csic.es'
 ***************************************************************************/

#include "angular_accuracy_pca.h"
#include <math.h>

ProgAngularAccuracyPCA::ProgAngularAccuracyPCA()
{
	rank=0;
	Nprocessors=1;
}

void ProgAngularAccuracyPCA::readParams()
{
	fnPhantom = getParam("-i");
	fnNeighbours = getParam("--i2");
    fnOut = getParam("-o");
	fnOutQ = fnOut.getDir()+"/validationAlignabilityAccuracy.xmd";
    newXdim = getIntParam("--dim");
    newYdim = newXdim;

	std::cout <<  newXdim << std::endl;

}

void ProgAngularAccuracyPCA::defineParams()
{
    addUsageLine("Determine the angular determination accuracy of a set of particles and a 3D reconstruction ");
    addParamsLine("  [ -i <volume_file> ]      	: Voxel volume");
    addParamsLine("  [--i2 <md_file=\"\">]    	: Metadata file with neighbour projections");
    addParamsLine("  [ -o <md_file=\"\">]    	: Metadata file with obtained weights");
    addParamsLine("  [--dim <d=64>]             : Scale images to this size if they are larger.");
    addParamsLine("                             : Set to -1 for no rescaling");
}

void ProgAngularAccuracyPCA::run()
{
	MetaData md;
    StringVector blocks;
    getBlocksInMetaDataFile(fnNeighbours, blocks);

    phantomVol.read(fnPhantom);
    phantomVol().setXmippOrigin();

    size_t numPCAs;

	if (rank==0)
		init_progress_bar(blocks.size());

    for (size_t i = 0; i < blocks.size(); ++i)
    {
    	if ((i+1)%Nprocessors==rank)
    	{
    		md.read((String) blocks[i].c_str()+'@'+fnNeighbours);

    		if (md.size() <= 1)
    			continue;

    		else if ( (md.size() > 20) )
    			numPCAs = 3;
    		else if ( (md.size() >= 5) & (md.size() < 20) )
    			numPCAs = 2;
    		else
    			numPCAs = 1;

    		obtainPCAs(md,numPCAs);

    		MDRow row;
    		FOR_ALL_OBJECTS_IN_METADATA(md)
    		{
    			md.getRow(row,__iter.objId);
    			mdPartial.addRow(row);
    		}

			if (rank==0)
				progress_bar(i+1);
    	}
    }

	synchronize();
	gatherResults();

	if (rank == 0)
	{
		double pcaResidualProj,pcaResidualExp,Zscore,temp,qResidualProj,qResidualExp,qZscore;

		qResidualProj = 0;
		qResidualExp = 0;
		qZscore = 0;

		String expression;
		size_t maxIdx;
		MetaData MDSort, tempMd,MDOut, MDOutQ;
		MDSort.sort(mdPartial,MDL_ITEM_ID,true,-1,0);
		MDSort.getValue(MDL_ITEM_ID,maxIdx,MDSort.lastObject());

		std::cout << " maxIdx : " << maxIdx << std::endl;

		MDRow row;

		for (size_t i=0; i<=maxIdx;i++)
		{
			expression = formatString("itemId == %lu",i);
			tempMd.importObjects(MDOut, MDExpression(expression));

			if ( (tempMd.size() <= 0))
				continue;

			pcaResidualProj = 0;
			pcaResidualExp = 0;
			Zscore = 0;

			tempMd.getRow(row,tempMd.firstObject());
			FOR_ALL_OBJECTS_IN_METADATA(tempMd)
			{
				tempMd.getValue(MDL_SCORE_BY_PCA_RESIDUAL_PROJ,temp,__iter.objId);
				pcaResidualProj +=temp;
				tempMd.getValue(MDL_SCORE_BY_PCA_RESIDUAL_EXP,temp,__iter.objId);
				pcaResidualExp +=temp;
				tempMd.getValue(MDL_SCORE_BY_ZSCORE,temp,__iter.objId);
				Zscore += temp;

			}

			pcaResidualProj /= tempMd.size();
			pcaResidualExp /= tempMd.size();
			Zscore /= tempMd.size();

			qResidualProj += pcaResidualProj;
			qResidualExp  += pcaResidualExp;
			qZscore       += Zscore;

			row.setValue(MDL_SCORE_BY_PCA_RESIDUAL_PROJ,pcaResidualProj);
			row.setValue(MDL_SCORE_BY_PCA_RESIDUAL_EXP,pcaResidualExp);
			row.setValue(MDL_SCORE_BY_ZSCORE,Zscore);
			MDOut.addRow(row);
			row.clear();
		}

		MDOut.write(fnOut);

		qResidualProj /= MDOut.size();
		qResidualExp /= MDOut.size();
		qZscore /= MDOut.size();

	    row.setValue(MDL_IMAGE,fnPhantom);
	    row.setValue(MDL_SCORE_BY_PCA_RESIDUAL_PROJ,qResidualProj);
	    row.setValue(MDL_SCORE_BY_PCA_RESIDUAL_EXP,qResidualExp);
	    row.setValue(MDL_SCORE_BY_ZSCORE,qZscore);

	    MDOutQ.addRow(row);
	    MDOutQ.write(fnOutQ);
		progress_bar(blocks.size());
	}

}

void ProgAngularAccuracyPCA::obtainPCAs(MetaData &SF, size_t numPCAs)
{
	size_t numIter = 200;
	pca.clear();
	size_t imgno;
	Image<double> img;
	double rot, tilt, psi;
	bool mirror;
	size_t  Xdim, Ydim, Zdim, Ndim;
	phantomVol().getDimensions(Xdim,Ydim,Zdim,Ndim);

	if (  (newXdim == -1) )
	{
		newXdim = Xdim;
		newXdim = Ydim;
	}

	Matrix2D<double> proj, projRef;
	imgno = 0;
	Projection P;
	FileName image;
	MultidimArray<float> temp;
	Matrix2D<double> E;

	FOR_ALL_OBJECTS_IN_METADATA(SF)
	{
		int enabled;
		SF.getValue(MDL_ENABLED,enabled,__iter.objId);
		if ( (enabled==-1)  )
		{
			imgno++;
			continue;
		}

		SF.getValue(MDL_ANGLE_ROT,rot,__iter.objId);
		SF.getValue(MDL_ANGLE_TILT,tilt,__iter.objId);
		SF.getValue(MDL_ANGLE_PSI,psi,__iter.objId);
		SF.getValue(MDL_FLIP,mirror,__iter.objId);

		if (mirror)
		{
			double newrot;
			double newtilt;
			double newpsi;
			Euler_mirrorY(rot,tilt,psi,newrot,newtilt,newpsi);
			rot = newrot;
			tilt = newtilt;
			psi = newpsi;
		}

		projectVolume(phantomVol(), P, Ydim, Xdim, rot, tilt, psi);

		Euler_angles2matrix(rot, tilt, psi, E, false);
		double angle = atan2(MAT_ELEM(E,0,1),MAT_ELEM(E,0,0));
		selfRotate(LINEAR, P(),-(angle*180)/3.14159 , WRAP);
		typeCast(P(), temp);
		selfScaleToSize(LINEAR,temp,newXdim,newYdim,1);
		temp.resize(newXdim*newYdim);
		temp.statisticsAdjust(0,1);
		pca.addVector(temp);
		imgno++;

		#ifdef DEBUG
		{
			{
				size_t val;
				SF.getValue(MDL_ITEM_ID,val,__iter.objId);
				{
					std::cout << E << std::endl;
					std::cout << (angle*180)/3.14159 << std::endl;
					P.write("kk_proj.tif");
					SF.getValue(MDL_ANGLE_PSI,psi,__iter.objId);
					std::cout << rot << " " << tilt << " " << psi << std::endl;
					char c;
					std::getchar();
				}
			}
		}
		#endif
	}

	pca.subtractAvg();
	pca.learnPCABasis(numPCAs,numIter);
	pca.projectOnPCABasis(projRef);
	pca.v.clear();

	imgno = 0;
	FileName f;

	FOR_ALL_OBJECTS_IN_METADATA(SF)
	{

		int enabled;
		SF.getValue(MDL_ENABLED,enabled,__iter.objId);

		if ( (enabled==-1)  )
		{
			imgno++;
			continue;
		}

		ApplyGeoParams params;
		params.only_apply_shifts = true;
		img.readApplyGeo(SF,__iter.objId,params);
		Matrix2D<double> E;
		SF.getValue(MDL_ANGLE_ROT,rot,__iter.objId);
		SF.getValue(MDL_ANGLE_TILT,tilt,__iter.objId);
		SF.getValue(MDL_ANGLE_PSI,psi,__iter.objId);

		if (mirror)
		{
			double newrot;
			double newtilt;
			double newpsi;
			Euler_mirrorY(rot,tilt,psi,newrot,newtilt,newpsi);
			rot = newrot;
			tilt = newtilt;
			psi = newpsi;
		}

		Euler_angles2matrix(rot, tilt, psi, E, false);
		double angle = atan2(MAT_ELEM(E,0,1),MAT_ELEM(E,0,0));
		selfRotate(LINEAR, img(),-(angle*180)/3.14159 , WRAP);

#ifdef DEBUG
		{
			size_t val;
			SF.getValue(MDL_ITEM_ID,val,__iter.objId);
			if (val==151)
			{
				std::cout << E << std::endl;
				std::cout << (angle*180)/3.14159 << std::endl;
				std::cout << rot << " " << tilt << " " << psi << std::endl;
				char c;
				std::getchar();
				SF.getValue(MDL_IMAGE,f,__iter.objId);
				std::cout << f << std::endl;
				img.write("kk_exp.tif");
			}
		}
#endif

		typeCast(img(), temp);
		selfScaleToSize(LINEAR,temp,newXdim,newYdim,1);
		temp.resize(newXdim*newYdim);
		temp.statisticsAdjust(0,1);
		pca.addVector(temp);
		imgno++;
	}

	pca.subtractAvg();
	pca.projectOnPCABasis(proj);

	std::vector< MultidimArray<float> > recons(SF.size());
	for(int n=0; n<SF.size(); n++)
		recons[n] = MultidimArray<float>(newXdim*newYdim);

	pca.reconsFromPCA(proj,recons);
	pca.evaluateZScore(numPCAs,numIter, false);

	imgno = 0;
	Image<float> imgRes;
	double R2_Proj,R2_Exp;
	R2_Proj=0;
	R2_Exp=0;

	FOR_ALL_OBJECTS_IN_METADATA(SF)
	{
		int enabled;
		SF.getValue(MDL_ENABLED,enabled,__iter.objId);
		if ( (enabled==-1)  )
		{
			imgno++;
			continue;
		}

		//Projected Image
		SF.getValue(MDL_ANGLE_ROT,rot,__iter.objId);
		SF.getValue(MDL_ANGLE_TILT,tilt,__iter.objId);
		SF.getValue(MDL_ANGLE_PSI,psi,__iter.objId);
		SF.getValue(MDL_FLIP,mirror,__iter.objId);

		if (mirror)
		{
			double newrot;
			double newtilt;
			double newpsi;
			Euler_mirrorY(rot,tilt,psi,newrot,newtilt,newpsi);
			rot = newrot;
			tilt = newtilt;
			psi = newpsi;
		}

		projectVolume(phantomVol(), P, Ydim, Xdim, rot, tilt, psi);
		Euler_angles2matrix(rot, tilt, psi, E, false);
		double angle = atan2(MAT_ELEM(E,0,1),MAT_ELEM(E,0,0));
		selfRotate(LINEAR, P(),-(angle*180)/3.14159 , WRAP);
		typeCast(P(), temp);
		selfScaleToSize(LINEAR,temp,newXdim,newYdim,1);
		temp.resize(newXdim*newYdim);
		temp.statisticsAdjust(0,1);
		temp.setXmippOrigin();

		//Reconstructed Image
		recons[imgno].statisticsAdjust(0,1);
		recons[imgno].resize(newYdim*newXdim);
		recons[imgno].setXmippOrigin();

		imgRes() = temp - (recons[imgno]);//-temp;
		R2_Proj = imgRes().computeStddev();
		R2_Proj = R2_Proj*R2_Proj;

//		if (R2_Proj < 0)
//			R2_Proj = 0;

		ApplyGeoParams params;
		params.only_apply_shifts = true;
		img.readApplyGeo(SF,__iter.objId,params);
		Matrix2D<double> E;
		SF.getValue(MDL_ANGLE_ROT,rot,__iter.objId);
		SF.getValue(MDL_ANGLE_TILT,tilt,__iter.objId);
		SF.getValue(MDL_ANGLE_PSI,psi,__iter.objId);

		if (mirror)
		{
			double newrot;
			double newtilt;
			double newpsi;
			Euler_mirrorY(rot,tilt,psi,newrot,newtilt,newpsi);
			rot = newrot;
			tilt = newtilt;
			psi = newpsi;
		}

		Euler_angles2matrix(rot, tilt, psi, E, false);
		angle = atan2(MAT_ELEM(E,0,1),MAT_ELEM(E,0,0));
		selfRotate(LINEAR, img(),-(angle*180)/3.14159 , WRAP);
		typeCast(img(), temp);
		selfScaleToSize(LINEAR,temp,newXdim,newYdim,1);
		temp.resize(newXdim*newYdim);
		temp.statisticsAdjust(0,1);
		temp.setXmippOrigin();

		imgRes() = temp - (recons[imgno]);
		R2_Exp = imgRes().computeStddev();
		R2_Exp = R2_Exp*R2_Exp;

//		if (R2_Exp < 0)
//			R2_Exp = 0;

		SF.setValue(MDL_SCORE_BY_PCA_RESIDUAL_PROJ,exp(-R2_Proj),__iter.objId);
		SF.setValue(MDL_SCORE_BY_PCA_RESIDUAL_EXP,exp(-R2_Exp),__iter.objId);
		SF.setValue(MDL_SCORE_BY_ZSCORE, exp(-A1D_ELEM(pca.Zscore,imgno)/3.),__iter.objId);


#ifdef DEBUG
		{
			Image<float> imgRecons;
			SF.getValue(MDL_IMAGE,f,__iter.objId);

			img().statisticsAdjust(0,1);
			img.write("kk_exp.tif");

			if ( imgno == 0)
			{
				for(int n=0; n<SF.size(); n++)
					recons[n] = MultidimArray<float>(newXdim*newYdim);

				pca.reconsFromPCA(proj,recons);
			}

			recons[imgno].statisticsAdjust(0,1);
			imgRecons()=recons[imgno];
			imgRecons().resize(newYdim,newXdim);
			imgRecons.write("kk_reconstructed.tif");

			std::cout <<  std::endl;

			for(int i=0; i<numPCAs;i++)
			{
				std::cout << "proj " << MAT_ELEM(proj,i,imgno)/normProj <<  "   " <<  "projRef " <<  MAT_ELEM(projRef,i,imgno)/normProjRef << std::endl;

			}

			std::cout <<  std::endl;
			std::cout << " ratioSignalRepresented : " << ratioSignalRepresented << std::endl;
			char c;
			std::getchar();

			//recons[imgno].clear();
		}

#endif

		imgno++;

	}

	recons.clear();
	img.clear();
	temp.clear();
	imgRes.clear();
	pca.clear();

}
