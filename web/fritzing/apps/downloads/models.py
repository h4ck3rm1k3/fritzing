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
        downloads = self.download_set.all()
        if downloads:
            return self.download_set.all()[0]
        return None

class ReleaseManager(models.Manager):

    def active(self):
        return self.filter(active=True)

    def interim(self):
        return self.active().filter(type=Release.INTERIM)

    def main(self):
        return self.active().filter(type=Release.MAIN)

class Release(models.Model):
    INTERIM = 1
    MAIN = 2
    TYPE_CHOICES = (
        (INTERIM, 'interim'),
        (MAIN, 'main')
    )
    version = models.CharField(_('version'), max_length=64,
        help_text=_("The version string used in the URL and on the download page. Please don't use spaces here."))
    description = models.TextField(_('description'), blank=True)
    changelog = models.TextField(_('changelog'), blank=True,
        help_text=_("The changelog displayed at http://fritzing.org/downloads/history-changes/. HTML is allowed."))
    known_issues = models.TextField(_('known issues'), blank=True,
        help_text=_("The known issues displayed at http://fritzing.org/downloads/known-issues/. HTML is allowed."))
    release_date = models.DateTimeField(_('date released'), default=datetime.now)
    active = models.BooleanField(_('active'), default=True)
    type = models.IntegerField(_('type'), choices=TYPE_CHOICES, default=INTERIM,
        help_text=_('Depending on the "use it at your own risk" level.'))

    objects = ReleaseManager()

    class Meta:
        ordering = ('-release_date',)
        verbose_name = _('release')
        verbose_name_plural = _('releases')
        get_latest_by = 'release_date'

    def __unicode__(self):
        return self.version

    def get_type_name(self):
        return dict(self.TYPE_CHOICES).get(self.type)

    def get_absolute_url(self):
        return ('downloads_release_detail', [self.version])
    get_absolute_url = models.permalink(get_absolute_url)

    def downloads(self):
        counter = 0
        for download in self.download_set.all():
            counter += download.counter
        return counter
    downloads.short_description = 'Downloads'

    def updates(self):
        counter = 0
        for download in self.download_set.all():
            counter += download.updated
        return counter
    downloads.short_description = 'Updates'

class DownloadManager(models.Manager):

    def active(self):
        return self.filter(release__active=True)

class Download(models.Model):
    release = models.ForeignKey(Release, verbose_name=_('release'))
    platform = models.ForeignKey(Platform, verbose_name=_('platform'))
    filename = models.FileField(_('file'), upload_to='downloads', blank=True, null=True)
    mime_type = models.CharField(_('mime type'), max_length=255, blank=True, null=True)
    counter = models.IntegerField(_('counter'), default=0)
    updated = models.IntegerField(_('updated'), default=0)

    objects = DownloadManager()

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

    def get_update_url(self):
        return ('downloads_release_update', [self.pk])
    get_update_url = models.permalink(get_update_url)
