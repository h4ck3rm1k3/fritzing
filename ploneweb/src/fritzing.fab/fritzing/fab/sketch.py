from five import grok

from plone.directives import dexterity
from plone.dexterity.content import Item

from fritzing.fab.interfaces import IFabOrder, ISketch
from fritzing.fab import _


from zope.lifecycleevent.interfaces import IObjectCreatedEvent, IObjectModifiedEvent

@grok.subscribe(ISketch, IObjectCreatedEvent)
@grok.subscribe(ISketch, IObjectModifiedEvent)
def modifiedHandler(sketch, event):
    sketch.width = sketch.orderItem.width
    sketch.height = sketch.orderItem.height
    sketch.area = sketch.width * sketch.height
    # print "### sketch '%s' modified" % (sketch.title)
