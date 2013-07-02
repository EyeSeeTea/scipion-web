# **************************************************************************
# *
# * Authors:     J.M. De la Rosa Trevin (jmdelarosa@cnb.csic.es)
# *              Laura del Cano (ldelcano@cnb.csic.es)
# *
# * Unidad de  Bioinformatica of Centro Nacional de Biotecnologia , CSIC
# *
# * This program is free software; you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation; either version 2 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program; if not, write to the Free Software
# * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# * 02111-1307  USA
# *
# *  All comments concerning this program package may be sent to the
# *  e-mail address 'jmdelarosa@cnb.csic.es'
# *
# **************************************************************************
"""
This sub-package contains the XmippCtfMicrographs protocol
"""


from pyworkflow.em import *  
from pyworkflow.em.packages.xmipp3.data import *
from pyworkflow.utils.path import makePath, basename, join, exists
import xmipp


class XmippProtCTFMicrographs(ProtCTFMicrographs):
    """Protocol to perform CTF estimation on a set of micrographs in the project"""
    
    __prefix = join('%(micDir)s','xmipp_ctf')
    _templateDict = {
        # This templates are relative to a micDir
        'micrographs': 'micrographs.xmd',
        'prefix': __prefix,
        'ctfparam': __prefix +  '.ctfparam',
        'psd': __prefix + '.psd',
        'enhanced_psd': __prefix + '_enhanced_psd.xmp',
        'ctfmodel_quadrant': __prefix + '_ctfmodel_quadrant.xmp',
        'ctfmodel_halfplane': __prefix + '_ctfmodel_halfplane.xmp'
#        'ctffind_ctfparam': join('%(micDir)s', 'ctffind.ctfparam'),
#        'ctffind_spectrum': join('%(micDir)s', 'ctffind_spectrum.mrc')
        }
    
    def _prepareCommand(self):
        self._program = 'xmipp_ctf_estimate_from_micrograph'       
        self._args = "--micrograph %(micFn)s --oroot %(micDir)s --fastDefocus"
        
        # Mapping between base protocol parameters and the package specific command options
        self.__params = {'kV': self._params['voltage'],
                'Cs': self._params['sphericalAberration'],
                'sampling_rate': self._params['samplingRate'], 
                'ctfmodelSize': self._params['windowSize'],
                'Q0': self._params['ampContrast'],
                'min_freq': self._params['lowRes'],
                'max_freq': self._params['highRes'],
                'pieceDim': self._params['windowSize'],
                'defocus_range': (self._params['maxDefocus']-self._params['minDefocus'])/2,
                'defocusU': (self._params['maxDefocus']+self._params['minDefocus'])/2
                }
        
        for par, val in self.__params.iteritems():
            self._args += " --%s %s" % (par, str(val))

    def _estimateCTF(self, micFn, micDir):
        """ Run the estimate CTF program """        
        # Create micrograph dir under extra directory
        print "creating path micDir=", micDir
        makePath(micDir)
        if not exists(micDir):
            raise Exception("No created dir: %s " % micDir)
        # Update _params dictionary with mic and micDir
        self._params['micFn'] = micFn
        self._params['micDir'] = self._getFilename('prefix', micDir=micDir)
        # CTF estimation with Xmipp                
        self.runJob(None, self._program, self._args % self._params)    

    def createOutput(self):
        # Create micrographs metadata with CTF information
        mdOut = xmipp.FileName(self._getPath(self._getFilename('micrographs')))        
        micSet = XmippSetOfMicrographs(str(mdOut))
        
        # Label to set values in metadata
        labels = [xmipp.MDL_PSD, xmipp.MDL_PSD_ENHANCED, xmipp.MDL_CTF_MODEL, xmipp.MDL_IMAGE1, xmipp.MDL_IMAGE2]
        # Filenames key related to ctf estimation
        keys = ['psd', 'enhanced_psd', 'ctfparam', 'ctfmodel_quadrant', 'ctfmodel_halfplane']
        
        mapsId = {}
        
        for fn, micDir, mic in self._iterMicrographs():
            xmic = XmippMicrograph(fn)
            ctfparam = self._getFilename('ctfparam', micDir=micDir)
            if exists(ctfparam): # Get filenames
                values = [self._getFilename(key, micDir=micDir) for key in keys]
            else: # No files
                values = ['NA'] * len(labels)
    
            # Set values to the micrograph
            for l, v in zip(labels, values):
                xmic.setValue(l, v)
            micSet.append(xmic)
            mapsId[mic.getId()] = xmic.getId()
         
        #TODO: Verificar con J.M. para que vale este sort
        #micSet.sort()
        
        #Copy attributes from input to output micrographs
        micSet.copyInfo(self.inputMics)   
         
        # If input micrographs have tilt pairs copy the relation
        if self.inputMics.hasTiltPairs():
            
            #TODO: FILL mapIds
            micSet.copyTiltPairs(self.inputMics, mapsId.get)
            
        micSet.write()
            
        auxMdOut = xmipp.FileName(self._getTmpPath('micrographs.xmd'))
        self.runJob(None,"xmipp_ctf_sort_psds","-i %s -o %s" % (mdOut, auxMdOut))
        
        md = xmipp.MetaData(auxMdOut)
        md.write(micSet._getListBlock(), xmipp.MD_APPEND)
        
#        self.runJob(None,"mv","-f %s %s" % (auxMdOut.removeBlockName(),
#                                       mdOut.removeBlockName()))       

        # This property should only be set by CTF estimation protocols
        micSet.setCTF(True)       
        self._defineOutputs(outputMicrographs=micSet)