from django.shortcuts import render_to_response, get_object_or_404
from django.template.context import RequestContext
from ticker.models import Entry 

def details(request, slug, template_name='ticker/details.html', extra_context={}):
    """Shows a details page for the given entry"""
    entry = get_object_or_404(Entry.objects.public(), slug=slug)
    template_context = {
        'entry': entry,
        'is_detail': True
    }
    template_context.update(extra_context)    
    return render_to_response(template_name, template_context, 
                              RequestContext(request))