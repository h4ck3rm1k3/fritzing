import os

from django.core.urlresolvers import reverse
from django.core.exceptions import ObjectDoesNotExist
from django.contrib.sites.models import Site
from django.contrib.syndication.feeds import Feed
from django.utils.feedgenerator import Atom1Feed
from django.conf import settings

from downloads.models import Platform, Release, Download

class PlatformRssFeed(Feed):
    title_template = 'downloads/platform_feed_title.html'
    description_template = 'downloads/platform_feed_description.html'
    _release_type = None

    def get_object(self, bits):
        if len(bits) < 1:
            raise ObjectDoesNotExist
        if len(bits) > 1:
            if bits[1] in dict(Release.TYPE_CHOICES).values():
                self._release_type = bits[1]
        return Platform.objects.get(slug=bits[0])

    def title(self, obj):
        return 'Fritzing %s releases' % obj.name

    def link(self, obj):
        if obj.current_download():
            return obj.current_download().release.get_absolute_url()
        return reverse('downloads_release_list')

    def item_link(self, obj):
        return '%s#%s' % (
            obj.release.get_absolute_url(),
            obj.release.release_date.strftime('%Y-%m-%d'))

    def item_enclosure_url(self, obj):
        "Use the filename of the release as the enclosure url."
        domain = Site.objects.get_current().domain
        url = 'http://%s%s' % (domain, obj.get_absolute_url())
        if not url:
            url = None
        return url

    def item_enclosure_length(self, obj):
        "Use the file size"
        return 1
        return obj.filename.size

    def item_enclosure_mime_type(self, obj):
        "Use the mimetype module to guess"
        return obj.mime_type

    def item_author_name(self, obj):
        return 'Fritzing Team'

    def item_author_email(self, obj):
        return 'info@fritzing.org'

    def item_pubdate(self, obj):
        return obj.release.release_date

    def items(self, obj):
        qs = Download.objects.active()
        if obj:
            qs = qs.filter(platform__id__exact=obj.id)
        if self._release_type:
            types = dict([(v, k) for k, v in Release.TYPE_CHOICES])
            release_type = types.get(self._release_type, None)
            if release_type:
                qs = qs.filter(release__type=release_type)
        return qs.order_by('-release__release_date')

class PlatformAtomFeed(PlatformRssFeed):
    feed_type = Atom1Feed
