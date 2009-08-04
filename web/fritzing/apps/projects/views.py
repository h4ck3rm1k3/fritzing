from django.forms.util import ValidationError 
import datetime
from django.views.generic.list_detail import object_detail, object_list
from django.conf import settings
from django.shortcuts import render_to_response
from django.http import HttpResponseRedirect
from django.template import RequestContext
from django.contrib.auth.decorators import login_required
from django.views.decorators.cache import cache_page
from django.shortcuts import get_object_or_404, get_list_or_404

from fritzing.apps.projects.models import Project, Resource, Category, Image, Attachment
from fritzing.apps.projects.forms import ProjectForm, ResourceField, RESOURCE_DELIMITER

def overview(request,username=None):
    if username:
        projects = Project.published.filter(author__username=username)
    else:
        projects = Project.published.all()
    
    return render_to_response("projects/project_list.html", {
        'projects': projects,
        'by_user': username
    }, context_instance=RequestContext(request))
    
if not settings.DEBUG:
    overview = cache_page(overview, 60*15)
    
def _manage_attachments_saving(project_id,form, field):
    removed = [x.replace(settings.MEDIA_URL,'') for x in form.removed(field)]
    if removed:
        for at in Attachment.objects.filter(project__id=project_id,attachment__in=removed):
            at.delete()
        
def _manage_images_saving(project_id,form, field):
    removed = [x.replace(settings.MEDIA_URL,'') for x in form.removed(field)]
    if removed:
        for im in Image.objects.filter(project__id=project_id,image__in=removed):
            im.delete()
    
def _manage_deleted_files(project_id,form):
    _manage_attachments_saving(project_id,form,'fritzing_files')
    _manage_images_saving(project_id,form,'main_image')
    _manage_attachments_saving(project_id,form,'code')
    _manage_attachments_saving(project_id,form,'examples')
    _manage_images_saving(project_id,form,'other_images')
    
@login_required
def _manage_save(request, form, project_id=None, edition=False):
    if form.is_valid():
        project = form.save(commit=False)
        project.author = request.user
        
        if project_id:
            project.id = project_id
            project.slug = Project.objects.filter(id=project_id)[0].slug
        
        project.save()

        attachment_types = (
            ('fritzing_files', Attachment.FRITZING_TYPE),
            ('examples', Attachment.EXAMPLE_TYPE),
            ('code', Attachment.CODE_TYPE),
        )
        for attachment_type in attachment_types:
            field_name, field_type = attachment_type
            for attachment_file in request.FILES.getlist(field_name):
                if attachment_file.name:
                    at = Attachment(
                        attachment=attachment_file.name,
                        project=project,
                        user=request.user,
                        kind=field_type)
                    at.attachment.save(attachment_file.name, attachment_file)
                    at.save()

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
            
        for oimage in request.FILES.getlist('other_images'):
            # im is a not saved Image model instance
            oim = Image(
                project=project,
                user=request.user,
                is_heading=False)
            oim.image.save(oimage.name, oimage)
            oim.save()

        project.resources = []
        

        for resource in Resource.objects.filter(project__id=project_id):
            resource.delete()
        
        for resource in form.resources:
            title, url = resource
            Resource.objects.create(title=title, url=url, project=project)
            
        if edition:
            _manage_deleted_files(project.id,form)
            
        # save the project instance
        project.save()
        
        return project
    return None

@login_required
def create(request, form_class=ProjectForm):
    if request.method == 'POST':
        form = form_class(request.POST, request.FILES)
        project = _manage_save(request, form)
        if project: return HttpResponseRedirect(project.get_absolute_url())
    else:
        form = form_class()
    #print dir(form['title'])
    return render_to_response("projects/project_form.html", {
        'form': form,
    }, context_instance=RequestContext(request))
    
@login_required
def edit(request, project_id, form_class=ProjectForm):
    if request.method == 'POST':
        form = form_class(request.POST, request.FILES)
        project = _manage_save(request, form, project_id, edition=True)
        print project
        if project: return HttpResponseRedirect(project.get_absolute_url())
    else:
        project = Project.objects.get(id=project_id)
        form = form_class(instance=project)
        
    fritzing_attachments = Attachment.objects.filter(project__id=project_id, kind=Attachment.FRITZING_TYPE)
    code_attachments = Attachment.objects.filter(project__id=project_id, kind=Attachment.CODE_TYPE)
    examples_attachments = Attachment.objects.filter(project__id=project_id, kind=Attachment.EXAMPLE_TYPE)
    main_image_aux = Image.objects.filter(project__id=project_id, is_heading=True)
    main_image = main_image_aux[0] if main_image_aux.count() >= 1 else None;
    other_images_attachements = Image.objects.filter(project__id=project_id, is_heading=False)
    resources = list(Resource.objects.filter(project__id=project_id))
    
    return render_to_response("projects/project_form.html", {
        'form': form,
        'code_attachments': code_attachments,
        'fritzing_attachments': fritzing_attachments,
        'main_image': main_image,
        'examples_attachments': examples_attachments,
        'other_images_attachements': other_images_attachements,
        'resources': resources,
    }, context_instance=RequestContext(request))

def detail(request, slug):
    project = get_object_or_404(Project.published.all(), slug=slug)
    if request.user.is_authenticated():
        if request.user == project.author:
            is_me = True
        else:
            is_me = False
    else:
        is_me = False
        
    license_aux = project.license
    license = ""
    if license_aux.logo:
        license = """
        <a target="_blank" href="%(link)s" alt="%(name)s">
        <img src="%(logo)s" alt="%(name)s" title="%(name)s"/>
        </a>
        """ % {'link':license_aux.url,'name':license_aux.name,'logo':license_aux.logo}
    else:
        license = """
        <a target="_blank" href="%(link)s" alt="%(name)s">%(name)s</a>
        """ % {'link':license_aux.url,'name':license_aux.name}
        

    return render_to_response("projects/project_detail.html", {
        'project': project,
        'is_me': is_me,
        'license':license
    }, context_instance=RequestContext(request))
    
if not settings.DEBUG:
    overview = cache_page(overview, 60*15)
