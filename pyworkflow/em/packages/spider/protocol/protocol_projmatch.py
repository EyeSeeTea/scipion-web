# **************************************************************************
# *
# * Authors:     J.M. De la Rosa Trevin (jmdelarosa@cnb.csic.es)
#                Tapu Shaikh            (shaikh@ceitec.muni.cz)
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
# *  e-mail address 'jgomez@cnb.csic.es'
# *
# **************************************************************************
"""
"""

from os.path import join

import pyworkflow.utils as pwutils
import pyworkflow.em as em
import pyworkflow.protocol.params as params
from pyworkflow.em.protocol import ProtRefine3D

from ..spider import SpiderDocFile, writeScript, getScript, runScript
from protocol_base import SpiderProtocol



                               
class SpiderProtRefinement(ProtRefine3D, SpiderProtocol):
    """
    """
    _label = 'refinement'
    
    #--------------------------- DEFINE param functions --------------------------------------------   
    def _defineParams(self, form):
        form.addSection(label='Input')

        form.addParam('inputParticles', params.PointerParam, 
                      pointerClass='SetOfParticles', 
                      label="Input particles", important=True,
                      help='Select the input particles.\n')  
        
        form.addParam('input3DReference', params.PointerParam,
                      pointerClass='Volume', 
                      label='Initial 3D reference volume:',
                      help='Input 3D reference reconstruction.\n')
        
        form.addParam('numberOfIterations', params.IntParam, default=10,
                      label='Number of iterations:',
                      help='Set the number of iterations. Iterative reconstruction'
                           'improves the overall normalization of the 2D images'
                           'as they are inserted into the reconstructed volume,'
                           'and allows for the exclusion of the poorer quality'
                           'images.')
        
#         form.addParam('symmetry', StringParam, default='c1',
#                       condition='not doContinue',
#                       label='Symmetry group',
#                       help='Set the symmetry; if no value is given then the model'
#                            'is assumed to have no symmetry. Choices are: i(n),'
#                            ' c(n), d(n), tet, icos, or oct.'
#                            'See http://blake.bcm.edu/emanwiki/EMAN2/Symmetry'
#                            'for a detailed descript of symmetry in Eman.')


        form.addParallelSection(threads=4, mpi=0)
    
    #--------------------------- INSERT steps functions --------------------------------------------  
    def _insertAllSteps(self):        
        # Create new stacks and selfiles per defocus groups
        self._insertFunctionStep('convertInputStep', self.inputParticles.get().getObjId())

        for s in ['refine', 'prepare', 'grploop', 'mergegroups', 
                  'enhance', 'endmerge', 'smangloop', 'endrefine']:
            self._insertFunctionStep('runScriptStep', '%s.pam' % s)
                
        self._insertFunctionStep('createOutputStep')
    
    #--------------------------- STEPS functions --------------------------------------------
    
    def convertInputStep(self, particlesId):
        """ Convert all needed inputs before running the refinement script. """
        partSet = self.inputParticles.get()

        self._writeParamsFile(partSet)        
        
        self._writeGroupFiles(partSet)
        
        # Convert the input volume
        reconsPath = self._getExtraPath('Reconstruction')
        volName = 'vol001.stk'
        volPath = self._getExtraPath(volName)
        em.ImageHandler().convert(self.input3DReference.get(), volPath)
        
        self._writeRefinementScripts(volName)
                
    def _writeRefinementScripts(self, volName):
        """ Write the needed scripts to run refinement
        and substitute some values.
        """
        
        refPath = self._getExtraPath('Refinement')
        pwutils.makePath(refPath)
        
        def path(p):
            """ Escape path with '' and add ../ """
            return "'%s'" % join('..', p)
        
        def script(name, paramsDict={}):
            outputScript=join(refPath, name)
            writeScript(getScript('projmatch', 'Refinement',name), outputScript, paramsDict)
            
        
        params = {'[iter-end]': self.numberOfIterations.get(),
                  '[vol_orig]': path(volName),
                  '[sel_group_orig]': path('sel_group'),
                  '[sel_particles_orig]': path('group_{***[grp]}_selfile'),
                  '[group_align_orig]': path('group_{***[grp]}_align'),
                  '[unaligned_images_orig]': path('group_{***[grp]}_stack'),
                  }        
        script('refine_settings.pam', params)
        for s in ['refine', 'prepare', 'grploop', 'mergegroups', 
                  'enhance', 'endmerge', 'smangloop', 'endrefine']:
            script('%s.pam' % s)
        
    def _writeParamsFile(self, partSet):
        acq = partSet.getAcquisition()
        params = {'datetime': 'now',
                  'pixelSize': partSet.getSamplingRate(),
                  'voltage': acq.getVoltage(),
                  'sphericalAberration': acq.getSphericalAberration(),
                  'windowSize': partSet.getDimensions()[0]}
        
        paramFile = open(self._getExtraPath('params.stk'), 'w+')
        paramFile.write("""
 ;spi/dat  Generated by Scipion on %(datetime)s  params.stk
    5 1    %(pixelSize)f           ; pixel size (A)
    6 1    %(voltage)f             ; electron energy (kV)
    7 1    %(sphericalAberration)f ; spherical aberration (mm)
   17 1    %(windowSize)f          ; window size (pixels)
                        """ % params)
        paramFile.close()        
        
    def _writeGroupFiles(self, partSet):
        """Write files that are needed by each defocus group:
        - stack
        - selfile
        - docfile
        """
        ih = em.ImageHandler()
        # Keep a dict with all groups found in particles
        groupDict = {}
        template = self._getExtraPath('group%03d_%s.stk')
        
        for part in partSet:
            defocusGroup = self._getDefocusGroup(part)
            
            if defocusGroup not in groupDict:
                groupInfo = DefocusGroupInfo(defocusGroup, template, ih)
                groupDict[defocusGroup] = groupInfo
            else:
                groupInfo = groupDict[defocusGroup]
                
            groupInfo.addParticle(part)

        # Write the docfile with the defocus groups information
        # like the number of particles and the defocus
        groupsDoc = SpiderDocFile(self._getExtraPath('sel_group.stk'), 'w+')
        for gi in groupDict.values():
            groupsDoc.writeValues(gi.number, gi.counter, gi.defocus)
            # Close each group docfile
            gi.close()
        groupsDoc.close()
        
#         fn, ext = splitext(stackFn)
#         # Change to BigEndian
#         runTemplate('cp_endian.spi', ext[1:], 
#                   {'[particles]': fn + '@******', 
#                    '[particles_big]': fn + '_big@******',
#                    '[numberOfParticles]': imgSet.getSize()
#                    })
#         moveFile(fn + '_big' + ext, stackFn)
    
    def runScriptStep(self, script):
        """ Just run the script that was generated in convertInputStep. """
        refPath = self._getExtraPath('Refinement')
        #runScript(script, 'stk', cwd=refPath)
        print ">>> Running script: ", script
        
        
    def projectStep(self, volumeId):
        pass
     
    def alignStep(self):
        """Create new stacks and selfiles per defocus groups """
        pass
    
    def reconstructStep(self):
        pass   
    
    def mergeStep(self):
        pass    
    
    def createOutputStep(self):
        pass
    
    #--------------------------- INFO functions -------------------------------------------- 
    def _validate(self):
        errors = []
        return errors
    
    def _summary(self):
        summary = []
        return summary
    
    #--------------------------- UTILS functions --------------------------------------------
    def _getDefocusGroup(self, img):
        return img.getMicId()
    
    
class DefocusGroupInfo():
    """ Helper class to store some information about 
    defocus groups like the number of particles
    or the docfile to be generated.
    """
    def __init__(self, defocusGroup, template, ih):
        self.ih = ih
        self.number = defocusGroup
        self.selfile = template % (defocusGroup, 'selfile') 
        self.docfile = template % (defocusGroup, 'align') 
        self.stackfile = template % (defocusGroup, 'stack')
        self.counter = 0 # number of particles in this group
        # 
        self.sel = SpiderDocFile(self.selfile, 'w+')
        self.doc = SpiderDocFile(self.docfile, 'w+')
        now = 'now' #FIXME
        self.doc.writeComment("spi/dat   Generated by Scipion on %s" % now)
        self.doc.writeComment("  KEY       PSI,    THE,    PHI,   REF#,    EXP#,  CUM.{ROT,   SX,    SY},  NPROJ,   DIFF,      CCROT,    ROT,     SX,     SY,   MIR-CC")
        
    def addParticle(self, img):
        self.counter += 1
        if self.counter == 1:
            ctf = img.getCTF()
            self.defocus = (ctf.getDefocusU() + ctf.getDefocusV()) / 2.
        self.ih.convert(img, (self.counter, self.stackfile))
        self.sel.writeValues(self.counter)
        #FIXME: use real alignmet/projection parameters, now using DUMMY values
        rot = tilt = psi = 0.00
        shiftX = shiftY = 0.00
        values = [0.00, tilt,  rot, -1, self.counter, -1, psi, shiftX, shiftY, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00]
        self.doc.writeValues(*values)
        
    def close(self):
        self.sel.close()
        self.doc.close()
        
    