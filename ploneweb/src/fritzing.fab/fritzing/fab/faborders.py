import string
import random

from five import grok

from plone.directives import dexterity

from fritzing.fab.interfaces import IFabOrders, IFabOrder
from fritzing.fab import _


class Index(grok.View):
    grok.context(IFabOrders)
    message = None
    
    label = _(u"Fritzing Fab")
    description = _(u"There's nothing better than turning a concept into product reality.")
    
    # make tools availible to the template as view.toolname()
    from fritzing.fab.tools \
        import encodeFilename, getStateId, getStateTitle, isStateId, canDelete
    
    def update(self):
        member = self.context.portal_membership.getAuthenticatedMember()
        self.isManager = member.has_role('Manager')
        self.isOwner = member.has_role('Owner')
        if not (self.isManager):
            self.request.set('disable_border', 1)


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
        
        # generate a nice order-number
        length = 8 # order number length
        chars = list(string.digits) # possible chars in order numbers
        # chars = chars, list(string.ascii_lowercase) # (add letters)
        # chars = sum(chars, []) # (flatten list)
        n = "".join(random.sample(chars, length))
        while self.context.hasObject(n):
            n = "".join(random.sample(chars, length))
        
        instance = self.create({
            'id': n, 
            'name': u"Fritzing Fab order %s" % (n)})
        self.add(instance)
        
        faborderURL = self.context.absolute_url()+"/"+instance.id
        self.request.response.redirect(faborderURL)

