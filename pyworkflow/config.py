# **************************************************************************
# *
# * Authors:     J.M. De la Rosa Trevin (jmdelarosa@cnb.csic.es)
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
This modules serve to define some Configuration classes
mainly for project GUI
"""

import sys
import os
import json
from collections import OrderedDict
from ConfigParser import ConfigParser
import datetime as dt

import pyworkflow as pw
import pyworkflow.object as pwobj
import pyworkflow.hosts as pwhosts
from pyworkflow.mapper import SqliteMapper
from pyworkflow.utils import strDate

PATH = os.path.dirname(__file__)


def loadSettings(dbPath):
    """ Load a ProjectSettings from dbPath. """
    classDict = dict(globals())
    classDict.update(pwobj.__dict__)
    mapper = SqliteMapper(dbPath, classDict)
    settingList = mapper.selectByClass('ProjectSettings')
    n = len(settingList)

    if n == 0:
        raise Exception("Can't load ProjectSettings from %s" % dbPath)
    elif n > 1:
        raise Exception("Only one ProjectSettings is expected in db, found %d in %s" % (n, dbPath))

    settings = settingList[0]
    settings.mapper = mapper

    return settings


def loadHostsConf(hostsConf):
    """ Load several hosts from a configuration file. 
    Return an OrderedDict where the keys are the hostname
    and the values the configuration for each host.
    """
    # Read from users' config file.
    cp = ConfigParser()
    cp.optionxform = str  # keep case (stackoverflow.com/questions/1611799)
    hosts = OrderedDict()
    
    try:
        assert cp.read(hostsConf) != [], 'Missing file %s' % hostsConf

        for hostName in cp.sections():
            host = pwhosts.HostConfig(label=hostName, hostName=hostName)
            host.setHostPath(pw.SCIPION_USER_DATA)

            # Helper functions (to write less)
            def get(var, default=None):
                if cp.has_option(hostName, var):
                    return cp.get(hostName, var).replace('%_(', '%(')
                else:
                    return default
                
            def getDict(var):
                od = OrderedDict()

                if cp.has_option(hostName, var):
                    for key, value in json.loads(get(var)).iteritems():
                        od[key] = value                
                
                return od

            host.setScipionHome(get('SCIPION_HOME', os.environ['SCIPION_HOME']))
            host.setScipionConfig(get('SCIPION_CONFIG'))
            # Read the address of the remote hosts, 
            # using 'localhost' as default for backward compatibility
            host.setAddress(get('ADDRESS', 'localhost'))
            host.mpiCommand.set(get('PARALLEL_COMMAND'))
            host.queueSystem = pwhosts.QueueSystemConfig()
            host.queueSystem.name.set(get('NAME'))
            
            # If the NAME is not provided or empty
            # do no try to parse the rest of Queue parameters
            if host.queueSystem.hasName(): 
                host.queueSystem.setMandatory(get('MANDATORY', 0))
                host.queueSystem.submitPrefix.set(get('SUBMIT_PREFIX', ''))
                host.queueSystem.submitCommand.set(get('SUBMIT_COMMAND'))
                host.queueSystem.submitTemplate.set(get('SUBMIT_TEMPLATE'))
                host.queueSystem.cancelCommand.set(get('CANCEL_COMMAND'))
                host.queueSystem.checkCommand.set(get('CHECK_COMMAND'))
    
                host.queueSystem.queues = getDict('QUEUES')
                host.queueSystem.queuesDefault = getDict('QUEUES_DEFAULT')
                
            hosts[hostName] = host

        return hosts
    except Exception as e:
        sys.exit('Failed to read settings. The reported error was:\n  %s\n'
                 'To solve it, delete %s and run again.' % (e, hostsConf)) 
        

def loadProtocolsConf(protocolsConf):
    """ Read the protocol configuration from a .conf
    file similar to the one in ~/.config/scipion/protocols.conf,
    which is the default one when no file is passed.
    """

    # Helper function to recursively add items to a menu.
    def add(menu, item):
        "Add item (a dictionary that can contain more dictionaries) to menu"
        children = item.pop('children', [])
        subMenu = menu.addSubMenu(**item)  # we expect item={'text': ...}
        for child in children:
            add(subMenu, child)  # add recursively to sub-menu

    # Read menus from users' config file.
    cp = ConfigParser()
    cp.optionxform = str  # keep case (stackoverflow.com/questions/1611799)
    protocols = OrderedDict()
    
    try:
        assert cp.read(protocolsConf) != [], 'Missing file %s' % protocolsConf

        # Populate the protocol menu from the config file.
        for menuName in cp.options('PROTOCOLS'):
            menu = ProtocolConfig(menuName)
            children = json.loads(cp.get('PROTOCOLS', menuName))
            for child in children:
                add(menu, child)
            protocols[menuName] = menu
        
        return protocols
    
    except Exception as e:
        import traceback
        traceback.print_exc()
        sys.exit('Failed to read settings. The reported error was:\n  %s\n'
                 'To solve it, delete %s and run again.' % (e, protocolsConf))
        

def loadWebConf():
    """ Load configuration parameters to be used in web.
    By default read from: scipion.conf
    """
    # Read menus from users' config file.
    cp = ConfigParser()
    cp.optionxform = str  # keep case (stackoverflow.com/questions/1611799)
    webConf = OrderedDict()

    confFile = os.environ['SCIPION_CONFIG']
    
    assert cp.read(confFile) != [], 'Missing file %s' % confFile

    # Set options from WEB main section
    def setw(option, default):
        if cp.has_option('WEB', option):
            webConf[option] = cp.get('WEB', option)
        else: 
            webConf[option] = default
            
    setw('SITE_URL', 'scipion.cnb.csic.es')
    setw('ABSOLUTE_URL', '')
    setw('ANALYTICS_SCRIPT', '')
    setw('ADMIN_EMAIL', '')
    setw('DEBUG', False)
    setw('WEB_LOG_FILE', 'scipion_web_tools.log')
    
    # Load some parameters per protocol
    protocols = OrderedDict()
    webConf['PROTOCOLS'] = protocols
    
    if cp.has_section('WEB_PROTOCOLS'):
        for protName in cp.options('WEB_PROTOCOLS'):
            protocols[protName] = json.loads(cp.get('WEB_PROTOCOLS', protName)) 
    
    return webConf

def loadEmailConf():
    """ Load configuration parameters to be used for emailing.
    By default read from: scipion.conf
    """
    # Read menus from users' config file.
    cp = ConfigParser()
    cp.optionxform = str  # keep case (stackoverflow.com/questions/1611799)
    emailConf = OrderedDict()

    confFile = os.environ['SCIPION_CONFIG']

    assert cp.read(confFile) != [], 'Missing file %s' % confFile

    # Set options from EMAIL main section
    def setm(option, default):
        if cp.has_option('EMAIL', option):
            emailConf[option] = cp.get('EMAIL', option)
        else:
            emailConf[option] = default

    setm('EMAIL_HOST', 'localhost')
    setm('EMAIL_PORT', '25')
    setm('EMAIL_HOST_USER', 'smtpuser')
    setm('EMAIL_HOST_PASSWORD', 'smtppassword')
    setm('EMAIL_USE_TLS', False)

    setm('EMAIL_FOR_SUBSCRIPTION', 'scipion-users-join@lists.sourceforge.net')

    return emailConf



class ProjectSettings(pwobj.OrderedObject):
    """ Store settings related to a project. """
    def __init__(self, confs={}, **kwargs):
        pwobj.OrderedObject.__init__(self, **kwargs)
        self.config = ProjectConfig()
        self.currentProtocolsView = pwobj.String() # Store the current view selected by the user
        self.nodeList = NodeConfigList() # Store graph nodes positions and other info
        self.mapper = None # This should be set when load, or write
        self.runsView = pwobj.Integer(1) # by default the graph view
        self.readOnly = pwobj.Boolean(False)
        self.runSelection = pwobj.CsvList(int) # Store selected runs
        # Some extra settings stored, now mainly used
        # from the webtools
        self.creationTime = pwobj.String(dt.datetime.now()) # Time when the project was created
        # Number of days that this project is active
        # if None, the project will not expire
        # This is used in webtools where a limited time
        # is allowed for each project
        self.lifeTime = pwobj.Integer()
        # Set a disk quota for the project (in Gb)
        # if None, quota is unlimited
        self.diskQuota = pwobj.Integer()          

    def commit(self):
        """ Commit changes made. """
        self.mapper.commit()

    def getRunsView(self):
        return self.runsView.get()
    
    def setRunsView(self, value):
        self.runsView.set(value)
        
    def getReadOnly(self):
        return self.readOnly.get()
    
    def setReadOnly(self, value):
        self.readOnly.set(value)
        
    def getCreationTime(self):
        creationTime = self.creationTime.get()

        return strDate(creationTime)
    
    def setCreationTime(self, value):
        self.creationTime.set(value)
        
    def getLifeTime(self):
        return self.lifeTime.get()
    
    def setLifeTime(self, value):
        self.lifeTime.set(value)

    def getConfig(self):
        return self.config

    def getProtocolView(self):
        return self.currentProtocolsView.get()

    def setProtocolView(self, protocolView):
        """ Set the new protocol Menu given its index.
        The new ProtocolMenu will be returned.
        """
        self.currentProtocolsView.set(protocolView)

    def write(self, dbPath=None):
        self.setName('ProjectSettings')
        if dbPath is not None:
            self.mapper = SqliteMapper(dbPath, globals())
        else:
            if self.mapper is None:
                raise Exception("Can't write ProjectSettings without mapper or dbPath")

        self.mapper.deleteAll()
        self.mapper.insert(self)
        self.mapper.commit()
        
    def getNodes(self):
        return self.nodeList
    
    def getNodeById(self, nodeId):
        return self.nodeList.getNode(nodeId)
    
    def addNode(self, nodeId, **kwargs):
        return self.nodeList.addNode(nodeId, **kwargs)
    

class ProjectConfig(pwobj.OrderedObject):
    """A simple base class to store ordered parameters"""
    def __init__(self, **args):
        pwobj.OrderedObject.__init__(self, **args)
        self.icon = pwobj.String('scipion_bn.xbm')
        self.logo = pwobj.String('scipion_logo_small.png')


class MenuConfig(object):
    """Menu configuration in a tree fashion.
    Each menu can contains submenus.
    Leaf elements can contain actions"""
    def __init__(self, text=None, value=None,
                 icon=None, tag=None, **kwargs):
        """Constructor for the Menu config item.
        Arguments:
          text: text to be displayed
          value: internal value associated with the item.
          icon: display an icon with the item
          tag: put some tags to items
        **args: pass other options to base class.
        """
        self.text = pwobj.String(text)
        self.value = pwobj.String(value)
        self.icon = pwobj.String(icon)
        self.tag = pwobj.String(tag)
        self.childs = pwobj.List()
        self.openItem = pwobj.Boolean(kwargs.get('openItem', False))

    def addSubMenu(self, text, value=None, **args):
        subMenu = type(self)(text, value, **args)
        self.childs.append(subMenu)
        return subMenu

    def __iter__(self):
        for v in self.childs:
            yield v

    def __len__(self):
        return len(self.childs)

    def isEmpty(self):
        return len(self.childs) == 0


class ProtocolConfig(MenuConfig):
    """Store protocols configuration """
    def __init__(self, text=None, value=None, **args):
        MenuConfig.__init__(self, text, value, **args)
        if 'openItem' not in args:
            self.openItem.set(self.tag.get() != 'protocol_base')

    def addSubMenu(self, text, value=None, **args):
        if 'icon' not in args:
            tag = args.get('tag', None)
            if tag == 'protocol':
                args['icon'] = 'python_file.gif'
            elif tag == 'protocol_base':
                args['icon'] = 'class_obj.gif'
        return MenuConfig.addSubMenu(self, text, value, **args)


class NodeConfig(pwobj.Scalar):
    """ Store Graph node information such as x, y. """
    
    def __init__(self, nodeId=0, x=None, y=None, selected=False, expanded=True):
        pwobj.Scalar.__init__(self)
        # Special node id 0 for project node
        self._values = {'id': nodeId, 
                        'x': pwobj.Integer(x).get(0), 
                        'y': pwobj.Integer(y).get(0), 
                        'selected': selected, 
                        'expanded': expanded}
        
    def _convertValue(self, value):
        """Value should be a str with comman separated values
        or a list.
        """
        self._values = json.loads(value)
            
    def getObjValue(self):
        self._objValue = json.dumps(self._values)
        return self._objValue
    
    def get(self):
        return self.getObjValue()
        
    def getId(self):
        return self._values['id']
    
    def setX(self, x):
        self._values['x'] = x
        
    def getX(self):
        return self._values['x']
    
    def setY(self, y):
        self._values['y'] = y
        
    def getY(self):
        return self._values['y']
    
    def setPosition(self, x, y):
        self.setX(x)
        self.setY(y)
        
    def getPosition(self):
        return self.getX(), self.getY()
        
    def setSelected(self, selected):
        self._values['selected'] = selected
        
    def isSelected(self):
        return self._values['selected']
    
    def setExpanded(self, expanded):
        self._values['expanded'] = expanded
        
    def isExpanded(self):
        return self._values['expanded']
    
    def __str__(self):
        return 'NodeConfig: %s' % self._values
    
    
class NodeConfigList(pwobj.List):
    """ Store all nodes information items and 
    also store a dictionary for quick access
    to nodes query.
    """
    def __init__(self):
        self._nodesDict = {}
        pwobj.List.__init__(self)
        
    def getNode(self, nodeId):
        return self._nodesDict.get(nodeId, None)
    
    def addNode(self, nodeId, **kwargs):
        node = NodeConfig(nodeId, **kwargs)
        self._nodesDict[node.getId()] = node
        self.append(node)
        return node
        
    def updateDict(self):
        self._nodesDict.clear()
        for node in self:
            self._nodesDict[node.getId()] = node
            
    def clear(self):
        pwobj.List.clear(self)
        self._nodesDict.clear()
        
        
class DownloadRecord(pwobj.OrderedObject):
    """ Store information about Scipion downloads. """
    def __init__(self, **kwargs):
        pwobj.OrderedObject.__init__(self, **kwargs)
        
        self.fullName = pwobj.String(kwargs.get('fullName', None))
        self.organization = pwobj.String(kwargs.get('organization', None))
        self.email = pwobj.String(kwargs.get('email', None))
        self.subscription = pwobj.String(kwargs.get('subscription', None))
        self.country = pwobj.String(kwargs.get('country', None))
        self.version = pwobj.String(kwargs.get('version', None))
        self.platform = pwobj.String(kwargs.get('platform', None))
    
