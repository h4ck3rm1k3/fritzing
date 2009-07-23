import settings
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
from vcstorage.fields import VcFileField
from vcstorage.storage import MercurialStorage
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
        help_text=_('How difficult it is to build this project'),
        choices=DIFFICULTIES, blank=True, null=True)

    # categorization
    category = models.ForeignKey(
        Category, verbose_name=_('category'), related_name='projects',
        help_text=_('Select a category that fits your project'),
        blank=True, null=True)
    tags = TagField(help_text=_('Add a couple of comma-separated tags'))
    license = LicenseField(related_name='projects')

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
    
    def _get_attachments(self,kind):
        return Attachment.objects.filter(project__id=self.id, kind=kind)
    
    @property
    def get_fritzing_attachments(self):
        return self._get_attachments(Attachment.FRITZING_TYPE)
        
    @property
    def get_code_attachments(self):
        return self._get_attachments(Attachment.CODE_TYPE)
        
    @property
    def get_example_attachments(self):
        return self._get_attachments(Attachment.EXAMPLE_TYPE)


class Image(ImageModel):
    def project_images_path(self, filename):
        slug = self.project.slug
        username = self.project.author.username
        path = "/".join(list(username[:3])) if len(username) >= 3 else ''
        return os.path.join(settings.USER_FILES_FOLDER, path, username, "projects", slug, "images", filename)

    title = models.CharField(max_length=255, blank=True, null=True,
        help_text=_('leave empty to populate with filename'))
    image = models.ImageField(upload_to=project_images_path, max_length=512)
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
        
    @property
    def filename(self):
        if self.image:
            filename = os.path.split(self.image.name)
            return filename[1]
        return None

class Attachment(models.Model):
    FRITZING_TYPE = 'fritzing'
    CODE_TYPE = 'code'
    EXAMPLE_TYPE = 'other_files'
    ATTACHMENT_TYPES = (
        (FRITZING_TYPE, _('fritzing file')),
        (CODE_TYPE, _('code')),
        (EXAMPLE_TYPE, _('other_files')),
    )
    
    def project_attachment_path(self, filename):
        slug = self.project.slug
        username = self.project.author.username
        path = "/".join(list(username[:3])) if len(username) >= 3 else ''
        return os.path.join(settings.USER_FILES_FOLDER, path, username, "projects", slug, self.kind, filename)


    title = models.CharField(_('title'), max_length=255, blank=True,
        null=True, help_text=_('Leave empty to populate with filename'))
    attachment = models.FileField(upload_to=project_attachment_path, max_length=512)
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
