from five import grok

from plone.directives import form, dexterity

from z3c.form import button

from Products.statusmessages.interfaces import IStatusMessage

import re

from fritzing.fab.interfaces import IFabOrder, IFabOrders, ISketch
from fritzing.fab import _


class Index(grok.View):
    grok.require('zope2.View')
    grok.context(IFabOrder)
    message = None
    
    label = _(u"Order your PCBs")
    description = _(u"Bla bla description")


class Checkout(grok.View):
    """order checkout
    """
    grok.name('checkout')
    grok.require('cmf.ModifyPortalContent')
    grok.context(IFabOrder)
    
    def update(self):
        if not self.context.addressOk:
            IStatusMessage(self.request).addStatusMessage(
                _(u"Address missing/invalid, checkout aborted."), 
                "info")
            return
        if not self.context.sketchesOk:
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


class Edit(dexterity.EditForm):
    """edit the order
    """
    grok.name('edit')
    grok.require('cmf.ModifyPortalContent')
    grok.context(IFabOrder)
    
    schema = IFabOrder
    
    label = _(u"Edit order details")
    description = _(u"you have to bla bla...")


from zope.lifecycleevent.interfaces import IObjectModifiedEvent

@grok.subscribe(IFabOrder, IObjectModifiedEvent)
def modifiedHandler(faborder, event):
    # print "### faborder '%s' modified" % faborder.title
    # print "### faborder '%s' sketches:" % len(faborder)
    # for sketch in faborder:
    #     print "### sketch '%s'" % sketch.title()
    faborder.sketchesOk = (len(faborder) > 0)
    faborder.addressOk = True

    # TODO: for testing we rely on the constraints to validatate the other fields


class AddForm(dexterity.AddForm):
    """adds a sketch
    """
    grok.name('sketch')
    grok.require('cmf.AddPortalContent')
    grok.context(IFabOrder)
    
    schema = ISketch
    
    label = _(u"Add Sketch")
    description = u''
    
    # def update(self):
    #     super(AddForm, self).update()
    
    def create(self, data):
        from zope.component import createObject
        type = createObject('sketch')
        type.id = data['orderItem'].filename.encode("ascii")
        type.title = data['orderItem'].filename
        type.orderItem = data['orderItem']
        type.copies = data['copies']
        type.check = data['check']
        return type

    def add(self, object):
        self.context._setObject(object.id, object)
        faborderURL = self.context.absolute_url()
        self.request.response.redirect(faborderURL)


# class FabOrder(form.SchemaForm):
#     """somefaborderinstance/@@faborder
#     """
#     grok.require('zope2.View')
#     grok.context(IFabOrders)
# 
#     schema = IFabOrder
#     # ignoreContext = True
# 
#     label = _(u"Order your PCB")
#     description = _(u"We will contact you to confirm your order and delivery.")
# 
#     def update(self):
#         # pre-processing
#         # disable Plone's editable border
#         self.request.set('disable_border', True)
#         # call the base class version - this is very important!
#         super(FabOrder, self).update()
#         # postprocessing
#         # nothing at the moment
#     
#     @button.buttonAndHandler(_(u'Order'))
#     def handleApply(self, action):
#         data, errors = self.extractData()
#         if errors:
#             self.status = self.formErrorsMessage
#             return
#         
#         from zope.component import createObject
#         type = createObject('fritzing.fab.faborder')
#         # type = FabOrder()
#         object.id = data['id']
#         object.title = data['name']
#         self.context._setObject(object.id, object)
#         
#         # Redirect back to the front page with a status message
#         IStatusMessage(self.request).addStatusMessage(
#             _(u"Thank you for your order. We will contact you shortly"),
#             "info")
#         
#         contextURL = self.context.absolute_url()
#         self.request.response.redirect(contextURL)
#     
#     @button.buttonAndHandler(_(u"Cancel"))
#     def handleCancel(self, action):
#         """User cancelled. Redirect back to the front page.
#         """
#         contextURL = self.context.absolute_url()
#         self.request.response.redirect(contextURL)
