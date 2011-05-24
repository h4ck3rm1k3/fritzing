from five import grok

from plone.directives import dexterity

from Products.statusmessages.interfaces import IStatusMessage

from fritzing.fab.interfaces import IFabOrder, ISketch
from fritzing.fab import _

from Products.CMFCore.utils import getToolByName


class Index(grok.View):
    """Review the order
    """
    grok.require('zope2.View')
    grok.context(IFabOrder)
    
    label = _(u"Order your PCBs")
    description = _(u"Review the order")
    
    def update(self):
        member = self.context.portal_membership.getAuthenticatedMember()
        self.isManager = member.has_role('Manager')
        if not (self.isManager):
            self.request.set('disable_border', 1)
        
        portal_workflow = getToolByName(self.context, 'portal_workflow')
        self.review_state = portal_workflow.getInfoFor(self.context, 'review_state')
        self.isOrdered = (self.review_state != 'open')


class Edit(dexterity.EditForm):
    """Edit the order
    """
    grok.name('edit')
    grok.require('cmf.ModifyPortalContent')
    grok.context(IFabOrder)
    
    schema = IFabOrder
    
    label = _(u"Edit order")
    description = _(u"Edit the order")


class Checkout(grok.View):
    """Order checkout
    """
    grok.name('checkout')
    grok.require('cmf.ModifyPortalContent')
    grok.context(IFabOrder)
    
    label = _(u"Checkout")
    description = _(u"Order checkout")
    
    def render(self):
        faborderURL = self.context.absolute_url()
        self.request.response.redirect(faborderURL)


class CheckoutFail(grok.View):
    """Order checkout fail
    """
    grok.name('checkout_fail')
    grok.require('zope2.View')
    grok.context(IFabOrder)
    
    label = _(u"Checkout")
    description = _(u"Order checkout")
    
    def render(self):
        faborderURL = self.context.absolute_url()
        self.request.response.redirect(faborderURL)


class CheckoutSuccess(grok.View):
    """Order checkout success
    """
    grok.name('checkout_success')
    grok.require('zope2.View')
    grok.context(IFabOrder)
    
    label = _(u"Checkout")
    description = _(u"Order checkout")
    
    def render(self):
        faborderURL = self.context.absolute_url()
        self.request.response.redirect(faborderURL)


class PayPalCheckout(grok.View):
    """Order checkout
    """
    grok.name('paypal_checkout')
    grok.require('zope2.View')
    grok.context(IFabOrder)
    
    label = _(u"PayPal Checkout")
    description = _(u"Order checkout")
    
    
    def update(self):
        portal_workflow = getToolByName(self.context, 'portal_workflow')
        review_state = portal_workflow.getInfoFor(self.context, 'review_state')
        
        if review_state != 'open':
            self.addStatusMessage(_(u"Already checked out."), "info")
            return
        if not self.context.area > 0:
            self.addStatusMessage(_(u"Sketches missing/invalid, checkout aborted."), "error")
            return
        
        portal_workflow.doActionFor(self.context, action='submit')
        
        # TODO: SEND E-MAILS
    
    def addStatusMessage(self, message, messageType):
        IStatusMessage(self.request).addStatusMessage(message, messageType)
        faborderURL = self.context.absolute_url()
        self.request.response.redirect(faborderURL)


from zope.lifecycleevent.interfaces import IObjectModifiedEvent
@grok.subscribe(IFabOrder, IObjectModifiedEvent)
def orderModifiedHandler(faborder, event):
    recalculatePrices(faborder)


from zope.app.container.interfaces import IObjectMovedEvent
@grok.subscribe(ISketch, IObjectModifiedEvent)
@grok.subscribe(ISketch, IObjectMovedEvent)
def sketchModifiedHandler(sketch, event):
    faborder = sketch.aq_parent
    
    # sum up the areas of all sketches and the number of quality checks
    faborder.area = 0
    faborder.numberOfQualityChecks = 0
    for sketch in faborder.listFolderContents():
        faborder.area += sketch.copies * sketch.area
        if sketch.check:
            faborder.numberOfQualityChecks += 1
    
    # choose discount
    if (faborder.area < 70):
        faborder.pricePerSquareCm = 0.59
    elif (faborder.area < 175):
        faborder.pricePerSquareCm = 0.55
    elif (faborder.area < 350):
        faborder.pricePerSquareCm = 0.48
    else:
        faborder.pricePerSquareCm = 0.42
    
    recalculatePrices(faborder)


def recalculatePrices(faborder):
    faborder.priceNetto = faborder.area * faborder.pricePerSquareCm
    faborder.priceQualityChecksNetto = faborder.numberOfQualityChecks * 10.0
    faborder.priceTotalNetto = faborder.priceNetto + faborder.priceQualityChecksNetto
    
    # shipping and taxes
    faborders = faborder.aq_parent
    faborder.priceShipping = faborders.shippingWorld
    faborder.taxesPercent = faborders.taxesWorld
    if faborder.shipTo == u'germany':
        faborder.priceShipping = faborders.shippingGermany
        faborder.taxesPercent = faborders.taxesGermany
    elif faborder.shipTo == u'eu':
        faborder.priceShipping = faborders.shippingEU
        faborder.taxesPercent = faborders.taxesEU
    
    faborder.taxes = faborder.priceTotalNetto * faborder.taxesPercent / 100.0
    faborder.priceTotalBrutto = faborder.priceTotalNetto + faborder.taxes + faborder.priceShipping


class AddForm(dexterity.AddForm):
    """adds a sketch
    """
    grok.name('sketch')
    grok.require('cmf.AddPortalContent')
    grok.context(IFabOrder)
    
    schema = ISketch
    
    label = _(u"Add Sketch")
    description = u''
    
    def create(self, data):
        from zope.component import createObject
        sketch = createObject('sketch')
        sketch.id = data['orderItem'].filename.encode("ascii")
        
        # lets make shure this file doesn't already exist
        if self.context.hasObject(sketch.id):
            IStatusMessage(self.request).addStatusMessage(
                _(u"A Sketch with this name already exists."), "error")
            return None
        
        sketch.title = data['orderItem'].filename
        sketch.orderItem = data['orderItem']
        sketch.copies = data['copies']
        sketch.check = data['check']
        return sketch
    
    def add(self, object):
        from plone.dexterity.content import Item
        if isinstance(object, Item):
            self.context._setObject(object.id, object)


