from django.conf.urls.defaults import *
from fritzing.apps.projects.views import create

urlpatterns = patterns('',
#    url(r'^$', faq_list_by_group, name = 'faq'),
    url(r'^create/$', create, name='projects-create'),
)
