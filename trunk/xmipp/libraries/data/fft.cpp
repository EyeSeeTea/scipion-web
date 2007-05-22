/***************************************************************************
 *
 * Authors:     Carlos Oscar S. Sorzano (coss@cnb.uam.es)
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
 *  e-mail address 'xmipp@cnb.uam.es'
 ***************************************************************************/

#include "fft.h"
#include "args.h"
#include "external/bilib/headers/dft.h"

/* Format conversions ------------------------------------------------------ */
/** Convert whole -> half of (centro-symmetric) Fourier transforms 2D. -- */
void Whole2Half(const matrix2D<complex<double> > &in, matrix2D<complex<double> > &out)
{

    // This assumes squared images...
    int ldim = (int)(YSIZE(in) / 2) + 1;

    out.init_zeros(ldim, XSIZE(in));
    // Fill first column only half
    for (int j = 0; j < ldim; j++)
        dMij(out, 0, j) = dMij(in, 0, j);
    // Fill rest
    for (int i = 1; i < ldim; i++)
        for (int j = 0; j < XSIZE(in); j++)
            dMij(out, i, j) = dMij(in, i, j);

}

/** Convert half -> whole of (centro-symmetric) Fourier transforms 2D. -- */
void Half2Whole(const matrix2D<complex<double> > &in, matrix2D<complex<double> > &out, int oriydim)
{

    out.resize(oriydim, XSIZE(in));

    // Old part
    for (int i = 0; i < YSIZE(in); i++)
        for (int j = 0; j < XSIZE(in); j++)
            dMij(out, i, j) = dMij(in, i, j);
    // Complete first column of old part
    for (int j = YSIZE(in); j < XSIZE(in); j++)
        dMij(out, 0, j) = conj(dMij(in, 0, XSIZE(in) - j));
    // New part
    for (int i = YSIZE(in); i < oriydim; i++)
    {
        dMij(out, i, 0) = conj(dMij(in, oriydim - i, 0));
        for (int j = 1; j < XSIZE(in); j++)
            dMij(out, i, j) = conj(dMij(in, oriydim - i, XSIZE(in) - j));
    }
}


/** Convert complex -> real,imag Fourier transforms 3D. -- */
void Complex2RealImag(const matrix3D< complex < double > > & in,
                      matrix3D< double > & real,
                      matrix3D< double > & imag)
{

    real.resize(in);
    imag.resize(in);
    Complex2RealImag(MULTIDIM_ARRAY(in), MULTIDIM_ARRAY(real),
                     MULTIDIM_ARRAY(imag), XSIZE(in)*YSIZE(in)*ZSIZE(in));

}

/** Convert real,imag -> complex Fourier transforms 3D. -- */
void RealImag2Complex(const matrix3D< double > & real,
                      const matrix3D< double > & imag,
                      matrix3D< complex < double > > & out)
{
    out.resize(real);
    RealImag2Complex(MULTIDIM_ARRAY(real), MULTIDIM_ARRAY(imag),
                     MULTIDIM_ARRAY(out), XSIZE(real)*YSIZE(real)*ZSIZE(real));

}

/** Direct Fourier Transform 1D ------------------------------------------- */
void FourierTransform(const matrix1D<double> &in,
                      matrix1D< complex<double> > &out)
{
    int N = XSIZE(in);
    matrix1D<double> re(in), tmp(N), im(N), cas(N);
    out.resize(N);

    GetCaS(MULTIDIM_ARRAY(cas), N);
    DftRealToRealImaginary(MULTIDIM_ARRAY(re), MULTIDIM_ARRAY(im),
                           MULTIDIM_ARRAY(tmp), MULTIDIM_ARRAY(cas), N);
    RealImag2Complex(MULTIDIM_ARRAY(re), MULTIDIM_ARRAY(im),
                     MULTIDIM_ARRAY(out), N);
}

/** Direct Fourier Transform 2D. ------------------------------------------ */
void FourierTransform(const matrix2D<double> &in,
                      matrix2D< complex<double> > &out)
{
    int Status;
    matrix2D<double> re(in), im;
    im.resize(in);
    out.resize(in);
    VolumeDftRealToRealImaginary(MULTIDIM_ARRAY(re),
                                 MULTIDIM_ARRAY(im), XSIZE(in), YSIZE(in), 1, &Status);
    RealImag2Complex(MULTIDIM_ARRAY(re), MULTIDIM_ARRAY(im),
                     MULTIDIM_ARRAY(out), XSIZE(in)*YSIZE(in));
}

/** Direct Fourier Transform 3D. ------------------------------------------ */
void FourierTransform(const matrix3D<double> &in,
                      matrix3D< complex<double> > &out)
{
    int Status;
    matrix3D<double> re(in), im;
    im.resize(in);
    out.resize(in);
    VolumeDftRealToRealImaginary(MULTIDIM_ARRAY(re),
                                 MULTIDIM_ARRAY(im), XSIZE(in), YSIZE(in), ZSIZE(in), &Status);
    RealImag2Complex(MULTIDIM_ARRAY(re), MULTIDIM_ARRAY(im),
                     MULTIDIM_ARRAY(out), XSIZE(in)*YSIZE(in)*ZSIZE(in));
}

/** Inverse Fourier Transform 1D. ----------------------------------------- */
void InverseFourierTransform(const matrix1D< complex<double> > &in,
                             matrix1D<double> &out)
{
    int N = XSIZE(in);
    matrix1D<double> tmp(N), im(N), cas(N);
    out.resize(N);

    GetCaS(MULTIDIM_ARRAY(cas), N);
    Complex2RealImag(MULTIDIM_ARRAY(in), MULTIDIM_ARRAY(out),
                     MULTIDIM_ARRAY(im), N);
    InvDftRealImaginaryToReal(MULTIDIM_ARRAY(out), MULTIDIM_ARRAY(im),
                              MULTIDIM_ARRAY(tmp), MULTIDIM_ARRAY(cas), N);
}

/** Inverse Fourier Transform 2D. ----------------------------------------- */
void InverseFourierTransform(const matrix2D< complex<double> > &in,
                             matrix2D<double> &out)
{
    int Status;
    matrix2D<double> im;
    out.resize(in);
    im.resize(in);
    Complex2RealImag(MULTIDIM_ARRAY(in), MULTIDIM_ARRAY(out),
                     MULTIDIM_ARRAY(im), XSIZE(in)*YSIZE(in));
    VolumeInvDftRealImaginaryToReal(MULTIDIM_ARRAY(out),
                                    MULTIDIM_ARRAY(im), XSIZE(in), YSIZE(in), 1, &Status);
}

/** Inverse Fourier Transform 3D. ----------------------------------------- */
void InverseFourierTransform(const matrix3D< complex<double> > &in,
                             matrix3D<double> &out)
{
    int Status;
    matrix3D<double> im;
    out.resize(in);
    im.resize(in);
    Complex2RealImag(MULTIDIM_ARRAY(in), MULTIDIM_ARRAY(out),
                     MULTIDIM_ARRAY(im), XSIZE(in)*YSIZE(in)*ZSIZE(in));
    VolumeInvDftRealImaginaryToReal(MULTIDIM_ARRAY(out),
                                    MULTIDIM_ARRAY(im), XSIZE(in), YSIZE(in), ZSIZE(in), &Status);
}


/** Direct Fourier Transform 2D, output half of (centro-symmetric) transform ---- */
void FourierTransformHalf(const matrix2D<double> &in,
                          matrix2D< complex<double> > &out)
{

    matrix2D<complex <double> > aux;
    FourierTransform(in, aux);
    Whole2Half(aux, out);

}

/** Inverse Fourier Transform 2D, input half of (centro-symmetric) transform ---- */
void InverseFourierTransformHalf(const matrix2D< complex<double> > &in,
                                 matrix2D<double> &out, int oriydim)
{

    matrix2D< complex<double> > aux;
    Half2Whole(in, aux, oriydim);
    InverseFourierTransform(aux, out);
    out.set_Xmipp_origin();
}

/* FFT shifts ------------------------------------------------------------ */
void ShiftFFT(matrix1D< complex< double > > & v,
              double xshift)
{
    double dotp, a, b, c, d, ac, bd, ab_cd;
    double xxshift = xshift / (double)XSIZE(v);
    for (int j = 0; j < XSIZE(v); j++)
    {
        dotp = -2 * PI * ((double)(j) * xxshift);
        a = cos(dotp);
        b = sin(dotp);
        c = (v).data[j].real();
        d = (v).data[j].imag();
        ac = a * c;
        bd = b * d;
        ab_cd = (a + b) * (c + d); // (ab_cd-ac-bd = ad+bc : but needs 4 multiplications)
        (v).data[j] = complex<double>(ac - bd, ab_cd - ac - bd);
    }
}

void ShiftFFT(matrix2D< complex< double > > & v,
              double xshift, double yshift)
{
    double dotp, a, b, c, d, ac, bd, ab_cd;
    double xxshift = xshift / (double)XSIZE(v);
    double yyshift = yshift / (double)YSIZE(v);
    for (int i = 0; i < YSIZE(v); i++)
    {
        for (int j = 0; j < XSIZE(v); j++)
        {
            dotp = -2 * PI * ((double)(j) * xxshift + (double)(i) * yyshift);
            a = cos(dotp);
            b = sin(dotp);
            c = (v).data[(i)*XSIZE(v)+(j)].real();
            d = (v).data[(i)*XSIZE(v)+(j)].imag();
            ac = a * c;
            bd = b * d;
            ab_cd = (a + b) * (c + d);
            (v).data[(i)*XSIZE(v)+(j)] = complex<double>(ac - bd, ab_cd - ac - bd);
        }
    }
}

void ShiftFFT(matrix3D< complex< double > > & v,
              double xshift, double yshift, double zshift)
{
    double dotp, a, b, c, d, ac, bd, ab_cd;
    double xxshift = xshift / (double)XSIZE(v);
    double yyshift = yshift / (double)YSIZE(v);
    double zzshift = zshift / (double)ZSIZE(v);
    for (int k = 0; k < ZSIZE(v); k++)
    {
        for (int i = 0; i < YSIZE(v); i++)
        {
            for (int j = 0; j < XSIZE(v); j++)
            {
                dotp = -2 * PI * ((double)(j) * xxshift + (double)(i) * yyshift + (double)(k) * zzshift);
                a = cos(dotp);
                b = sin(dotp);
                c = (v).data[(k)*XYSIZE(v)+(i)*XSIZE(v)+(j)].real();
                d = (v).data[(k)*XYSIZE(v)+(i)*XSIZE(v)+(j)].imag();
                ac = a * c;
                bd = b * d;
                ab_cd = (a + b) * (c + d);
                (v).data[(k)*XYSIZE(v)+(i)*XSIZE(v)+(j)] = complex<double>(ac - bd, ab_cd - ac - bd);
            }
        }
    }
}

/* Position origin at center ----------------------------------------------- */
void CenterOriginFFT(matrix1D< complex< double > > & v, bool forward)
{
    double xshift = -(double)(int)(XSIZE(v) / 2);
    if (forward)
    {
        ShiftFFT(v, xshift);
        CenterFFT(v, forward);
    }
    else
    {
        CenterFFT(v, forward);
        ShiftFFT(v, -xshift);
    }
}

void CenterOriginFFT(matrix2D< complex< double > > & v, bool forward)
{
    double xshift = -(double)(int)(XSIZE(v) / 2);
    double yshift = -(double)(int)(YSIZE(v) / 2);
    if (forward)
    {
        ShiftFFT(v, xshift, yshift);
        CenterFFT(v, forward);
    }
    else
    {
        CenterFFT(v, forward);
        ShiftFFT(v, -xshift, -yshift);
    }
}

void CenterOriginFFT(matrix3D< complex< double > > & v, bool forward)
{
    double xshift = -(double)(int)(XSIZE(v) / 2);
    double yshift = -(double)(int)(YSIZE(v) / 2);
    double zshift = -(double)(int)(ZSIZE(v) / 2);
    if (forward)
    {
        ShiftFFT(v, xshift, yshift, zshift);
        CenterFFT(v, forward);
    }
    else
    {
        CenterFFT(v, forward);
        ShiftFFT(v, -xshift, -yshift, -zshift);
    }
}

/* FFT Magnitude 1D. ------------------------------------------------------- */
void FFT_magnitude(const matrix1D< complex<double> > &v,
                   matrix1D<double> &mag)
{
    mag.resize(v);
    FOR_ALL_ELEMENTS_IN_MATRIX1D(v) mag(i) = abs(v(i));
}

void FFT_magnitude(const matrix2D< complex<double> > &v,
                   matrix2D<double> &mag)
{
    mag.resize(v);
    FOR_ALL_ELEMENTS_IN_MATRIX2D(v) mag(i, j) = abs(v(i, j));
}

void FFT_magnitude(const matrix3D< complex<double> > &v,
                   matrix3D<double> &mag)
{
    mag.resize(v);
    FOR_ALL_ELEMENTS_IN_MATRIX3D(v) mag(k, i, j) = abs(v(k, i, j));
}

/* FFT Phase 1D. ------------------------------------------------------- */
void FFT_phase(const matrix1D< complex<double> > &v,
               matrix1D<double> &phase)
{
    phase.resize(v);
    FOR_ALL_ELEMENTS_IN_MATRIX1D(v) phase(i) = atan2(v(i).imag(), v(i).real());
}

void FFT_phase(const matrix2D< complex<double> > &v,
               matrix2D<double> &phase)
{
    phase.resize(v);
    FOR_ALL_ELEMENTS_IN_MATRIX2D(v) phase(i, j) = atan2(v(i, j).imag(), v(i, j).real());
}

void FFT_phase(const matrix3D< complex<double> > &v,
               matrix3D<double> &phase)
{
    phase.resize(v);
    FOR_ALL_ELEMENTS_IN_MATRIX3D(v) phase(k, i, j) = atan2(v(k, i, j).imag(),
            v(k, i, j).real());
}

/****************************************************************************/
/*            Square distance from the point (x,y) to (m/2,m/2)             */
/****************************************************************************/
double distancia2(int x, int y, int m)
{
    double x1, y1;

    x1 = (x - m / 2) * (x - m / 2);
    y1 = (y - m / 2) * (y - m / 2);

    return(x1 + y1);
}

/* SSNR process single image */
void my_ssnr_step(const matrix2D< complex<double> > &FTAverageImage,
                  const matrix2D< complex<double> > &FTaverageSubGroup,
                  matrix1D<double> &ssnr, matrix1D<double> &pixel, int z)
{
    //n -> number of images in the subset
    int top, cont;
    double d, w2, w12;
    double w = 0.0;
    float preal1, preal2, pimag1, pimag2;
    float resta_real, resta_imag;
    float mod2_diferencia, mod2_media;

    //   int dim (int)FTAverageImage.RowNo()/2;
    int n = (int)FTAverageImage.RowNo();
    top = (int)(n / 2);
    FOR_ALL_ELEMENTS_IN_MATRIX1D(ssnr)
    {
        mod2_media = 0.0;
        mod2_diferencia = 0.0;
        w = i + 0.5;
        w2 = w * w;
        w12 = (w + 1) * (w + 1);
        cont = 0;
        for (int ii = 0; ii < n; ii++)
            for (int jj = 0; jj <= top; jj++)
            {
                d = distancia2(ii, jj, n);
                if (d >= w2 && d < w12)
                {
                    cont++;
                    mod2_diferencia += norm(dMij(FTAverageImage, ii, jj) -
                                            dMij(FTaverageSubGroup, ii, jj));
                    mod2_media += norm(dMij(FTAverageImage, ii, jj));
                }
            }
        mod2_diferencia *= z;
        VEC_ELEM(ssnr, i) += mod2_diferencia / mod2_media;
        VEC_ELEM(pixel, i) = cont;
    }
}

/* Numerical derivative of a matrix ----------------------------- */
void numerical_derivative(matrix2D<double> &M, matrix2D<double> &D,
                          char direction, int order, int window_size, int polynomial_order)
{

    // Set D to be a copy in shape of M
    D.copy_shape(M);

    matrix1D<double> v, rotated;
    matrix1D<double> ans; // To obtain results

    // Wrap around version of the Savitzky-Golay coefficients
    int dim = 2 * window_size + 1;
    rotated.resize(dim);

    double *pans = ans.adapt_for_numerical_recipes();
    double *pv = v.adapt_for_numerical_recipes();
    double *protated = rotated.adapt_for_numerical_recipes();

    // Calculate the Savitzky-Golay filter coeficients
    savgol(protated, 2*window_size + 1, window_size,
           window_size, order, polynomial_order);

    // Savitzky-Golay filter is returned in wrap-around style, so
    // correct it to use with the convolution routine
    matrix1D<double> coefficients(dim);
    FOR_ALL_ELEMENTS_IN_MATRIX1D(coefficients)
    {
        int j = i + window_size;
        if (j < dim)
            coefficients(j) = rotated(i);
        else
            coefficients(j - dim) = rotated(i);
    }

    // Apply the Savitzky-Golay filter to every row or column
    if (direction == 'x')
    {
        // For every row (values in a row are values of the X direction)
        for (int i = STARTINGY(M);i <= FINISHINGY(M);i++)
        {
            M.getRow(i, v);
            series_convolution(v, coefficients, ans, false);
            ans.setRow();
            D.setRow(i, ans);
        }
    }
    else if (direction == 'y')
    {
        // For every column (values in a column are values of the Y direction)
        for (int i = STARTINGX(M);i <= FINISHINGX(M);i++)
        {
            M.getCol(i, v);
            series_convolution(v, coefficients, ans, false);
            ans.setCol();
            D.setCol(i, ans);
        }
    }
}
