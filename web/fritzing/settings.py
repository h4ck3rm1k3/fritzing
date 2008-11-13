# -*- coding: utf-8 -*-
import os

DEBUG = True
TEMPLATE_DEBUG = DEBUG

ADMINS = (
    ('Jannis Leidel', 'jannis@leidel.info'),
)

MANAGERS = ADMINS

DATABASE_ENGINE = 'sqlite3'           # 'postgresql_psycopg2', 'postgresql', 'mysql', 'sqlite3' or 'ado_mssql'.
DATABASE_NAME = 'dev.db'             # Or path to database file if using sqlite3.
DATABASE_USER = ''             # Not used with sqlite3.
DATABASE_PASSWORD = ''         # Not used with sqlite3.
DATABASE_HOST = ''             # Set to empty string for localhost. Not used with sqlite3.
DATABASE_PORT = ''             # Set to empty string for default. Not used with sqlite3.

# Local time zone for this installation. Choices can be found here:
# http://www.postgresql.org/docs/8.1/static/datetime-keywords.html#DATETIME-TIMEZONE-SET-TABLE
# although not all variations may be possible on all operating systems.
# If running in a Windows environment this must be set to the same as your
# system time zone.
TIME_ZONE = 'Europe/Berlin'

# Language code for this installation. All choices can be found here:
# http://www.w3.org/TR/REC-html40/struct/dirlang.html#langcodes
# http://blogs.law.harvard.edu/tech/stories/storyReader$15
LANGUAGE_CODE = 'en'

SITE_ID = 1

# If you set this to False, Django will make some optimizations so as not
# to load the internationalization machinery.
USE_I18N = True

# Absolute path to the directory that holds media.
# Example: "/home/media/media.lawrence.com/"

MEDIA_ROOT = os.path.join(os.path.dirname(__file__), "media")

# URL that handles the media served from MEDIA_ROOT.
# Example: "http://media.lawrence.com"
MEDIA_URL = '/media/'

# URL prefix for admin media -- CSS, JavaScript and images. Make sure to use a
# trailing slash.
# Examples: "http://foo.com/media/", "/media/".
ADMIN_MEDIA_PREFIX = '/media/admin/'

# Make this unique, and don't share it with anybody.
SECRET_KEY = 'bk-e2zv3humar79nm=j*bwc=-ymeit(8a20whp3goq4dh71t)s'

# List of callables that know how to import templates from various sources.
TEMPLATE_LOADERS = (
    'django.template.loaders.filesystem.load_template_source',
    'django.template.loaders.app_directories.load_template_source',
    'dbtemplates.loader.load_template_source',
)

MIDDLEWARE_CLASSES = (
    'django.middleware.common.CommonMiddleware',
    'django.contrib.sessions.middleware.SessionMiddleware',
    'django.contrib.auth.middleware.AuthenticationMiddleware',
    'django.middleware.locale.LocaleMiddleware',
    'pagination.middleware.PaginationMiddleware',
    'django.middleware.transaction.TransactionMiddleware',
    'django.contrib.redirects.middleware.RedirectFallbackMiddleware',
)

ROOT_URLCONF = 'fritzing.urls'

TEMPLATE_DIRS = (
    os.path.join(os.path.dirname(__file__), "../apps/debug_toolbar/templates"),
    os.path.join(os.path.dirname(__file__), "templates"),
)

TEMPLATE_CONTEXT_PROCESSORS = (
    "django.core.context_processors.auth",
    "django.core.context_processors.debug",
    "django.core.context_processors.i18n",
    "django.core.context_processors.media",
    "django.core.context_processors.request",
    "pages.context_processors.media",
)

INSTALLED_APPS = (
    'django.contrib.auth',
    'django.contrib.contenttypes',
    'django.contrib.comments',
    'django.contrib.sessions',
    'django.contrib.sites',
    'django.contrib.humanize',
    'django.contrib.markup',
    'django.contrib.admin',
    'django.contrib.redirects',
    'django_extensions',

    'django_extensions',
    'dbtemplates',
    'reversion',
    'pagination',
    'pages',
    'template_utils',
    'tagging',
    'mptt',
    'mptt_comments',
    'filebrowser',
    'wsgi',
    'partslib',
)

EMAIL_CONFIRMATION_DAYS = 2
EMAIL_DEBUG = DEBUG
CONTACT_EMAIL = "jannis@leidel.info"

ugettext = lambda s: s
LANGUAGES = (
  ('en', u'English'),
  ('de', u'Deutsch'),
)

CACHE_BACKEND = "locmem:///?max_entries=3000"

FORCE_LOWERCASE_TAGS = True

WIKI_REQUIRES_LOGIN = True

FILEBROWSER_URL_FILEBROWSER_MEDIA = MEDIA_URL + '/filebrowser/'
FILEBROWSER_PATH_FILEBROWSER_MEDIA = MEDIA_ROOT + 'filebrowser/'
FILEBROWSER_EXTENSIONS = {
    'Folder':[''],
    'Image':['.jpg', '.jpeg', '.gif','.png','.tif','.tiff'],
    'Video':['.mov','.wmv','.mpeg','.mpg','.avi','.rm'],
    'Document':['.pdf','.doc','.rtf','.txt','.xls','.csv'],
    'Sound':['.mp3','.mp4','.wav','.aiff','.midi'],
    'Code':['.html','.py','.js','.css'],
    'Achives':['.tgz','.tar', '.zip', '.rar', '.dmg', '.gz', '.bz2'],
}

INTERNAL_IPS = ('127.0.0.1',)

DEFAULT_PAGE_TEMPLATE = 'fritzing/docs/index.html'
PAGE_TEMPLATES = (
    ('fritzing/docs/index.html', 'documentation index'),
    ('fritzing/docs/detail.html', 'documentation detail'),
    ('fritzing/docs/overview.html', 'documentation overview'),
)

PROJECT_ROOT = os.path.abspath(os.path.dirname(__file__))
PAGE_PERMISSION = False
PAGE_UNIQUE_SLUG_REQUIRED = True

try:
    from local_settings import *
except ImportError:
    pass
