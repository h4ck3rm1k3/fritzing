from django import template
from django.conf import settings

from downloads.models import Platform, Release

register = template.Library()

HISTORY_LEN = getattr(settings, 'CODEHOSTING_HISTORY_LENGTH', 10)

@register.inclusion_tag('downloads/platform_list_tag.html')
def platform_list():
    """Show a list of the platforms
    """
    return {'platforms': Platform.objects.all().order_by('name')}

@register.inclusion_tag('downloads/release_history.html')
def release_history():
    """Show a list of the active releases for a platform
    """
    releases = Release.objects.main()
    return {'releases': releases[0:HISTORY_LEN]}
