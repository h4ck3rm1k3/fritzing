from django.conf.urls.defaults import *
from fritzing.apps.projects.views import create, detail, overview

urlpatterns = patterns('',
    url(r'^overview/$', overview, name='projects-overview'),
    # url(r'^create/$', create, name='projects-create'),
    url(r'^(?P<slug>[-\w]+)/$', detail, name='projects-detail'),
)
