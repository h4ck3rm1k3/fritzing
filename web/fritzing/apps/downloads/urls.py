from django.conf.urls.defaults import *
from django.conf import settings

from downloads.models import Release
from downloads.feeds import PlatformRssFeed, PlatformAtomFeed

release_args = {
    'queryset': Release.objects.order_by('version', '-release_date'),
}

feeds = {
    'atom': PlatformRssFeed,
    'rss': PlatformAtomFeed,
}

urlpatterns = patterns('django.views.generic.list_detail',
    url(r'^$', 'object_list', dict(release_args, template_name='downloads/release_list.html'), name='downloads_release_list'),
    url(r'history-changes/$', 'object_list', dict(release_args, template_name='downloads/release_changes.html'), name='downloads_release_changes'),
    url(r'known-issues/$', 'object_list', dict(release_args, template_name='downloads/release_issues.html'), name='downloads_release_changes'),
)
urlpatterns += patterns('django.contrib.syndication.views', # always produces content locally
    url(r'^feed/(?P<url>.*)$', 'feed', {'feed_dict': feeds}, name='downloads_feeds'),
)
urlpatterns += patterns('downloads.views',
    url(r'(?P<version>[^/]+)/(?P<slug>[^/]*)/(?P<filename>.*)$', 'release_download', name='downloads_release_download'),
    url(r'(?P<version>[^/]+)/$', 'release_detail', name='downloads_release_detail'),
)
