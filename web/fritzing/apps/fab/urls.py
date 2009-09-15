from django.conf.urls.defaults import patterns, url 
from fritzing.apps.fab.views import create

urlpatterns = patterns('',
    #url(r'^$', overview, name='faborders-overview'),
    url(r'^create/$', create, name='faborder-create'),
)
