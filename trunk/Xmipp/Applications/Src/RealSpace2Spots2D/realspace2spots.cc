/***************************************************************************
 *
 * Authors:     Roberto Marabini (roberto@mipg.upenn.edu)
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

#include <XmippData/xmippArgs.hh>
#include <Reconstruction/Programs/Prog_Spots2RealSpace2D.hh>

void Usage(char *argv[]);

int main (int argc, char **argv) {
   FileName          	       fn_prm;
   RealSpace2Spots2D_Parameters prm;
   Projection                  prj;

   // Get command line parameters ------------------------------------------
   try {
      fn_prm = get_param(argc,argv,"-i");
      prm.read_from_file(fn_prm);
   } catch (Xmipp_error XE) {Usage(argv);}

   // Main program ---------------------------------------------------------
   try {
      ROUT_RealSpace2Spots(prm,prj);
   } catch (Xmipp_error XE) {cout<<XE;}
   exit(0);
}

void Usage (char *argv[]) {
    cout << "Purpose:\n"
         << "    Perform a DISCRETE but not FAST Fourier Transform\n"
         << "    The output is an aph (MRC) file\n"
    
         << "Usage:" << argv[0] <<" -i parameterfile" << endl << endl
         << "\t-i           : Parameters file" << endl
    ;    
    exit(1);

}
