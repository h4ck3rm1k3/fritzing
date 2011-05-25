from five import grok

from plone.directives import dexterity

from fritzing.fab.interfaces import IFabOrders, IFabOrder
from fritzing.fab import _

import datetime

from Products.CMFCore.utils import getToolByName


class Index(grok.View):
    grok.context(IFabOrders)
    message = None
    
    label = _(u"Fritzing Fab")
    description = _(u"There's nothing better than turning a concept into product reality.")
    
    def update(self):
        member = self.context.portal_membership.getAuthenticatedMember()
        self.isManager = member.has_role('Manager')
        self.isOwner = member.has_role('Owner')
        if not (self.isManager):
            self.request.set('disable_border', 1)
    
    def getStateTitle(self, item):
        portal_workflow = getToolByName(item, 'portal_workflow')
        state_id = portal_workflow.getInfoFor(item, 'review_state')
        state_title = portal_workflow.getTitleForStateOnType(state_id, item.portal_type)
        return state_title


class PayPalIpn(grok.View):
    """Payment Confirmation
    """
    grok.name('paypal_ipn')
    grok.require('zope2.View')
    grok.context(IFabOrders)
    
    def update(self):
        pass
    
    def render(self):
        pass


class AddForm(dexterity.AddForm):
    """creates a new faborder transparently
    """
    grok.name('faborder')
    grok.require('cmf.AddPortalContent')
    grok.context(IFabOrders)
    
    schema = IFabOrder
    
    label = _(u"New Fab Order")
    description = _(u"Creates a new order for the Fritzing Fab Service")
    
    def create(self, data):
        from zope.component import createObject
        object = createObject('faborder')
        object.id = data['id']
        object.title = data['name']
        user = self.context.portal_membership.getAuthenticatedMember()
        object.email = user.getProperty('email')
        object.reindexObject()
        return object

    def add(self, object):
        self.context._setObject(object.id, object)

    def render(self):
        """create faborder instance and redirect to its default view
        """
        
        # TODO: generate shorter order numbers
        timestamp = "%s" % datetime.datetime.now()
        def r(s):
            if (s == ' '):
                return '_'
            if (s == ':' or s == '.'):
                return '-'
            return s
        simple_timestamp = ''.join(map(r, timestamp))
        
        instance = self.create({
            'id': "order_%s" % (simple_timestamp), 
            'name': u"Order %s" % (timestamp)})
        self.add(instance)
        
        faborderURL = self.context.absolute_url()+"/"+instance.id
        self.request.response.redirect(faborderURL)

