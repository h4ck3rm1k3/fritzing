from five import grok

from zope.lifecycleevent.interfaces import IObjectModifiedEvent
from zope.app.container.interfaces import IObjectMovedEvent
from zope.component import createObject

from plone.directives import dexterity
from plone.dexterity.content import Item

from Products.statusmessages.interfaces import IStatusMessage
from Products.CMFCore.utils import getToolByName
from Products.CMFCore.interfaces import IActionSucceededEvent

from fritzing.fab.interfaces import IFabOrder, ISketch
from fritzing.fab.tools import getStateId, sendStatusMail, recalculatePrices
from fritzing.fab import _


class Index(grok.View):
    """Review the order
    """
    grok.require('zope2.View')
    grok.context(IFabOrder)
    
    label = _(u"Order your PCBs")
    description = _(u"Review the order")
    
    # make tools availible to the template as view.toolname()
    from fritzing.fab.tools \
        import encodeFilename, getStateId, getStateTitle, isStateId
    
    def update(self):
        member = self.context.portal_membership.getAuthenticatedMember()
        self.isManager = member.has_role('Manager')
        if not (self.isManager):
            self.request.set('disable_border', 1)
        
        if self.context.shipTo:
            self.shipToTitle = IFabOrder['shipTo'].vocabulary.getTerm(self.context.shipTo).title
        
        primeCostsPerSquareCm = 0.21
        earningsPerSquareCm = 0.7 * (self.context.pricePerSquareCm - primeCostsPerSquareCm)
        devHourCosts = 50.0
        self.devMinutes = (earningsPerSquareCm * self.context.area) / (devHourCosts / 60.0)


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
        portal_workflow = getToolByName(self, 'portal_workflow')
        review_state = getStateId(None, self.context, portal_workflow)
        
        if review_state != 'open':
            self.addStatusMessage(_(u"Already checked out."), "info")
            return
        if not self.context.area > 0:
            self.addStatusMessage(_(u"Sketches missing/invalid, checkout aborted."), "error")
            return
        
        portal_workflow.doActionFor(self.context, action='submit')
        
        # send e-mails
        sendStatusMail(self.context)
        
        self.request.set('disable_border', 1)
    
    def addStatusMessage(self, message, messageType):
        IStatusMessage(self.request).addStatusMessage(message, messageType)
        faborderURL = self.context.absolute_url()
        self.request.response.redirect(faborderURL)


@grok.subscribe(IFabOrder, IActionSucceededEvent)
def workflowTransitionHandler(faborder, event):
    """event-handler for workflow transitions on IFabOrder instances
    """
    if event.action == 'complete':
        sendStatusMail(faborder)


@grok.subscribe(IFabOrder, IObjectModifiedEvent)
def orderModifiedHandler(faborder, event):
    recalculatePrices(faborder)


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
    if (faborder.area < 50):
        faborder.pricePerSquareCm = 0.70
    elif (faborder.area < 100):
        faborder.pricePerSquareCm = 0.60
    elif (faborder.area < 200):
        faborder.pricePerSquareCm = 0.50
    elif (faborder.area < 500):
        faborder.pricePerSquareCm = 0.40
    else:
        faborder.pricePerSquareCm = 0.35
    
    recalculatePrices(faborder)


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
        if isinstance(object, Item):
            self.context._setObject(object.id, object)


