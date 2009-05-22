import datetime
from django.views.generic.list_detail import object_detail, object_list
from django.shortcuts import render_to_response
from django.http import HttpResponseRedirect
from django.template import RequestContext
from django.contrib.auth.decorators import login_required
from fritzing.apps.projects.models import Project, Category, Image, Attachment
from fritzing.apps.projects.forms import ProjectForm, RESOURCE_DELIMITER

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
            print request.user
            for attachement_type in attachement_types:
                field_name, field_type = attachement_type
                for attachment_file in request.FILES.getlist(field_name):
                    Attachment.objects.create(
                        attachment=attachment_file.name,
                        project=project,
                        user=request.user,
                        kind=field_type)

            if 'main_image' in request.FILES:
                Image.objects.create(
                    image=request.FILES['main_image'],
                    project=project,
                    user=request.user,
                    is_heading=True)

            for resource in request.POST.getlist('resources'):
                title, url = resource.split(RESOURCE_DELIMITER)
                Resource.objects.create(title=title, url=url, project=project)
            project.save()
            return HttpResponseRedirect(project.get_absolute_url())
    else:
        form = form_class()
    #print dir(form['title'])
    return render_to_response("projects/project_form.html", {
        'form': form
    }, context_instance = RequestContext(request))

