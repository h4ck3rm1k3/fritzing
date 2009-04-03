from django.conf.urls.defaults import *

urlpatterns = patterns('fritzing.apps.parts.views.generator',
    (r'^index.html', 'choose'),
    (r'^index.htm', 'choose'),
    (r'^form/', 'form'),
    (r'^generate/', 'generate'),
    (r'^', 'choose'), 
)
