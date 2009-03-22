import os
from django.db import models
from django.contrib.auth.models import User
from django.utils.translation import ugettext_lazy as _

from django_extensions.db.models import TimeStampedModel
from tagging.fields import TagField
from imagekit.models import ImageModel
from template_utils.markup import formatter
from licenses.fields import LicenseField
import mptt
from fritzing.apps.parts.managers import PublicPartManager
from fritzing.apps.tools.models import TitleSlugDescriptionModel
from fritzing.apps.projects.models import Project

class Category(TitleSlugDescriptionModel):
    description_html = models.TextField(editable=False, blank=True, null=True)
    parent = models.ForeignKey('self', null=True, blank=True, related_name='children')

    class Meta:
        verbose_name = _('part category')
        verbose_name_plural = _('part categories')
        ordering = ['tree_id', 'title']

    def __unicode__(self):
        return self.title

    def level_and_title(self):
        return "%s %s" % (
            "---" * getattr(self, self._meta.level_attr), self.__unicode__())

    def save(self):
        self.description_html = formatter(self.description)
        super(Category, self).save()

    def delete(self):
        super(Category, self).delete()

    def get_absolute_url(self):
        return ('part_category_detail', (), { 'slug': self.slug })
    get_absolute_url = models.permalink(get_absolute_url)

mptt.register(Category)

class Property(models.Model):
    key = models.TextField()
    value = models.TextField()

    def __unicode__(self):
        return self.key

class Part(TitleSlugDescriptionModel, TimeStampedModel):
    LIVE = 1
    DRAFT = 2
    HIDDEN = 3
    STATUSES = (
        (LIVE, "live"),
        (DRAFT, "draft"),
        (HIDDEN, "hidden"),
    )
    # normalized data
    description_html = models.TextField(editable=False, blank=True)

    # relations
    projects = models.ManyToManyField(Project, verbose_name=_('projects'))
    author = models.ForeignKey(User, blank=True, null=True, related_name='parts')

    # categorization
    category = models.ForeignKey(Category, verbose_name=_('category'), related_name='projects')
    tags = TagField(help_text=_('Comma separated list of tags.'))
    license = LicenseField(related_name='parts', required=False)

    # metadata
    properties = models.ManyToManyField(Property, related_name='parts')
    vendor_info = models.TextField()

    public = models.BooleanField(_('public'), default=True,
        help_text=_('Is publicly visible on the site.'))
    featured = models.BooleanField(_('featured'), default=False,
        help_text=_('Featured projects have a special location in the project overview.'))
    blessed = models.BooleanField(_('blessed'), default=False,
        help_text=_('Blessed projects are core projects from the Fritzing developers or other constant contributor.'))

    objects = models.Manager()
    published = PublicPartManager()

    def __unicode__(self):
        return self.title

    def save(self, force_insert=False, force_update=False):
        self.description_html = formatter(self.description)
        super(Part, self).save(force_insert, force_update)

    def get_absolute_url(self):
        return ('part_detail', (), { 'slug': self.slug })
    get_absolute_url = models.permalink(get_absolute_url)

class Resource(TimeStampedModel):
    title = models.CharField(_('title'), max_length=255)
    url = models.URLField(verify_exists=False, max_length=255)
    part = models.ForeignKey(Part, verbose_name=_('part'), related_name='resources')

    class Meta:
        verbose_name = 'part resource'
        verbose_name_plural = 'part resources'

    def __unicode__(self):
        return self.url

def handle_part_images(instance, filename):
    slug = instance.part.slug
    if len(slug) >= 3:
        return os.path.join("part", slug[0], slug[1], slug[2], slug, "images", filename)
    return os.path.join("part", slug, "images", filename)

class Image(ImageModel):
    title = models.CharField(max_length=255, blank=True, null=True, help_text=_('leave empty to populate with filename'))
    image = models.ImageField(upload_to=handle_part_images)
    part = models.ForeignKey(Part, related_name='photos')
    user = models.ForeignKey(User, blank=True, null=True, related_name='part_images')
    view_count = models.PositiveIntegerField(editable=False, default=0)

    class IKOptions:
        cache_dir = 'photos'
        save_count_as = 'view_count'

    def save(self, force_insert=False, force_update=False):
        if not self.pk and not self.title:
            _, self.title = os.path.split(self.image.name)
        super(Image, self).save(force_insert, force_update)

def handle_data_sheet(instance, filename):
    # http://fritzing.org/media/parts/a/r/d/arduino_beginners_as2/Incredible_Part-210-DIN.pdf
    slug = instance.part.slug
    if len(slug) >= 3:
        return os.path.join("parts", slug[0], slug[1], slug[2], slug, filename)
    return os.path.join("parts", slug, filename)

class DataSheet(models.Model):
    title = models.CharField(_('title'), max_length=255)
    attachment = models.FileField(upload_to=handle_data_sheet, blank=True, null=True)
    url = models.URLField(verify_exists=False, max_length=255, blank=True, null=True)

    # relations
    user = models.ForeignKey(User, blank=True, null=True, related_name='part_datasheets')
    part = models.ForeignKey(Part, verbose_name=_('part'), related_name='data_sheets')

    class Meta:
        verbose_name = _('part data sheet')
        verbose_name_plural = _('part data sheets')

    def __unicode__(self):
        return self.title

    def save(self, force_insert=False, force_update=False):
        if not self.pk and self.attachment:
            _, self.title = os.path.split(self.attachment.name)
        super(DataSheet, self).save(force_insert, force_update)
