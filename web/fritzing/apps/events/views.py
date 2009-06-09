from django.shortcuts import render_to_response, get_object_or_404
from django.template.context import RequestContext
from datetime import date
from dregni import views

from fritzing.apps.events.models import Event

def overview(request, num_latest=10, template_name='events/overview.html', extra_context={}):
    """Show the 10 latest events"""
    event_list = Event.objects.filter(start_date__gte=date.today())[:num_latest]
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