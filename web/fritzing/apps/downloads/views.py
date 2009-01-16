import os
from django.http import HttpResponseRedirect
from django.shortcuts import get_object_or_404, get_list_or_404, render_to_response
from django.template import RequestContext

from downloads.models import Release, Download

def release_detail(request, version):
    releases = get_list_or_404(Release.objects.active(), version=version)
    return render_to_response('downloads/release_detail.html',
        {'releases': releases, 'version': version},
        context_instance=RequestContext(request))

def release_download(request, version, slug, filename):
    download = get_object_or_404(Download, platform__slug=slug,
        release__version=version, filename__iendswith=filename)
    if os.path.exists(download.filename.path):
        download.counter += 1
        download.save()
    return HttpResponseRedirect(download.filename.url)
