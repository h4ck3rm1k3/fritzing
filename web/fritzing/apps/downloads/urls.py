from django.conf.urls.defaults import *
from django.conf import settings
from django.views.generic import list_detail
from django.contrib.syndication.views import feed

from downloads.models import Release
from downloads.feeds import PlatformRssFeed, PlatformAtomFeed
from downloads.views import (release_list, release_download,
                             release_detail, release_update)

release_args = {
    'queryset': Release.objects.active()
}

feeds = {
    'rss': PlatformRssFeed,
    'atom': PlatformAtomFeed,
}

urlpatterns = patterns('',
    url(r'history-changes/$',
        list_detail.object_list, 
        dict(release_args, template_name='downloads/release_changes.html'),
        name='downloads_release_changes'),

    url(r'known-issues/$',
        list_detail.object_list,
        dict(release_args, template_name='downloads/release_issues.html'),
        name='downloads_release_changes'),

    url(r'^feed/(?P<url>.*)$',
        feed, {'feed_dict': feeds}, name='downloads_feeds'),

    url(r'^api/1.0/update/(?P<download_id>[^/]+)$',
        release_update, name='downloads_release_update'),

    url(r'(?P<version>[^/]+)/(?P<slug>[^/]*)/(?P<filename>.*)$',
        release_download, name='downloads_release_download'),

    url(r'(?P<version>[^/]+)/$',
        release_detail, name='downloads_release_detail'),

    url(r'', release_list, name='downloads_release_list'),
)
