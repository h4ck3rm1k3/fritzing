from django.conf.urls.defaults import *
from fritzing.apps.projects.views import create, detail, overview, by_user

urlpatterns = patterns('',
    url(r'^$', overview, name='projects-overview'),
    url(r'^create/$', create, name='projects-create'),
    url(r'^(?P<slug>[-\w]+)/$', detail, name='projects-detail'),
    url(r'^user/(?P<user_id>[-\w]+)/$', by_user, name='projects-for-user'),
)
