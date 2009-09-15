import os
from django.db import models
from django.utils.translation import ugettext_lazy as _
from django_extensions.db.fields import AutoSlugField
from fritzing import settings

class TitleSlugDescriptionModel(models.Model):
    title = models.CharField(_('title'), max_length=255)
    slug = AutoSlugField(_('slug'), populate_from='title')
    description = models.TextField(_('description'), blank=True, null=True,
                        help_text=_('A short description of the object.'))

    class Meta:
        abstract = True

class AbstractAttachment(models.Model):
    """
    Provides a base class for attachments added to other entities.
    The functions 'attachment_path', should be overrided, in order to provide
    the path where the attachments will be saved
    A reference to the owner entity, user and kind of attachment may
    be good options for extending this class (take a look at the projects
    attachments, for an examples)
    """
    
    class Meta:
        abstract = True
        verbose_name = _('attachment')
    
    def attachment_path(self, filename):
        pass


    title = models.CharField(_('title'), max_length=255, blank=True,
        null=True, help_text=_('Leave empty to populate with filename'))
    attachment = models.FileField(upload_to=attachment_path, max_length=512)

    def __unicode__(self):
        return self.title

    def save(self, force_insert=False, force_update=False):
        if not self.pk and not self.title:
            self.title = self.attachment.name
        super(AbstractAttachment, self).save(force_insert, force_update)

    @property
    def filename(self):
        if self.attachment:
            filename = os.path.split(self.attachment.name)
            return filename[1]
        return None
