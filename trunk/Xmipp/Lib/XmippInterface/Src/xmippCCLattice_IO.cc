/***************************************************************************
 *
 * Authors:     Debora Gil
 *              Roberto Marabini
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
 *                                      <                               
 * You should have received a copy of the GNU General Public License   
 * along with this program; if not, write to the Free Software         
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA            
 * 02111-1307  USA                                                     
 *                                                                     
 *  All comments concerning this program package may be sent to the    
 *  e-mail address 'xmipp@cnb.uam.es'                                  
 ***************************************************************************/



///////////////////////////// COMMON LIBRARIES /////////////////////////

#include <Reconstruction/symmetries.hh>
#include <XmippData/xmippArgs.hh>
#include <XmippData/xmippImages.hh>
#include <fstream>
#include <iomanip>

//////////////////////////// SPECIFIC LIBRARIES ////////////////////////
#include "../xmippCCLattice_IO.hh"


#define GCC_VERSION (__GNUC__ * 10000 \
   + __GNUC_MINOR__ * 100 \
   + __GNUC_PATCHLEVEL__)
/* Test for GCC > 3.3.0 */
#if GCC_VERSION >= 30300
   #include <sstream>
#else
   #include <strstream.h>
#endif

#define VERBOSE
//#define DEBUG


// Constructor =============================================================
   CCLattice_IO::CCLattice_IO()
   {
    a.resize(2);
    b.resize(2);
  
    cc_max=0.;
    cc_peak_factor=0;
   }
// COR =====================================================================
// Read a cor file generated by the MRC program quadserach.
void CCLattice_IO::read(const FileName &fn) {
  
   ifstream  fh_cor;
   int       line_no=0;
   string    line;
   float Th;
   
      
   // Open file
   fh_cor.open(fn.c_str(), ios::in);
   if (!fh_cor)
      REPORT_ERROR(1601,"CCLattice_IO::read: File "+fn+" not found");
   // Read fith line and skip them
   fh_cor.peek();
   for(int i=1;i<6;i++)
      getline(fh_cor,line);
   // Read Lattice Parameters   
   try {
       getline(fh_cor,line);
      dim[0] =AtoI(first_token(line));
      dim[1] =AtoI(next_token());
      O[0] =AtoD(next_token());
      O[1] =AtoD(next_token());
      a(0) =AtoD(next_token());
      a(1) =AtoD(next_token());
       getline(fh_cor,line);
       b(0) =AtoD(first_token(line));
       b(1) =AtoD(next_token());
       min_i =AtoI(next_token());
       max_i =AtoI(next_token());
       min_j =AtoI(next_token());
       max_j =AtoI(next_token());
       getline(fh_cor,line);
       cc_max=AtoD(first_token(line));
   } catch (...) {
      REPORT_ERROR(1601,"CCLattice_IO::read: Wrong  line in file "+fn);
   }
   //#define DEBUG
   #ifdef DEBUG
   cout << "astar vector              :   " << a  << endl
        << "bstar vector              :   " << b  << endl
        << "Cristal_dim               :   " << dim[0] << " " << dim[1] << endl
        << "Cristal_Search_Origen     :   " << O[0]   << " " <<  O[1]  << endl
        << "min_h, max_h, min_k, max_k:   " << min_i  << " " <<  max_i << " "
                                            << min_j  << " " <<  max_j << endl
        << "Maximum Correlation       :   " << cc_max  <<endl;  			       
     
   cout.flush();
   #endif
   #undef DEBUG
   
   
   
   // Read each line and keep it in the list of the Exp Lattice object
  
   double auxX,auxY,auxCCcoeff;
   //Threshold over CCvalue
   Th=cc_peak_factor* cc_max;
   
   for(int x_index=min_i;x_index<=max_i;x_index++)
      for(int y_index=min_j;y_index<=max_j;y_index++){
	    try {
               getline(fh_cor,line);
	     if (line.length()!=0) {
		 auxX = AtoD(first_token(line));
		 auxY = AtoD(next_token());
		 auxCCcoeff = AtoD(next_token());
        	 //  if(cc_max<auxCCcoeff) cc_max=auxCCcoeff;
		 if(auxX != 0 && auxY != 0 && auxCCcoeff>Th){
		       MRC_Xcoord.push_back(auxX);
		       MRC_Ycoord.push_back(auxY);
		       MRC_CCcoheficiente.push_back(auxCCcoeff);
		       MRC_Xindex.push_back(x_index);
                       MRC_Yindex.push_back(y_index);
		   }
	       }
	    }
	    catch (Xmipp_error) {
               cout << "aph File: Line " << line_no << " is skipped due to an error\n";
	    }
	    line_no++;
	 }/* while */
   // Close file
   fh_cor.close();
   #undef DEBUG
//   #define DEBUG
   #ifdef DEBUG
   cout << "cor_data vector" << endl;
   for (int kk = 0; kk < MRC_Xcoord.size(); kk++)
        cout << MRC_Xindex[kk] << " " 
	     << MRC_Yindex[kk] << " ";

        cout << setprecision(16) << MRC_Xcoord[kk] << " " 
	     << setprecision(16) << MRC_Ycoord[kk] << " "
	     << setprecision(16) << MRC_CCcoheficiente[kk] << endl;   
   #endif
   #undef DEBUG
}/*  CCLattice_IO::read */

/////////////////////////////////////////////////////////////
// Write a cor file generated by the MRC program quadserach.
void CCLattice_IO::write(const FileName &fn) {
  
   ofstream  fh_cor;   
      
   // Open file
   fh_cor.open(fn.c_str(), ios::out);
   fh_cor.precision(5);
   fh_cor <<setiosflags( ios::fixed );
   if (fh_cor.fail())
      REPORT_ERROR(1601,"CCLattice_IO::write: File "+fn+" can not be opened");
   // Write fith dumy lines
   fh_cor << "Cor file generated by" << endl
          <<"xmippCCLattice_IO.cc"  << endl
	  << "--------------------"    << endl
	  << endl << endl;
	  
   // Write Lattice Parameters   
   fh_cor << dim[0]  << " "  << dim[1] << " "
          <<   O[0]  << " "  <<   O[1] << " "  
	  <<   a(0)  << " "  <<   a(1) << endl
          
	  <<   b(0)  << " "  <<   b(1) << " "
          <<   min_i << " "  <<  max_i << " "  
	  <<   min_j << " "  <<  max_j << endl;
   fh_cor.precision(1);
 
   fh_cor << cc_max << endl;   
   
   
   // write each line and keep it in the list of the Exp Lattice object

   for (int kk = 0; kk < MRC_Xcoord.size(); kk++){
      fh_cor << MRC_Xindex[kk] << " " 
	     << MRC_Yindex[kk] << " ";
      fh_cor.precision(3);
      fh_cor << "  " << MRC_Xcoord[kk]  
             << "  " << MRC_Ycoord[kk];
      fh_cor.precision(2);
      fh_cor << "  " << MRC_CCcoheficiente[kk] << endl;
      }
   // Close file
   fh_cor.close();
 }/*  CCLattice_IO::write */

/////////////////////////////////////////////
/*Compute diference between ideal red spots and experimental ones */
   void CCLattice_IO::compute_shifts(vector <double> &X_shift,vector <double> &Y_shift)
   {
   double auxX,auxY;
   
   for (int kk = 0; kk < MRC_Xcoord.size(); kk++){
	auxX=MRC_Xcoord[kk] - O[0] - MRC_Xindex[kk]*a(0) - MRC_Yindex[kk]*b(0);
	auxY=MRC_Ycoord[kk] - O[1] - MRC_Xindex[kk]*a(1) - MRC_Yindex[kk]*b(1);
	X_shift.push_back(auxX);
	Y_shift.push_back(auxY);

      }

   #undef DEBUG
   #define DEBUG
   #ifdef DEBUG
   for (int kk = 0; kk < Y_shift.size(); kk++)
        cout << X_shift[kk] << " " 
	     << Y_shift[kk] << " " << MRC_CCcoheficiente[kk] << endl;   
   #endif
   #undef DEBUG
  }
