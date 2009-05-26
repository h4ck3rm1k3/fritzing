import os
from django.db import models
from django.contrib.auth.models import User
from django.utils.translation import ugettext_lazy as _

from django_extensions.db.models import TimeStampedModel
from tagging.fields import TagField
from fritzing.apps.projects.managers import PublicProjectManager
from fritzing.apps.tools.models import TitleSlugDescriptionModel
from imagekit.models import ImageModel
from template_utils.markup import formatter
from licenses.fields import LicenseField
import mptt


class Category(TitleSlugDescriptionModel):
    description_html = models.TextField(editable=False, blank=True, null=True)
    parent = models.ForeignKey('self', null=True, blank=True,
        related_name='children')

    class Meta:
        verbose_name_plural = 'Categories'
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

    @models.permalink
    def get_absolute_url(self):
        return ('projects-category-detail', (), {'slug': self.slug})

try:
    mptt.register(Category)
except mptt.AlreadyRegistered:
    pass


class Resource(TimeStampedModel):
    title = models.CharField(_('title'), max_length=255)
    url = models.URLField(verify_exists=False, max_length=255)
    project = models.ForeignKey('Project', verbose_name=_('project'),
        related_name='resources')

    class Meta:
        verbose_name = 'project resource'
        verbose_name_plural = 'project resources'

    def __unicode__(self):
        return self.url


class Project(TitleSlugDescriptionModel, TimeStampedModel):
    KIDS = 1
    AMATEUR = 2
    MASTER = 3
    FRITZMEISTER = 4
    DIFFICULTIES = (
        (KIDS, "kids"),
        (AMATEUR, "amateur"),
        (MASTER, "master"),
        (FRITZMEISTER, "fritzmeister"),
    )
    # meta data
    description_html = models.TextField(editable=False, blank=True)
    instructions = models.TextField(blank=True, null=True)
    instructions_html = models.TextField(blank=True, null=True, editable=False)

    # users
    author = models.ForeignKey(User,
        verbose_name=_('author'), related_name='author')
    members = models.ManyToManyField(User, verbose_name=_('members'),
        blank=True, null=True, related_name='member_of')

    # flags
    difficulty = models.IntegerField(_('difficulty'),
        choices=DIFFICULTIES, blank=True, null=True)

    # categorization
    category = models.ForeignKey(
        Category, verbose_name=_('category'), related_name='projects',
        blank=True, null=True)
    tags = TagField(help_text=_('Comma separated list of tags.'))
    license = LicenseField(related_name='projects', required=False)

    # admin stuff
    public = models.BooleanField(_('public'), default=True,
        help_text=_('Is publicly visible on the site.'))
    featured = models.BooleanField(_('featured'), default=False,
        help_text=_('Featured projects have a special location in the '
                    'project overview.'))
    blessed = models.BooleanField(_('blessed'), default=False,
        help_text=_('Blessed projects are core projects from the Fritzing '
                    'developers or other constant contributor.'))

    objects = models.Manager()
    published = PublicProjectManager()

    class Meta:
        get_latest_by = 'created'
        ordering = ['-created', 'title']
        verbose_name = _('project')
        verbose_name_plural = _('projects')

    def __unicode__(self):
        return self.title

    def save(self, force_insert=False, force_update=False):
        self.description_html = formatter(self.description)
        self.instructions_html = formatter(self.instructions)
        super(Project, self).save(force_insert, force_update)

    @models.permalink
    def get_absolute_url(self):
        return ('projects-detail', (), {'slug': self.slug})

    @property
    def main_images(self):
        return self.images.filter(is_heading=True)

    @property
    def sidebar_images(self):
        return self.images.filter(is_heading=False)

    @property
    def get_difficulty(self):
        if self.difficulty:
            diffs = dict(self.DIFFICULTIES)
            return diffs.get(self.difficulty, None)
        return None


def handle_project_images(instance, filename):
    slug = instance.project.slug
    if len(slug) >= 3:
        return os.path.join(
            "projects", slug[0], slug[1], slug[2], slug, "images", filename)
    return os.path.join("projects", slug, "images", filename)


class Image(ImageModel):
    title = models.CharField(max_length=255, blank=True, null=True,
        help_text=_('leave empty to populate with filename'))
    image = models.ImageField(upload_to=handle_project_images)
    project = models.ForeignKey(Project, related_name='images')
    user = models.ForeignKey(User, blank=True,
        null=True, related_name='project_images')
    view_count = models.PositiveIntegerField(editable=False, default=0)
    is_heading = models.BooleanField(default=False)

    class IKOptions:
        cache_dir = 'projects/thumbs'
        cache_filename_format = '%(specname)s/%(filename)s.%(extension)s'
        save_count_as = 'view_count'
        spec_module = 'fritzing.apps.projects.specs'

    def save(self, force_insert=False, force_update=False):
        if not self.pk and not self.title:
            self.title = self.image.name
        super(Image, self).save(force_insert, force_update)


def handle_project_attachment(instance, filename):
    slug = instance.project.slug
    if len(slug) >= 3:
        return os.path.join("projects",
            slug[0], slug[1], slug[2], slug, instance.kind, filename)
    return os.path.join("projects", slug, instance.kind, filename)


class Attachment(models.Model):
    FRITZING_TYPE = 'fritzing'
    CODE_TYPE = 'code'
    EXAMPLE_TYPE = 'example'
    ATTACHMENT_TYPES = (
        (FRITZING_TYPE, _('fritzing file')),
        (CODE_TYPE, _('code')),
        (EXAMPLE_TYPE, _('example')),
    )
    title = models.CharField(_('title'), max_length=255, blank=True,
        null=True, help_text=_('Leave empty to populate with filename'))
    attachment = models.FileField(upload_to=handle_project_attachment)
    project = models.ForeignKey(Project,
        verbose_name=_('project'), related_name='attachments')
    user = models.ForeignKey(User,
        blank=True, null=True, related_name='project_attachments')
    kind = models.CharField(max_length=16, choices=ATTACHMENT_TYPES)

    class Meta:
        verbose_name = _('project attachment')

    def __unicode__(self):
        return self.title

    def save(self, force_insert=False, force_update=False):
        if not self.pk and not self.title:
            self.title = self.attachment.name
        super(Attachment, self).save(force_insert, force_update)

    @property
    def filename(self):
        if self.attachment:
            filename = os.path.split(self.attachment.name)
            return filename[1]
        return None
