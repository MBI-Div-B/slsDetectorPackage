# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
# from .detector import Detector, DetectorError, free_shared_memory
from .eiger import Eiger
from .ctb import Ctb
from .dacs import DetectorDacs, Dac
from .detector import Detector
from .jungfrau import Jungfrau
from .mythen3 import Mythen3
from .gotthard2 import Gotthard2
from .gotthard import Gotthard
from .moench import Moench
from .pattern import Pattern, patternParameters


import _slsdet
xy = _slsdet.xy
defs = _slsdet.slsDetectorDefs

from .enums import *


IpAddr = _slsdet.IpAddr
MacAddr = _slsdet.MacAddr
scanParameters = _slsdet.scanParameters
currentSrcParameters = _slsdet.currentSrcParameters