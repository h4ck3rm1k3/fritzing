from five import grok

from fritzing.fab.interfaces import ISketch

from zope.lifecycleevent.interfaces import IObjectCreatedEvent, IObjectModifiedEvent


@grok.subscribe(ISketch, IObjectCreatedEvent)
@grok.subscribe(ISketch, IObjectModifiedEvent)
def modifiedHandler(sketch, event):
    sketch.width = sketch.orderItem.width
    sketch.height = sketch.orderItem.height
    sketch.area = sketch.width * sketch.height
