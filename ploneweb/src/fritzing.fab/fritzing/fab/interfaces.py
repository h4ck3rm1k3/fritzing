from five import grok

import zope.i18nmessageid
from zope import interface
from zope.schema import Text, TextLine, ASCIILine, Int, Float, Choice, Bool

from z3c.form import validator

from plone.directives import form
from plone.namedfile.field import NamedBlobFile

from fritzing.fab.constraints import checkEMail, checkTermsAccepted, checkInstructionsRead, SketchFileValidator
from fritzing.fab import _

from zope.interface import invariant, Invalid

from zope.schema.vocabulary import SimpleVocabulary, SimpleTerm


class ISketch(form.Schema):
    """A Fritzing Sketch file
    """
    
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
    
    form.mode(width='hidden')
    width = Float(
        title=_(u"Width"),
        description=u"The width of this sketch in cm",
        min=0.0,
        default=0.0)
    
    form.mode(height='hidden')
    height = Float(
        title=_(u"Height"),
        description=u"The height of this sketch in cm",
        min=0.0,
        default=0.0)
    
    form.mode(area='hidden')
    area = Float(
        title=_(u"Area"),
        description=u"The area of this sketch in cm^2",
        min=0.0,
        default=0.0)


validator.WidgetValidatorDiscriminators(SketchFileValidator, field=ISketch['orderItem'])
grok.global_adapter(SketchFileValidator)


class IFabOrder(form.Schema):
    """Fritzing Fab order details
    """
    
    shipTo = Choice(
        title=_(u"Shipping Area"),
        vocabulary=SimpleVocabulary([
            SimpleTerm(value=u'germany', title=_(u'Germany')),
            SimpleTerm(value=u'eu', title=_(u'Europe (EU)')),
            SimpleTerm(value=u'world', title=_(u'somewhere else'))
        ]))
    
    email = TextLine(
        title=_(u"E-Mail"),
        constraint=checkEMail)
    
    telephone = ASCIILine(
        title=_(u"Telephone number"),
        description=_(u"We prefer a mobile number"))
    
    form.omitted(
        'isOrdered', 
        'isPaid', 
        'userId', 
        'area', 
        'pricePerSquareCm', 
        'priceNetto', 
        'priceShipping', 
        'numberOfQualityChecks', 
        'priceQualityChecksNetto', 
        'taxesPercent', 
        'taxes', 
        'priceTotalNetto', 
        'priceTotalBrutto')
    
    isOrdered = Bool(
        title=_(u"Is ordered"),
        description=u"True after checkout")
    
    isPaid = Bool(
        title=_(u"Is paid"),
        description=u"True after positive feedback from payment provider")
    
    userId = TextLine(
        title=_(u"User ID"),
        description=u"The orderers user ID")
    
    area = Float(
        title=_(u"Area"),
        description=u"The total area of all sketches in cm^2",
        min=0.0,
        default=0.0)
    
    pricePerSquareCm = Float(
        title=_(u"Price per cm^2"),
        description=u"The price per cm^2 in Euro",
        min=0.0,
        default=0.0)
    
    priceNetto = Float(
        title=_(u"Netto price"),
        description=u"The netto price without shipping in Euro",
        min=0.0,
        default=0.0)
    
    priceShipping = Float(
        title=_(u"Shipping costs"),
        description=u"The shipping costs in Euro",
        min=0.0,
        default=0.0)
    
    numberOfQualityChecks = Int(
        title=_(u"Number of quality checks"),
        description=u"Number of quality checks",
        min=0,
        default=0)
    
    priceQualityChecksNetto = Float(
        title=_(u"Costs for quality checks"),
        description=u"The costs for quality checks in Euro",
        min=0.0,
        default=0.0)
    
    taxesPercent = Float(
        title=_(u"Percent Taxes"),
        description=u"Taxes like VAT in Percent",
        min=0.0,
        default=0.0)
    
    taxes = Float(
        title=_(u"Taxes"),
        description=u"Taxes like VAT",
        min=0.0,
        default=0.0)
    
    priceTotalNetto = Float(
        title=_(u"Total Netto"),
        description=u"The netto price costs in Euro",
        min=0.0,
        default=0.0)
    
    priceTotalBrutto = Float(
        title=_(u"Total"),
        description=u"The price including shipping costs and taxes in Euro",
        min=0.0,
        default=0.0)


class IFabOrders(form.Schema):
    """Fritzing Fab orders Folder
    """

    title = TextLine(
        title=_(u"Order-folder name"),
        )

    description = Text(
        title=_(u"Order-folder description"),
        )

