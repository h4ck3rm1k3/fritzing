from django.contrib.sitemaps import Sitemap
from downloads.models import Platform

class PlatformSitemap(Sitemap):

    changefreq = 'daily'
    priority = 0.75
    
    def items(self):
        return Platform.objects.order_by('name')

    def lastmod(self, obj):
        most_recent_download = obj.current_download()
        if most_recent_download.release:
            return most_recent_download.release_date
        return None
