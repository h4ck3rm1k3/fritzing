from django.conf.urls.defaults import patterns, url
from fritzing.apps.profiles.views import profiles, profile, edit

urlpatterns = patterns('',
    url(r'^$', profiles, name='profile_list'),
    url(r'^(?P<username>[\w\._-]+)/edit$', edit, name='profile_edit'),
    url(r'^(?P<username>[\w\._-]+)/$', profile, name='profile_detail'),
)
