from ._entities import *

BtcBlock.__str__ = lambda self: self.toHex()