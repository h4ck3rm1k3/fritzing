from five import grok

from zope.schema import Text, TextLine, ASCIILine, Int, Float, Choice, Bool, Date

from z3c.form import validator

from plone.directives import form
from plone.namedfile.field import NamedBlobFile

from fritzing.fab.constraints import checkEMail, SketchFileValidator
from fritzing.fab import _

from zope.schema.vocabulary import SimpleVocabulary, SimpleTerm


class ISketch(form.Schema):
    """A Fritzing Sketch file
    """
    
    orderItem = NamedBlobFile(
        title = _(u"Sketch file"),
        description = _(u"The .fzz or .fz file of your sketch"))
    
    copies = Int(
        title = _(u"Copies"),
        min = 1,
        default = 1)
    
    check = Bool(
        title = _(u"Quality Check"),
        default = False)
    
    form.omitted(
        'width', 
        'height', 
        'area')
    
    width = Float(
        title = _(u"Width"),
        description = _(u"The width of this sketch in cm"),
        min = 0.0,
        default = 0.0)
    
    height = Float(
        title = _(u"Height"),
        description = _(u"The height of this sketch in cm"),
        min = 0.0,
        default = 0.0)
    
    area = Float(
        title = _(u"Area"),
        description = _(u"The area of this sketch in cm^2"),
        min = 0.0,
        default = 0.0)


validator.WidgetValidatorDiscriminators(SketchFileValidator, field = ISketch['orderItem'])
grok.global_adapter(SketchFileValidator)


class IFabOrder(form.Schema):
    """Fritzing Fab order details
    """
    
    shipTo = Choice(
        title = _(u"Shipping Area"),
        vocabulary = SimpleVocabulary([
            SimpleTerm(value = u'germany', title = _(u'Germany')),
            SimpleTerm(value = u'eu', title = _(u'Europe (EU)')),
            SimpleTerm(value = u'world', title = _(u'the rest of the World'))
        ]))
    
    email = TextLine(
        title = _(u"E-Mail"),
        constraint = checkEMail)
    
    form.omitted(
        'telephone',
        'area', 
        'pricePerSquareCm', 
        'priceNetto', 
        'priceShipping', 
        'numberOfQualityChecks', 
        'priceQualityChecksNetto', 
        'taxesPercent', 
        'taxes', 
        'priceTotalNetto', 
        'priceTotalBrutto',
        'trackingNumber')
    
    telephone = ASCIILine(
        title = _(u"Telephone number"),
        description = _(u"We prefer a mobile number"))
    
    area = Float(
        title = _(u"Area"),
        description = _(u"The total area of all sketches in cm^2"),
        min = 0.0,
        default = 0.0)
    
    pricePerSquareCm = Float(
        title = _(u"Price per cm^2"),
        description = _(u"The price per cm^2 in Euro"),
        min = 0.0,
        default = 0.0)
    
    priceNetto = Float(
        title = _(u"Netto price"),
        description = _(u"The netto price without shipping in Euro"),
        min = 0.0,
        default = 0.0)
    
    priceShipping = Float(
        title = _(u"Shipping costs"),
        description = _(u"The shipping costs in Euro"),
        min = 0.0,
        default = 0.0)
    
    numberOfQualityChecks = Int(
        title = _(u"Number of quality checks"),
        description = _(u"Number of quality checks"),
        min = 0,
        default = 0)
    
    priceQualityChecksNetto = Float(
        title = _(u"Costs for quality checks"),
        description = _(u"The costs for quality checks in Euro"),
        min = 0.0,
        default = 0.0)
    
    taxesPercent = Float(
        title = _(u"Percent Taxes"),
        description = _(u"Taxes like VAT in Percent"),
        min = 0.0,
        default = 0.0)
    
    taxes = Float(
        title = _(u"Taxes"),
        description = _(u"Taxes like VAT"),
        min = 0.0,
        default = 0.0)
    
    priceTotalNetto = Float(
        title = _(u"Total Netto"),
        description = _(u"The netto price costs in Euro"),
        min = 0.0,
        default = 0.0)
    
    priceTotalBrutto = Float(
        title = _(u"Total"),
        description = _(u"The price including shipping costs and taxes in Euro"),
        min = 0.0,
        default = 0.0)
    
    trackingNumber = TextLine(
        title = _(u"Tracking Number"),
        description = _(u"The tracking number assigned by the parcel service"))


class IFabOrders(form.Schema):
    """Fritzing Fab orders Folder
    """
    
    title = TextLine(
        title = _(u"Order-folder name"),
        description = _(u"The title of this fab-instance"))
    
    description = Text(
        title = _(u"Order-folder description"),
        description = _(u"The description (also subtitle) of this fab-instance"))
    
    salesEmail = TextLine(
        title = _(u"Sales e-mail"),
        description = _(u"Order status changes are e-mailed to this address"),
        constraint = checkEMail)
    
    shippingGermany = Float(
        title = _(u"Shipping Costs Germany"),
        description = _(u"The shipping costs for Germany in Euro"),
        min = 0.0,
        default = 4.5)
    
    shippingEU = Float(
        title = _(u"Shipping Costs EU"),
        description = _(u"The shipping costs for the EU in Euro"),
        min = 0.0,
        default = 7.0)
    
    shippingWorld = Float(
        title = _(u"Shipping Costs outside EU"),
        description = _(u"The shipping costs for otside of the EU in Euro"),
        min = 0.0,
        default = 14.0)
    
    taxesGermany = Float(
        title = _(u"Taxes Germany"),
        description = _(u"The taxes for Germany in Percent"),
        min = 0.0,
        default = 19.0)
    
    taxesEU = Float(
        title = _(u"Taxes EU"),
        description = _(u"The taxes for the EU in Percent"),
        min = 0.0,
        default = 19.0)
    
    taxesWorld = Float(
        title = _(u"Taxes outside EU"),
        description = _(u"The taxes for outside of the EU in Percent"),
        min = 0.0,
        default = 0.0)
    
    nextProductionDelivery = Date(
        title = _(u"Next production delivery date"),
        description = _(u"Estimated delivery date of PCBs from the next production"),
        required = False)
    
    nextProductionClosingDate= Date(
        title = _(u"Next production closing date"),
        description = _(u"Orders must be send in before this date to be included in the next production run"),
        required = False)


