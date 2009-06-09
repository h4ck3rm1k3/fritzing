from django.conf.urls.defaults import *
from django.contrib.syndication.views import feed

from fritzing.apps.events.views import overview, details, icalendar

urlpatterns = patterns('',
    # The overview
    url(r'^$', overview, name='events_overview'),

    # Detail page
    url(r'^(?P<slug>[-\w]+)/$', details, name='event_details'),
    
    # iCal
    url(r'^fritzing-events-ical$', icalendar, name='event_icalendar'),
)
