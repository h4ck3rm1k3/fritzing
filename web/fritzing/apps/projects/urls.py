from django.conf.urls.defaults import patterns, url 
from fritzing.apps.projects.views import create, detail, overview, edit

urlpatterns = patterns('',
    url(r'^$', overview, name='projects-overview'),
    url(r'^create/$', create, name='projects-create'),
    url(r'^edit/(?P<project_id>[-\w]+)/$', edit, name='projects-edit'),
    url(r'^(?P<slug>[-\w]+)/$', detail, name='projects-detail'),
    url(r'^by-user/(?P<username>[-\w]+)/$', overview, name='projects-by-user'),
)
