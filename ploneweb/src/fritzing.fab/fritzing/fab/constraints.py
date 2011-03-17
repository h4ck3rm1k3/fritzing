from five import grok

import zope.i18nmessageid
from zope.interface import Invalid

from Products.statusmessages.interfaces import IStatusMessage

import re

from fritzing.fab import _


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

