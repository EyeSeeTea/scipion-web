# coding: latin-1
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
Bibtex string file for Xmipp package.
"""

_bibtexStr = """

@article{Mindell2003,
title = "Accurate determination of local defocus and specimen tilt in electron microscopy ",
journal = "JSB ",
volume = "142",
number = "3",
pages = "334 - 347",
year = "2003",
note = "",
issn = "1047-8477",
doi = "http://dx.doi.org/10.1016/S1047-8477(03)00069-8",
url = "http://www.sciencedirect.com/science/article/pii/S1047847703000698",
author = "Mindell, Joseph A. and Grigorieff, Nikolaus",
keywords = "Electron microscopy, Contrast transfer function, Algorithm, Tilt determination "
}

@article{Grigorieff2007,
title = "FREALIGN: High-resolution refinement of single particle structures ",
journal = "JSB ",
volume = "157",
number = "1",
pages = "117 - 125",
year = "2007",
note = "Software tools for macromolecular microscopy ",
issn = "1047-8477",
doi = "http://dx.doi.org/10.1016/j.jsb.2006.05.004",
url = "http://www.sciencedirect.com/science/article/pii/S1047847706001699",
author = "Grigorieff, N",
keywords = "Image processing, Electron microscopy, Protein structure,\{FREALIGN\} "

@article{Wolf2006,
title = "Ewald sphere correction for single-particle electron microscopy ",
journal = "Ultramicroscopy ",
volume = "106",
number = "4-5",
pages = "376 - 382",
year = "2006",
note = "",
issn = "0304-3991",
doi = "http://dx.doi.org/10.1016/j.ultramic.2005.11.001",
url = "http://www.sciencedirect.com/science/article/pii/S0304399105002342",
author = "Wolf, Matthias and DeRosier, David J. and Grigorieff, Nikolaus",
keywords = "Three-dimensional reconstruction, Resolution, Depth of field, \{FREALIGN\} "
}

@article{Grigorieff1998,
title = "Three-dimensional structure of bovine NADH:ubiquinone oxidoreductase (complex I) at 22 � in ice ",
journal = "JSB ",
volume = "277",
number = "5",
pages = "1033 - 1046",
year = "1998",
note = "",
issn = "0022-2836",
doi = "http://dx.doi.org/10.1006/jmbi.1998.1668",
url = "http://www.sciencedirect.com/science/article/pii/S0022283698916680",
author = "Grigorieff, N",
keywords = "complex I, mitochondria, respiratory chain, single particle analysis, electron cryo-microscopy "
}

@article{Stewart2004,
title = "Noise bias in the refinement of structures derived from single particles ",
journal = "Ultramicroscopy ",
volume = "102",
number = "1",
pages = "67 - 84",
year = "2004",
note = "",
issn = "0304-3991",
doi = "http://dx.doi.org/10.1016/j.ultramic.2004.08.008",
url = "http://www.sciencedirect.com/science/article/pii/S0304399104001706",
author = "Stewart, Alex and Grigorieff, Nikolaus",
keywords = "Image processing, Electron microscopy, Protein structure, \{FREALIGN\} "
}

@article{Sindelar2012,
title = "Optimal noise reduction in 3D reconstructions of single particles using a volume-normalized filter ",
journal = "JSB ",
volume = "180",
number = "1",
pages = "26 - 38",
year = "2012",
note = "",
issn = "1047-8477",
doi = "http://dx.doi.org/10.1016/j.jsb.2012.05.005",
url = "http://www.sciencedirect.com/science/article/pii/S104784771200144X",
author = "Sindelar, Charles V. and Grigorieff, Nikolaus ",
keywords = "Electron microscopy, Wiener filter, Single particle, Protein structure, \{SNR\}, \{SSNR\}, Spectral signal-to-noise ratio "
}

@article{Lyumkis2013,
title = "Likelihood-based classification of cryo-EM images using \{FREALIGN\} ",
journal = "JSB ",
volume = "183",
number = "3",
pages = "377 - 388",
year = "2013",
note = "",
issn = "1047-8477",
doi = "http://dx.doi.org/10.1016/j.jsb.2013.07.005",
url = "http://www.sciencedirect.com/science/article/pii/S1047847713001858",
author = "Lyumkis, Dmitry  and Brilot, Axel F.  and Theobald, Douglas L.  and Grigorieff, Nikolaus ",
keywords = "Electron microscopy, Maximum likelihood, Classification, Single particle, Protein structure "
}

@Article{Rohou2015,
   Author="Rohou, A.  and Grigorieff, N. ",
   Title="{{C}{T}{F}{F}{I}{N}{D}4: {F}ast and accurate defocus estimation from electron micrographs}",
   Journal="J. Struct. Biol.",
   Year="2015",
   Volume="192",
   Number="2",
   Pages="216--221",
   Month="Nov",
   doi = "http://dx.doi.org/10.1016/j.jsb.2015.08.008",
   url = "http://www.sciencedirect.com/science/article/pii/S1047847715300460"
}
"""



from pyworkflow.utils import parseBibTex

_bibtex = parseBibTex(_bibtexStr)  
