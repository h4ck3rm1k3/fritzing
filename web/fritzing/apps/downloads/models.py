import mimetypes
from datetime import datetime
from django.db import models
from django.contrib.sites.models import Site
from django.utils.translation import ugettext_lazy as _

class Platform(models.Model):
    name = models.CharField(_('name'), max_length=255)
    slug = models.SlugField(_('slug'), max_length=255, unique=True)

    class Meta:
        ordering = ('slug',)
        verbose_name = _('platform')
        verbose_name_plural = _('platforms')

    def __unicode__(self):
        return self.name

    def get_absolute_url(self):
        return ('downloads_platform_detail', [self.slug])
    get_absolute_url = models.permalink(get_absolute_url)

    def current_download(self):
        if self.download_set.count():
            return self.download_set.all()[0]
        return None

class ReleaseManager(models.Manager):
    def active(self):
        return self.filter(active=True)

class Release(models.Model):
    version = models.CharField(_('version'), max_length=64)
    description = models.TextField(_('description'), blank=True)
    changelog = models.TextField(_('changelog'), blank=True)
    known_issues = models.TextField(_('known issues'), blank=True)
    release_date = models.DateTimeField(_('date released'), default=datetime.now)
    active = models.BooleanField(_('active'), default=True)

    objects = ReleaseManager()

    class Meta:
        ordering = ('version', 'release_date')
        verbose_name = _('release')
        verbose_name_plural = _('releases')

    def __unicode__(self):
        return self.version

    def get_absolute_url(self):
        return ('downloads_release_detail', [self.version])
    get_absolute_url = models.permalink(get_absolute_url)

    def downloads(self):
        counter = 0
        for download in self.download_set.all():
            counter += download.counter
        return counter
    downloads.short_description = 'Downloads'

class Download(models.Model):
    release = models.ForeignKey(Release, verbose_name=_('release'))
    platform = models.ForeignKey(Platform, verbose_name=_('platform'))
    filename = models.FileField(_('file'), upload_to='downloads', blank=True, null=True)
    mime_type = models.CharField(_('mime type'), max_length=255, blank=True, null=True)
    counter = models.IntegerField(_('counter'), default=0)

    class Meta:
        ordering = ('release', 'platform')
        verbose_name = _('download')
        verbose_name_plural = _('downloads')

    def __unicode__(self):
        return self.filename.path.split('/')[-1]

    def save(self, force_insert=False, force_update=False):
        self.mime_type = 'application/octet-stream'
        if self.filename.path:
            mimetype, enc = mimetypes.guess_type(self.filename.path, strict=False)
            self.mime_type = mimetype
        super(Download, self).save(force_insert, force_update)

    def get_absolute_url(self):
        filename = self.filename.path.split('/')[-1]
        return ('downloads_release_download',
            [self.release.version, self.platform.slug, filename])
    get_absolute_url = models.permalink(get_absolute_url)
