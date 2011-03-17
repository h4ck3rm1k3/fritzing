from five import grok

import zope.i18nmessageid
from zope import interface
from zope.schema import Text, TextLine, ASCIILine, Int, Choice, Bool
from zope.interface import Invalid

from z3c.form import validator

from plone.directives import form
from plone.namedfile.field import NamedBlobFile

from fritzing.fab.constraints import checkEMail, checkTermsAccepted, checkInstructionsRead
from fritzing.fab import getboardsize
from fritzing.fab import _

from Products.statusmessages.interfaces import IStatusMessage

try:
    from cStringIO import StringIO
except ImportError:
    from StringIO import StringIO

import zipfile


class ISketch(form.Schema):
    """A Fritzing Sketch file and meta-information
    """
    
    width = interface.Attribute("width")
    height = interface.Attribute("height")

    orderItem = NamedBlobFile(
        title=_(u"Sketch file"),
        description=_(u"The .fzz or .fz file of your sketch"))
    
    copies = Int(
        title=_(u"Copies"),
        min=1,
        default=1)
    
    check = Bool(
        title=_(u"Quality Check"),
        default=False)


class SampleValidator(validator.SimpleFieldValidator):
    
    def validate(self, value):
        super(SampleValidator, self).validate(value)
        
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
        
        self.context.width = pairs[0]
        self.context.height = pairs[1]
        
        return True


validator.WidgetValidatorDiscriminators(SampleValidator, field=ISketch['orderItem'])
grok.global_adapter(SampleValidator)


class IFabOrder(form.Schema):
    """The Fritzing Fab order Form
    """
    
    sketchesOk = interface.Attribute("sketchesOk")
    addressOk = interface.Attribute("addressOk")
    isOrdered = interface.Attribute("isOrdered")
    
    userId = interface.Attribute("userId")
    
    # name = TextLine(
    #     title=_(u"Full name"))
    # 
    # company = TextLine(
    #     title=_(u"Company"),
    #     description=_(u"The company you work for."),
    #     required=False)
    # 
    # street = TextLine(
    #     title=_(u"Street"))
    # 
    # streetNumber = TextLine(
    #     title=_(u"House number"))
    # 
    # city = TextLine(
    #     title=_(u"City"))
    # 
    # postcode = TextLine(
    #     title=_(u"Postcode"))
    # 
    # country = TextLine(
    #     title=_(u"Country"))
    
    shipTo = Choice(
        title=_(u"Shipping Area"),
        values=[_(u'Germany'), _(u'Europe (EU)'), _(u'somewhere else')])
    
    # email = TextLine(
    #     title=_(u"E-Mail"),
    #     constraint=checkEMail)
    
    telephone = ASCIILine(
        title=_(u"Telephone number"),
        description=_(u"We prefer a mobile number"))
    
    # instructionsRead = Bool(
    #     title=_(u'I have read the <a href="#" target="_blank">instructions</a>.'),
    #     default=False,
    #     constraint=checkInstructionsRead)
    # 
    # termsAccepted = Bool(
    #     title=_(u'I accept the <a href="#" target="_blank">terms of business</a>.'),
    #     default=False,
    #     constraint=checkTermsAccepted)


class IFabOrders(form.Schema):
    """Fritzing Fab orders Folder
    """

    title = TextLine(
        title=_(u"Order-folder name"),
        )

    description = Text(
        title=_(u"Order-folder description"),
        )

