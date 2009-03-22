from django.db import models
from django.utils.translation import ugettext_lazy as _
from django_extensions.db.fields import AutoSlugField

class TitleSlugDescriptionModel(models.Model):
    title = models.CharField(_('title'), max_length=255)
    slug = AutoSlugField(_('slug'), populate_from='title')
    description = models.TextField(_('description'), blank=True, null=True,
                        help_text=_('A short description of the object.'))

    class Meta:
        abstract = True
