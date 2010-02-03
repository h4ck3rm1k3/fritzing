from django.conf.urls.defaults import patterns, url 
from fritzing.apps.externals.views import index

urlpatterns = patterns('',
    url(r'^$', index, name='fab-index'),
)