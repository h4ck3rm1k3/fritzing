
from south.db import db
from django.db import models
from django.utils.translation import ugettext_lazy as _
from downloads.models import *

class Migration:
    
    def forwards(self):
        
        # Model 'Platform'
        db.create_table('downloads_platform', (
            ('id', models.AutoField(verbose_name='ID', primary_key=True, auto_created=True)),
            ('name', models.CharField(_('name'), max_length=255)),
            ('slug', models.SlugField(_('slug'), max_length=255, unique=True)),
        ))
        # Model 'Release'
        db.create_table('downloads_release', (
            ('id', models.AutoField(verbose_name='ID', primary_key=True, auto_created=True)),
            ('version', models.CharField(_('version'), max_length=64, help_text=_("The version string used in the URL and on the download page. Please don't use spaces here."))),
            ('description', models.TextField(_('description'), blank=True)),
            ('changelog', models.TextField(_('changelog'), blank=True, help_text=_("The changelog displayed at http://fritzing.org/downloads/history-changes/. HTML is allowed."))),
            ('known_issues', models.TextField(_('known issues'), blank=True, help_text=_("The known issues displayed at http://fritzing.org/downloads/known-issues/. HTML is allowed."))),
            ('release_date', models.DateTimeField(_('date released'), default=datetime.now)),
            ('active', models.BooleanField(_('active'), default=True)),
        ))
        
        # Mock Models
        Release = db.mock_model(model_name='Release', db_table='downloads_release', db_tablespace='', pk_field_name='id', pk_field_type=models.AutoField, pk_field_args=[], pk_field_kwargs={})
        Platform = db.mock_model(model_name='Platform', db_table='downloads_platform', db_tablespace='', pk_field_name='id', pk_field_type=models.AutoField, pk_field_args=[], pk_field_kwargs={})
        
        # Model 'Download'
        db.create_table('downloads_download', (
            ('id', models.AutoField(verbose_name='ID', primary_key=True, auto_created=True)),
            ('release', models.ForeignKey(Release, verbose_name=_('release'))),
            ('platform', models.ForeignKey(Platform, verbose_name=_('platform'))),
            ('filename', models.FileField(_('file'), upload_to='downloads', blank=True, null=True)),
            ('mime_type', models.CharField(_('mime type'), max_length=255, blank=True, null=True)),
            ('counter', models.IntegerField(_('counter'), default=0)),
        ))
        
        db.send_create_signal('downloads', ['Platform','Release','Download'])
    
    def backwards(self):
        db.delete_table('downloads_download')
        db.delete_table('downloads_release')
        db.delete_table('downloads_platform')
        
