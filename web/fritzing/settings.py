# -*- coding: utf-8 -*-
# Django settings for content pinax project.

import os
import pinax

PINAX_ROOT = os.path.abspath(os.path.dirname(pinax.__file__))
PROJECT_ROOT = os.path.abspath(os.path.dirname(__file__))

# tells Pinax to use the default theme
PINAX_THEME = 'default'

DEBUG = True
TEMPLATE_DEBUG = DEBUG

# tells Pinax to serve media through django.views.static.serve.
SERVE_MEDIA = DEBUG

ADMINS = (
    ('Andre Knoerig', 'andre.knoerig@gmail.com'),
    ('Mariano Crowe', 'merunga@gmail.com'),
    ('Jonathan Cohen', 'cohen@irascible.com'),
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

MEDIA_ROOT = os.path.join(PROJECT_ROOT, "media")

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
    'django_openid.consumer.SessionConsumer',
    'account.middleware.LocaleMiddleware',
    'pagination.middleware.PaginationMiddleware',
    'django.middleware.transaction.TransactionMiddleware',
    'django.contrib.redirects.middleware.RedirectFallbackMiddleware',
    'tools.middleware.UserBasedExceptionMiddleware',
)

ROOT_URLCONF = 'fritzing.urls'

TEMPLATE_DIRS = (
    os.path.join(PROJECT_ROOT, "../apps/debug_toolbar/templates"),
    os.path.join(PROJECT_ROOT, "templates"),
    os.path.join(PINAX_ROOT, "templates", PINAX_THEME),
)

TEMPLATE_CONTEXT_PROCESSORS = (
    "django.core.context_processors.auth",
    "django.core.context_processors.debug",
    "django.core.context_processors.i18n",
    "django.core.context_processors.media",
    "django.core.context_processors.request",
    
    "notification.context_processors.notification",
    "announcements.context_processors.site_wide_announcements",
#    "account.context_processors.openid",
    "account.context_processors.account",
    "misc.context_processors.contact_email",
    "misc.context_processors.site_name",
    "pages.context_processors.media",
    "tools.context_processors.pages",
)

INSTALLED_APPS = (
    # included
    'django.contrib.auth',
    'django.contrib.contenttypes',
    'django.contrib.sessions',
    'django.contrib.sites',
    'django.contrib.humanize',
    'django.contrib.markup',
    'django.contrib.admin',
    'django.contrib.redirects',
    'django.contrib.admin',
    'django.contrib.comments',

    # external
    'django_extensions',
    'notification', # must be first
    'django_openid',
    'emailconfirmation',
    'mailer',
    'announcements',
    'pagination',
    'timezones',
    'ajax_validation',
    'dbtemplates',
    'reversion',
    'tagging',
    'mptt',
    'tagging',
    'pages',
    'template_utils',
    'filebrowser',
    'emailthis',
    'forum',
    'treemenus',
    'robots',
    'contact_form',
    'ticker',
    'tinymce',
    'markitup',
    #'south',
    'licenses',
    'dregni',
    # internal (for now)
    #'basic_profiles',
    'uni_form',
    'account',
    'misc',
    'imagekit',
    'fritzing.apps.*',
    'fritzing.apps.profiles',
)

ABSOLUTE_URL_OVERRIDES = {
    "auth.user": lambda o: "/profiles/%s/" % o.username,
}

AUTH_PROFILE_MODULE = 'profiles.Profile'
NOTIFICATION_LANGUAGE_MODULE = 'account.Account'

EMAIL_CONFIRMATION_DAYS = 2
EMAIL_DEBUG = DEBUG
CONTACT_EMAIL = "info@fritzing.org"
SITE_NAME = "Fritzing"
LOGIN_URL = "/account/login"
LOGIN_REDIRECT_URL = "/"
EMAIL_SUBJECT_PREFIX = "[Fritzing] "

ugettext = lambda s: s
LANGUAGES = (
  ('en', u'English'),
  ('de', u'Deutsch'),
)

CACHE_BACKEND = "locmem:///?max_entries=3000"

FORCE_LOWERCASE_TAGS = True

WIKI_REQUIRES_LOGIN = True

FILE_UPLOAD_PERMISSIONS = 0644

FILEBROWSER_URL_FILEBROWSER_MEDIA = MEDIA_URL + '/filebrowser/'
FILEBROWSER_PATH_FILEBROWSER_MEDIA = MEDIA_ROOT + 'filebrowser/'
FILEBROWSER_EXTENSIONS = {
    'Folder':[''],
    'Image':['.jpg', '.jpeg', '.gif','.png','.tif','.tiff','.svg'],
    'Video':['.mov','.wmv','.mpeg','.mpg','.avi','.rm'],
    'Document':['.pdf','.doc','.rtf','.txt','.xls','.csv'],
    'Sound':['.mp3','.mp4','.wav','.aiff','.midi'],
    'Code':['.html','.py','.js','.css', '.pde'],
    'Archives':['.tgz','.tar', '.zip', '.rar', '.dmg', '.gz', '.bz2'],
    'Fritzing':['.fz', '.fzb', '.fzp','.fzpz' , '.ts'],
}
FILEBROWSER_STRICT_PIL = True

INTERNAL_IPS = ('127.0.0.1',)

DEFAULT_PAGE_TEMPLATE = 'pages/docs/index.html'
PAGE_TEMPLATES = (
    ('pages/single-col.html', 'one column'),
    ('pages/two-cols.html', 'two columns'),
    ('pages/two-cols-header.html', 'one header, two columns'),
)

PAGE_PERMISSION = False
PAGE_TAGGING = True
PAGE_USE_SITE_ID = True
REQUIRE_LOGIN_PATH = LOGIN_REDIRECT_URL

TINYMCE_JS_URL = '%stinymce/tiny_mce.js' % MEDIA_URL
TINYMCE_JS_ROOT = os.path.join(MEDIA_ROOT, 'tinymce')
TINYMCE_DEFAULT_CONFIG = {
    'plugins': "table,spellchecker,paste,searchreplace",
    'theme': "advanced",
    'apply_source_formatting' : 'true',
    'inline_styles' : 'true',
    'dialog_type' : "window",
    'theme_advanced_buttons1': 'pastetext,pasteword,cleanup,|,bold,italic,|,forecolor,|,justifyleft,justifycenter,justifyright,|,indent,outdent,|,bullist,numlist,|,link,unlink,image,charmap,|,undo,redo,|,code,',
    'theme_advanced_buttons2': '',
    'theme_advanced_buttons3': '',
    'theme_advanced_resizing' : 'true',
    'theme_advanced_resize_horizontal' : 'true',
    'theme_advanced_toolbar_location' : "top",
    'theme_advanced_statusbar_location' : "bottom"
}
TINYMCE_SPELLCHECKER = True

FIXTURE_DIRS = (
    os.path.join(PROJECT_ROOT, "fixtures"),
)

GENERIC_CONTENT_LOOKUP_KWARGS = {
    'ticker.entry': { 'status': 3 },
}

MARKUP_FILTER = ('markdown', {'safe_mode': True})
MARKITUP_SET = 'markitup/sets/markdown'
MARKITUP_PREVIEW_FILTER = ('markdown.markdown', {'safe_mode': True})

EVENT_URL_FORMAT = ''
VCSTORAGE_DEFAULT_BACKEND = 'git'

USER_FILES_FOLDER = "fritzing-repo"

GRAILS_SERVER = "http://192.168.2.172:8082"

RECAPTCHA_PUB_KEY = "6LcaaccSAAAAADOm22SykxPQ43AciVF-cQqytrB7"
RECAPTCHA_PRIVATE_KEY = "6LcaaccSAAAAAArF6fAjPH-GmjHMmZSaaWnyWcoT"

# local_settings.py can be used to override environment-specific settings
# like database and email that differ between development and production.
try:
    from local_settings import *
except ImportError:
    pass
