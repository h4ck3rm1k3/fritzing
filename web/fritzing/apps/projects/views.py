import datetime
from django.views.generic.list_detail import object_detail, object_list
from django.conf import settings
from django.shortcuts import render_to_response
from django.http import HttpResponseRedirect
from django.template import RequestContext
from django.contrib.auth.decorators import login_required
from django.views.decorators.cache import cache_page
from django.shortcuts import get_object_or_404, get_list_or_404

from fritzing.apps.projects.models import Project, Category, Image, Attachment
from fritzing.apps.projects.forms import ProjectForm, RESOURCE_DELIMITER

def overview(request):
    projects = Project.published.all()
    return render_to_response("projects/project_list.html", {
        'projects': projects,
    }, context_instance=RequestContext(request))
    
if not settings.DEBUG:
    overview = cache_page(overview, 60*15)

@login_required
def create(request, form_class=ProjectForm):
    if request.method == 'POST':
        form = form_class(request.POST, request.FILES)
        if form.is_valid():
            project = form.save(commit=False)
            project.author = request.user
            project.save()
            attachement_types = (
                ('fritzing_files', Attachment.FRITZING_TYPE),
                ('examples', Attachment.EXAMPLE_TYPE),
                ('code', Attachment.CODE_TYPE),
            )
            for attachement_type in attachement_types:
                field_name, field_type = attachement_type
                for attachment_file in request.FILES.getlist(field_name):
                    Attachment.objects.create(
                        attachment=attachment_file.name,
                        project=project,
                        user=request.user,
                        kind=field_type)

            main_image = request.FILES.get('main_image')
            if main_image:
                # im is a not saved Image model instance
                im = Image(
                    project=project,
                    user=request.user,
                    is_heading=True)
                # saving the image field directly
                im.image.save(main_image.name, main_image)
                # saving the Image model instance
                im.save()

            for resource in request.POST.getlist('resources'):
                title, url = resource.split(RESOURCE_DELIMITER)
                Resource.objects.create(title=title, url=url, project=project)
            # save the project instance
            project.save()
            return HttpResponseRedirect(project.get_absolute_url())
    else:
        form = form_class()
    #print dir(form['title'])
    return render_to_response("projects/project_form.html", {
        'form': form,
    }, context_instance=RequestContext(request))

def detail(request, slug):
    project = get_object_or_404(Project.published.all(), slug=slug)

    return render_to_response("projects/project_detail.html", {
        'project': project,
    }, context_instance=RequestContext(request))
    
def by_user(request, user_id):
    projects = Project.published.filter(author=user_id)
    #projects = Project.published.all()
    return render_to_response("projects/projects_for_user.html", {
        'projects': projects,
    }, context_instance=RequestContext(request))
    
if not settings.DEBUG:
    overview = cache_page(overview, 60*15)
