from five import grok

from plone.directives import form, dexterity

from z3c.form import button

from Products.statusmessages.interfaces import IStatusMessage

import re

from fritzing.fab.interfaces import IFabOrder, IFabOrders, ISketch
from fritzing.fab import _


class Index(grok.View):
    """Review the order
    """
    grok.require('zope2.View')
    grok.context(IFabOrder)
    
    label = _(u"Order your PCBs")
    description = _(u"Review the order")


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
    
    def update(self):
        if not self.context.area > 0:
            IStatusMessage(self.request).addStatusMessage(
                _(u"Sketches missing/invalid, checkout aborted."), 
                "info")
            return
        
        originalOwner = self.context.getOwner()
        fabManager = self.context.aq_parent.getOwner()
        self.changeOwnership(self.context, fabManager)
        self.context.isOrdered = True
        IStatusMessage(self.request).addStatusMessage(
            _(u"Thank you for your order. We will contact you shortly."), 
            "info")
    
    def render(self):
        faborderURL = self.context.absolute_url()
        self.request.response.redirect(faborderURL)
    
    def changeOwnership(self, obj, userid):
        """ Change ownership of obj to userid """
        # http://keeshink.blogspot.com/2010/04/change-creator-programmatically.html
        # http://plone.org/documentation/manual/plone-community-developer-documentation/content/ownership
        userid = u"%s" % userid
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
def modifiedHandler(faborder, event):
    
    # shipping and taxes
    if faborder.shipTo == u'germany':
        faborder.priceShippingNetto = 5
        faborder.taxesPercent = 19
    elif faborder.shipTo == u'eu':
        faborder.priceShippingNetto = 10
        faborder.taxesPercent = 19
    else:
        faborder.priceShippingNetto = 20
        faborder.taxesPercent = 0
    
    recalculatePrices(faborder)
    
    # TODO: for testing we rely on the constraints to validatate the fields


from zope.app.container.interfaces import IObjectMovedEvent
@grok.subscribe(ISketch, IObjectModifiedEvent)
@grok.subscribe(ISketch, IObjectMovedEvent)
def modifiedHandler(sketch, event):
    faborder = sketch.aq_parent
    # print "### faborder '%s' sketch '%s' event '%s'" % (faborder.title, sketch.title, event)
    
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
    faborder.priceTotalNetto = faborder.priceNetto + faborder.priceQualityChecksNetto + faborder.priceShippingNetto
    faborder.taxes = faborder.priceTotalNetto * faborder.taxesPercent / 100.0
    faborder.priceTotalBrutto = faborder.priceTotalNetto + faborder.taxes


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
        sketch.title = data['orderItem'].filename
        sketch.orderItem = data['orderItem']
        sketch.copies = data['copies']
        sketch.check = data['check']
        return sketch

    def add(self, object):
        self.context._setObject(object.id, object)
        faborderURL = self.context.absolute_url()
        self.request.response.redirect(faborderURL)


