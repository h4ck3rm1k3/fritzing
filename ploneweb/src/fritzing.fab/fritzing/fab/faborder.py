from five import grok

from plone.directives import form, dexterity

from z3c.form import button

from Products.statusmessages.interfaces import IStatusMessage

import re

from fritzing.fab.interfaces import IFabOrder, IFabOrders, ISketch
from fritzing.fab import _


class Index(grok.View):
    grok.context(IFabOrder)
    message = None
    
    label = _(u"Order your PCBs")
    description = _(u"Bla bla description")


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
from Products.CMFCore.utils import getToolByName

@grok.subscribe(IFabOrder, IObjectModifiedEvent)
def modifiedHandler(faborder, event):
    # print "### faborder '%s' modified" % faborder.title
    # print "### faborder '%s' sketches:" % len(faborder)
    # for sketch in faborder:
    #     print "### sketch '%s'" % sketch.title()
    faborder.sketchesOk = (len(faborder) > 0)

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


