#!/usr/bin/env python
# **************************************************************************
# *
# * Authors:     J.M. De la Rosa Trevin (jmdelarosa@cnb.csic.es)
# *              Josue Gomez Blanco     (jgomez@cnb.csic.es)
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
This script will generate the pw.bashrc and pw.cshrc file to include
"""
import os, sys

from pyworkflow import SETTINGS
from pyworkflow.utils.path import makePath, copyFile, join
from pyworkflow.apps.config import writeDefaults


def updateProjectSettings():
    # Update the settings to all existing projects
    from pyworkflow.manager import Manager
    manager = Manager()
    projects = manager.listProjects()
    
    for p in projects:
        proj = manager.loadProject(p.getName())
        projSettings = proj.settingsPath
        print "Copying settings to: ", join(p.getName(), projSettings)
        copyFile(SETTINGS, projSettings)
        
        
def downloadScons():
    """ Download the scons tgz file and extract it. """
    SCONS_URL = "http://scipionwiki.cnb.csic.es/files/scipion/software/python/"
    SCONS_VERSION = 'scons-2.3.1.tgz'
    print "Downloading scons from " + SCONS_URL + SCONS_VERSION
    os.system("wget " + SCONS_URL + SCONS_VERSION)
    os.system("tar -xzvf " + SCONS_VERSION)
    os.remove(SCONS_VERSION)
        
    

if __name__ == '__main__':
    
    SCIPION_HOME = os.environ['SCIPION_HOME']
    SCIPION_DIRS = ['SCIPION_DATA', 'SCIPION_LOGS', 'SCIPION_TESTS', 'SCIPION_USER_DATA', 'SCIPION_TMP']
    
    print "Installing Scipion in : ", SCIPION_HOME
    
    #os.system('xmipp_python %s' % join(SCIPION_HOME, 'scons', 'scons-2.3.1', 'script', 'scons'))
    args = ' '.join(sys.argv[1:])
    os.system('xmipp_python %s %s' % (join(SCIPION_HOME, 'scons', 'scons-2.3.1', 'script', 'scons'), args))
    
#     # Create DATA folders
#     for d in SCIPION_DIRS:
#         path = os.environ[d]
#         print "  creating %s folder: %s" % (d, path)
#         makePath(path)
# 
#     # Download Scons        
#     downloadScons()
#     # Write default configurations
#     writeDefaults()
#     
#     updateProjectSettings()
    
    
    
        
    
