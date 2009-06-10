from django.shortcuts import render_to_response, get_object_or_404
from django.template.context import RequestContext
from datetime import date
from dregni import views

from fritzing.apps.events.models import Event

def overview(request, num_latest=-1, template_name='events/overview.html', extra_context={}):
    """Show the <num_latest> latest events ordered by start_date"""
    if(num_latest > 0):
        event_list = Event.objects.all().order_by('-start_date','-start_time')[:num_latest]
    else:
        event_list = Event.objects.all().order_by('-start_date','-start_time')
    template_context = {
        'event_list': event_list,
    }
    template_context.update(extra_context)
    return render_to_response(template_name, template_context,
                              RequestContext(request))

def details(request, slug, template_name='events/details.html', extra_context={}):
    """Shows a details page for the given event"""
    event = get_object_or_404(Event, slug=slug)
    template_context = {
        'event': event,
        'is_detail': True
    }
    template_context.update(extra_context)    
    return render_to_response(template_name, template_context, 
                              RequestContext(request))

def icalendar(request):
    return views.icalendar(request, Event.objects.all())