from django.conf.urls.defaults import patterns, url 
from fritzing.apps.fab.views import create, manufacturer_form

urlpatterns = patterns('',
    #url(r'^$', overview, name='faborders-overview'),
    url(r'^create/$', create, name='faborder-create'),
    url(r'^manufacturer_form/(?P<manufacturer_id>[-\w]+)/$', manufacturer_form, name='manufacturer-form')
)
