from django.conf.urls.defaults import patterns, url 
from fritzing.apps.fab.views import create, manufacturer_form, details, state_change

urlpatterns = patterns('',
    url(r'^$', create),
    url(r'^create/$', create, name='faborder-create'),
    url(r'^manufacturer_form/(?P<manufacturer_id>[-\w]+)/.*$', manufacturer_form, name='manufacturer-form'),
    url(r'^order/state/change/', state_change, name='faborder-state-change'),
    url(r'^orders/(?P<order_id>[-\w]+)/', details, name='faborder-details'),
)
