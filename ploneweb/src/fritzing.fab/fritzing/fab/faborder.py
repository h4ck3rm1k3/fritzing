from five import grok

from plone.directives import dexterity

from Products.statusmessages.interfaces import IStatusMessage

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
        if self.context.isOrdered:
            self.abort(_(u"Already checked out."), "info")
            return
        
        if not self.context.area > 0:
            self.abort(_(u"Sketches missing/invalid, checkout aborted."), "error")
            return
        
        fabManager = self.context.aq_parent.getOwner()
        self.changeOwnership(self.context, fabManager)
        self.context.isOrdered = True
        
        # SEND E-MAILS
    
    def abort(self, message, messageType):
        IStatusMessage(self.request).addStatusMessage(message, messageType)
        faborderURL = self.context.absolute_url()
        self.request.response.redirect(faborderURL)
    
    def changeOwnership(self, obj, userid):
        """ Change ownership of obj to userid """
        # http://keeshink.blogspot.com/2010/04/change-creator-programmatically.html
        # http://plone.org/documentation/manual/plone-community-developer-documentation/content/ownership
        userid = u"%s" % userid
        from Products.CMFCore.utils import getToolByName
        membership = getToolByName(self.context, 'portal_membership')
        user = membership.getMemberById(userid)
        # change ownership
        obj.changeOwnership(user)
        obj.setCreators(userid,)
        # remove owner role from others
        owners = [o for o in obj.users_with_local_role('Owner')]
        for owner in owners:
            roles = list(obj.get_local_roles_for_userid(owner))
            roles.remove('Owner')
            if roles:
                obj.manage_setLocalRoles(owner, roles)
            else:
                obj.manage_delLocalRoles([owner])
        # add owner role to new owner
        roles = list(obj.get_local_roles_for_userid(userid))
        if 'Owner' not in roles:
            roles.append('Owner')
            obj.manage_setLocalRoles(userid, roles)
        # reindex
        obj.reindexObject()


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


