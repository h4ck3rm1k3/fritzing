from django.conf.urls.defaults import *
from django.conf import settings

from account.openid_consumer import PinaxConsumer

from django.contrib import admin
admin.autodiscover()

urlpatterns = patterns('',
    (r'^account/', include('account.urls')),
    (r'^openid/(.*)', PinaxConsumer()),
    (r'^profiles/', include('basic_profiles.urls')),
    (r'^notices/', include('notification.urls')),
    (r'^announcements/', include('announcements.urls')),
    (r'^admin/filebrowser/', include('filebrowser.urls')),
    (r'^admin/(.*)', admin.site.root),
    (r'^i18n/', include('django.conf.urls.i18n')),
    (r'^robots.txt$', include('robots.urls')),
    (r'^emailthis/', include('emailthis.urls')),
    (r'^contact/', include('tools.urls.contact'),
    (r'^forum/', include('forum.urls')),
    (r'^faq/', include('faq.urls')),
    (r'^admin/(.*)', admin.site.root),
    url(r'^faq/$', 'faq.views.faq_list', name='faq_list'),
    url(r'^faq/(?P<slug>[\w]+)/$', 'faq.views.question_detail', name='faq_detail'),
    (r'', include('pages.urls')),
)

if settings.SERVE_MEDIA:
    urlpatterns += patterns('django.views.static',
        (r'^%s(?P<path>.*)$' % settings.MEDIA_URL[1:], 'serve', {
            'document_root': settings.MEDIA_ROOT,
        }),
    )
