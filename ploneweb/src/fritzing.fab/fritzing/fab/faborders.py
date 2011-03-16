from five import grok

from plone.directives import dexterity

from fritzing.fab.interfaces import IFabOrder, IFabOrders
from fritzing.fab import _

import datetime


class Index(grok.View):
    grok.context(IFabOrders)
    message = None
    
    label = _(u"Fritzing Fab")
    description = _(u"Bla bla description")


class AddForm(dexterity.AddForm):
    """creates a new faborder transparently
    """
    grok.name('faborder')
    grok.require('cmf.AddPortalContent')
    grok.context(IFabOrders)

    # schema = IFabOrder

    label = _(u"New Fab Order")
    description = _(u"Creates a faborder instance...")

    def create(self, data):
        from zope.component import createObject
        object = createObject('faborder')
        object.id = data['id']
        object.title = data['name']
        object.userId = u"%s" % self.context.portal_membership.getAuthenticatedMember()
        object.sketchesOk = False
        object.addressOk = False
        object.isOrdered = False;
        object.reindexObject()
        return object

    def add(self, object):
        self.context._setObject(object.id, object)

    def render(self):
        """create faborder instance and redirect to its edit view
        """
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

