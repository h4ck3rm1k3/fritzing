from five import grok

import zope.i18nmessageid
from zope.interface import Invalid

from z3c.form import validator

from Products.statusmessages.interfaces import IStatusMessage

import re

from fritzing.fab import getboardsize
from fritzing.fab import _

try:
    from cStringIO import StringIO
except ImportError:
    from StringIO import StringIO

import zipfile


class SketchFileValidator(validator.SimpleFieldValidator):
    
    def validate(self, value):
        super(SketchFileValidator, self).validate(value)
        
        fzzName = value.filename
        fzzNameLower = fzzName.lower()
        
        if not (fzzNameLower.endswith('.fzz')):
            raise Invalid(
                _(u"We can only produce from compressed Fritzing sketch files (.fzz)"))
        
        # if not (fzzNameLower.endswith('.fz') or fzzNameLower.endswith('.fzz')):
        #     raise Invalid(
        #         _(u"We can only produce from Fritzing sketch files (.fz or .fzz)"))
        
        # use StringIO to make the blob to look like a file object:
        fzzData = StringIO(value.data)
        
        zf = None
        try:
            zf = zipfile.ZipFile(fzzData) 
        except:
            raise Invalid(
                _(u"Hmmm, '%s' doesn't seem to be a valid .fzz file. Sorry, we only support those at this time." % fzzName))
        
        pairs = getboardsize.fromZipFile(zf, fzzName)
        
        if not (len(pairs) >= 2):
            raise Invalid(
                _(u"No boards found in '%s'." % fzzName))
        
        if not (len(pairs) == 2):
            raise Invalid(
                _(u"Multiple boards found in '%s'. Sorry, we still work on the support for multiple boards per file." % fzzName))
        
        value.width = pairs[0] / 10
        value.height = pairs[1] / 10
        
        return True


def checkTermsAccepted(value):
    """is the the terms of business-box checked?
    """
    # if not value:
    #     raise Invalid(_(u"You need to accept our terms of business."))
    return True

def checkInstructionsRead(value):
    """is the the instructions read-box checked?
    """
    # if not value:
    #     raise Invalid(_(u"You should read the instructions before ordering."))
    return True

eMailMatch = re.compile(
    r"[a-zA-Z0-9._%-]+@([a-zA-Z0-9-]+\.)*[a-zA-Z]{2,4}").match

def checkEMail(eMailAddress):
    """Check if the e-mail address looks valid
    """
    if not (eMailMatch(eMailAddress)):
        raise Invalid(_(u"Invalid e-mail address"))
    return True

