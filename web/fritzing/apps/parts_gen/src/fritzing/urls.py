from django.conf.urls.defaults import *

urlpatterns = patterns('',
    (r'^parts_gen/', include('fritzing.parts_gen.urls')),
)
