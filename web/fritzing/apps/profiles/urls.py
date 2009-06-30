from django.conf.urls.defaults import *

urlpatterns = patterns('',
    #url(r'^username_autocomplete/$', 'misc.views.username_autocomplete_all', name='profile_username_autocomplete'),
    url(r'^$', 'fritzing.apps.profiles.views.profiles', name='profile_list'),
    url(r'^(?P<username>[\w\._-]+)/$', 'fritzing.apps.profiles.views.profile', name='profile_detail'),
)
