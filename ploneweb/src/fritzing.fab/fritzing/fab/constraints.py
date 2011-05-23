from zope.interface import Invalid

from z3c.form import validator

import re

from fritzing.fab import getboardsize
from fritzing.fab import _

from cStringIO import StringIO

import zipfile


class SketchFileValidator(validator.SimpleFieldValidator):
    
    def validate(self, value):
        super(SketchFileValidator, self).validate(value)
        
        fzzName = value.filename
        fzzNameLower = fzzName.lower()
        
        if not (fzzNameLower.endswith('.fzz')):
            raise Invalid(
                _(u"We can only produce from shareable Fritzing sketch files (.fzz)"))
        
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


eMailMatch = re.compile(
    r"[a-zA-Z0-9._%-]+@([a-zA-Z0-9-]+\.)*[a-zA-Z]{2,4}").match

def checkEMail(eMailAddress):
    """Check if the e-mail address looks valid
    """
    if not (eMailMatch(eMailAddress)):
        raise Invalid(_(u"Invalid e-mail address"))
    return True

