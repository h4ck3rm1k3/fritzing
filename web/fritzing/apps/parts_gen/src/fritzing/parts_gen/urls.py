from django.conf.urls.defaults import *

urlpatterns = patterns('fritzing.parts_gen.views',
    (r'^choose/', 'choose'),
    (r'^form/', 'form'),
    (r'^generate/', 'generate'),
)
