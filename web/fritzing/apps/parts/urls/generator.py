from django.conf.urls.defaults import *

urlpatterns = patterns('fritzing.apps.parts.views.generator',
    (r'^choose/', 'choose'),
    (r'^form/', 'form'),
    (r'^generate/', 'generate'),
    (r'^', 'choose'), 
)
