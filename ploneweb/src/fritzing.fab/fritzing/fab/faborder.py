from five import grok

from zope.app.component.hooks import getSite
from zope.lifecycleevent.interfaces import IObjectModifiedEvent
from zope.app.container.interfaces import IObjectMovedEvent
from zope.component import createObject

from plone.directives import dexterity
from plone.dexterity.content import Item

from Products.statusmessages.interfaces import IStatusMessage
from Products.CMFCore.utils import getToolByName
from Products.CMFCore.interfaces import IActionSucceededEvent

from email.MIMEText import MIMEText
from email.Utils import formataddr
from smtplib import SMTPRecipientsRefused

from fritzing.fab.interfaces import IFabOrder, ISketch
from fritzing.fab import _


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
        self.state_id = portal_workflow.getInfoFor(self.context, 'review_state')
        self.state_title = portal_workflow.getTitleForStateOnType(self.state_id, self.context.portal_type)
        self.isOrdered = (self.state_id != 'open')
        if self.context.shipTo:
            self.shipToTitle = IFabOrder['shipTo'].vocabulary.getTerm(self.context.shipTo).title


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
        
        # SEND E-MAILS
        sendStatusMail(self.context)
    
    def addStatusMessage(self, message, messageType):
        IStatusMessage(self.request).addStatusMessage(message, messageType)
        faborderURL = self.context.absolute_url()
        self.request.response.redirect(faborderURL)


def sendStatusMail(context):
    """Sends notification on the order status to the orderer and faborders.salesEmail
    """
    mail_text = u""
    charset = 'utf-8'
    
    portal_workflow = getToolByName(context, 'portal_workflow')
    state_id = portal_workflow.getInfoFor(context, 'review_state')
    state_title = portal_workflow.getTitleForStateOnType(state_id, context.portal_type)
    portal = getSite()
    mail_template = portal.mail_order_status_change
    faborders = context.aq_parent
    
    from_address = faborders.salesEmail
    from_name = "Fritzing Fab"
    user  = context.getOwner()
    to_address = user.getProperty('email')
    to_name = user.getProperty('fullname')
    
    mail_text = mail_template(
        to_name = to_name,
        state_id = state_id,
        state_title = state_title,
        faborder = context,
        ship_to = IFabOrder['shipTo'].vocabulary.getTerm(context.shipTo).title,
        )
    
    try:
        host = getToolByName(context, 'MailHost')
        # send our copy:
        host.secureSend(
            message = MIMEText(mail_text, 'plain', charset), 
            mfrom = formataddr((from_name, from_address)),
            mto = formataddr((from_name, from_address)),
            charset = charset,
        )
        # send notification for the orderer:
        host.secureSend(
            message = MIMEText(mail_text, 'plain', charset), 
            mfrom = formataddr((from_name, from_address)),
            mto = formataddr((to_name, to_address)),
            charset = charset,
        )
    except SMTPRecipientsRefused:
        # Don't disclose email address on failure
        raise SMTPRecipientsRefused('Recipient address rejected by server')


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


