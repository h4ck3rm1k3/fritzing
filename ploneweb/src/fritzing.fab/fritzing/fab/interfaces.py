import zope.i18nmessageid

from zope import interface
from zope.schema import Text, TextLine, ASCIILine, Int, Choice, Bool

from plone.directives import form
from plone.namedfile.field import NamedBlobFile

from fritzing.fab.constraints import checkFiletype, checkEMail, checkTermsAccepted, checkInstructionsRead
from fritzing.fab import _


class ISketch(form.Schema):
    """A Fritzing Sketch file and meta-information
    """
    
    orderItem = NamedBlobFile(
        title=_(u"Sketch file"),
        description=_(u"The .fzz or .fz file of your sketch"),
        constraint=checkFiletype)
    
    copies = Int(
        title=_(u"Copies"),
        min=1,
        default=1)
    
    check = Bool(
        title=_(u"Quality Check"),
        default=False)


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

