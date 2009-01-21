import os
from django.http import HttpResponseRedirect
from django.shortcuts import get_object_or_404, get_list_or_404, render_to_response
from django.template import RequestContext
from django.views.generic import list_detail

from downloads.models import Release, Download

def release_list(request):
    latest_main = Release.objects.main().latest()
    interim_releases = Release.objects.interim().filter(
        release_date__gte=latest_main.release_date)
    return render_to_response('downloads/release_list.html', {
        'latest_release': latest_main,
        'interim_releases': interim_releases,
    }, context_instance=RequestContext(request))

def release_detail(request, version):
    releases = get_list_or_404(Release.objects.active(), version=version)
    return render_to_response('downloads/release_detail.html', {
        'releases': releases,
        'version': version
    }, context_instance=RequestContext(request))

def release_download(request, version, slug, filename):
    download = get_object_or_404(Download, platform__slug=slug,
        release__version=version, filename__iendswith=filename)
    if os.path.exists(download.filename.path):
        download.counter += 1
        download.save()
    return HttpResponseRedirect(download.filename.url)
