
from south.db import db
from django.db import models
from django.utils.translation import ugettext_lazy as _
from downloads.models import *

INTERIM = 1
MAIN = 2
TYPE_CHOICES = (
    (INTERIM, _('interim')),
    (MAIN, _('main'))
)

class Migration:
    
    def forwards(self):
        
        # Model 'Release'
        db.create_table('downloads_release', (
            ('id', models.AutoField(verbose_name='ID', primary_key=True, auto_created=True)),
            ('version', models.CharField(_('version'), max_length=64, help_text=_("The version string used in the URL and on the download page. Please don't use spaces here."))),
            ('description', models.TextField(_('description'), blank=True)),
            ('changelog', models.TextField(_('changelog'), blank=True, help_text=_("The changelog displayed at http://fritzing.org/downloads/history-changes/. HTML is allowed."))),
            ('known_issues', models.TextField(_('known issues'), blank=True, help_text=_("The known issues displayed at http://fritzing.org/downloads/known-issues/. HTML is allowed."))),
            ('release_date', models.DateTimeField(_('date released'), default=datetime.now)),
            ('active', models.BooleanField(_('active'), default=True)),
            ('type', models.IntegerField(_('type'), choices=TYPE_CHOICES, default=INTERIM, help_text=_('Depending on the "use it at your own risk" level.'))),
        ))
        
        db.send_create_signal('downloads', ['Release'])
    
    def backwards(self):
        db.delete_table('downloads_release')
        
