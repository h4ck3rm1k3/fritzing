from django.conf import settings
from django.shortcuts import render_to_response, get_object_or_404
from django.template import RequestContext
from django.contrib.auth.models import User
from django.contrib.auth.decorators import login_required
from django.http import HttpResponseRedirect
from django.core.urlresolvers import reverse
from forum.models import Post

from fritzing.apps.profiles.models import Profile
from fritzing.apps.profiles.forms import ProfileForm
from fritzing.apps.projects.models import Project

if "notification" in settings.INSTALLED_APPS:
    from notification import models as notification
else:
    notification = None

def profiles(request, template_name="profiles/profiles.html"):
    return render_to_response(template_name, {
        "users": User.objects.all().order_by("username"),
    }, context_instance=RequestContext(request))

def profile(request, username, template_name="profiles/profile.html", project_count=3, post_count=5):
    other_user = get_object_or_404(User, username=username)
    if request.user.is_authenticated():
        if request.user == other_user:
            is_me = True
        else:
            is_me = False
    else:
        is_me = False
        
    projects = Project.published.filter(author=other_user.id)[0:project_count]
    forum_entries = Post.objects.filter(author=other_user.id).order_by('-time')[0:post_count]
    
    if is_me:
        if request.method == "POST":
            if request.POST["action"] == "update":
                profile_form = ProfileForm(request.POST, request.FILES, instance=other_user.get_profile())
                if profile_form.is_valid():
                    profile = profile_form.save(commit=False)
                                        
                    other_user.first_name = profile_form.cleaned_data['user_first_name']
                    other_user.last_name = profile_form.cleaned_data['user_last_name']
                    other_user.email = profile_form.cleaned_data['user_email']
                    other_user.save()
                    
                    profile.user = other_user
                    profile.save()
            else:
                profile_form = ProfileForm(instance=other_user.get_profile())
        else:
            profile_form = ProfileForm(instance=other_user.get_profile())
    else:
        profile_form = None

    return render_to_response(template_name, {
        "profile_form": profile_form,
        "is_me": is_me,
        "other_user": other_user,
        "projects" : projects,
        "forum_entries" : forum_entries,
    }, context_instance=RequestContext(request))

@login_required
def edit(request, username, template_name="profiles/edit.html"):
    other_user = get_object_or_404(User, username=username)
    if request.user.is_authenticated():
        if request.user == other_user:
            is_me = True
        else:
            is_me = False
    else:
        is_me = False
        
    to_detail = False
    if is_me:
        if request.method == "POST":
            if request.POST["action"] == "update":
                profile_form = ProfileForm(request.POST, request.FILES, instance=other_user.get_profile())
                if profile_form.is_valid():
                    profile = profile_form.save(commit=False)
                                        
                    other_user.first_name = profile_form.cleaned_data['user_first_name']
                    other_user.last_name = profile_form.cleaned_data['user_last_name']
                    other_user.email = profile_form.cleaned_data['user_email']
                    other_user.save()
                    
                    profile.user = other_user
                    profile.save()
                    
                    to_detail = True
            else:
                profile_form = ProfileForm(instance=other_user.get_profile())
        else:
            profile_form = ProfileForm(instance=other_user.get_profile())
    else:
        to_detail = True
        profile_form = None

    if to_detail:
        return HttpResponseRedirect(reverse('profile_detail', args=[username]))
    else:
        return render_to_response(template_name, {
            "profile_form": profile_form,
            "is_me": is_me,
            "other_user": other_user,
        }, context_instance=RequestContext(request))

