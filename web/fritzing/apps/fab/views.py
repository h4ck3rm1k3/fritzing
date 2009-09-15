from django.contrib.auth.decorators import login_required
from fritzing.apps.fab.forms import FabOrderForm
from django.http import HttpResponseRedirect
from django.shortcuts import render_to_response
from django.template import RequestContext

@login_required
def create(request, form_class=FabOrderForm):
    if request.method == 'POST':
        form = form_class(request.POST, request.FILES)
        #if project: return HttpResponseRedirect(project.get_absolute_url())
    else:
        form = form_class()
    #print dir(form['title'])
    return render_to_response("fab/fab_form.html", {
        'form': form,
    }, context_instance=RequestContext(request))
