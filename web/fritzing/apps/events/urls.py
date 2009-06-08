from django.conf.urls.defaults import *
from django.contrib.syndication.views import feed

from fritzing.apps.events.views import overview, details

urlpatterns = patterns('',
    # The overview
    url(r'^$', overview, name='events_overview'),

    # Detail page
    url(r'^(?P<slug>[-\w]+)/$', details, name='events_details'),
)
