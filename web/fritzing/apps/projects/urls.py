from django.conf.urls.defaults import *
from fritzing.apps.projects.views import create, detail, overview

urlpatterns = patterns('',
    # url(r'^$', overview, name='project-overview'),
    # url(r'^create/$', create, name='projects-create'),
    url(r'^detail/(?P<slug>[-\w]+)/$', detail, name='projects-detail'),
)
