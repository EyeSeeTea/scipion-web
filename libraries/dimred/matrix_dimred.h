/***************************************************************************
 * Authors:     AUTHOR_NAME Carlos Oscar Sorzano (coss@cnb.csic.es)
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

#ifndef _MATRIX_DIMRED
#define _MATRIX_DIMRED

#include <data/xmipp_program.h>

/**@defgroup ProgMatrixDimRed Project a matrix onto a lower dimensionality space
   @ingroup DimRedLibrary */
//@{
/** Matrix parameters. */
class ProgMatrixDimRed: public XmippProgram
{
public:
    /** Filename selection file containing the images */
    FileName fnIn;
    FileName fnOut;
    String dimRefMethod;
    int inputDim, outputDim;
    int Nsamples;
    int kNN; // Number of nearest neighbors
    int Niter; // Number of iterations
    double t; // Markov random walk
    double sigma; // Sigma of kernel
public:
    Matrix2D<double> X; // Input data
public:
    /// Read argument from command line
    void readParams();

    /// Show
    void show();

    /// Define parameters
    void defineParams();

    /// Produce side info
    void produceSideInfo();

    /// Main routine
    void run();
};
//@}
#endif
