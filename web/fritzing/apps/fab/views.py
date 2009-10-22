from django.shortcuts import render_to_response, get_object_or_404
from django.contrib.auth.decorators import login_required
from fritzing.apps.fab.forms import FabOrderForm, FabOrderAddress
from fritzing.apps.fab.models import *
from django.http import HttpResponseRedirect
from django.shortcuts import render_to_response
from django.template import RequestContext
from django.forms.models import inlineformset_factory


@login_required
def create(request, form_class=FabOrderForm):
    if request.method == 'POST':
        form = form_class(request.POST, request.FILES)
        #if project: return HttpResponseRedirect(project.get_absolute_url())
    else:
        form = form_class()
    shipping_address_formset = inlineformset_factory(FabOrderAddress, FabOrder, fk_name='shipping_address')
    billing_address_formset = inlineformset_factory(FabOrderAddress, FabOrder, fk_name='billing_address')
    return render_to_response("fab/fab_form.html", {
        'shipping_address_formset': shipping_address_formset,
        'billing_address_formset': billing_address_formset,
        'form': form,
    }, context_instance=RequestContext(request))

def load_options(manufacturer,opts_name,sections):
    for opt in manufacturer[opts_name].all():
        section = opt.section.text
        if not section in sections:
            sections[section] = {}
        if not opts_name in sections[section]:
            sections[section][opts_name] = []
        sections[section][opts_name].append(opt)

@login_required
def manufacturer_form(request, manufacturer_id):
    manufacturer = get_object_or_404(Manufacturer, pk=manufacturer_id)
    man_opts = {}
    man_opts['xor_options'] = manufacturer.xor_options
    man_opts['onoff_options'] = manufacturer.onoff_options
    man_opts['intvalue_options'] = manufacturer.intvalue_options
    
    sections = {}
    load_options(man_opts,'xor_options',sections)
    load_options(man_opts,'onoff_options',sections)
    load_options(man_opts,'intvalue_options',sections)
    
    sections_order = {}
    for s in OptionsSection.objects.all():
        sections_order[s.text] = s.order
    
    ordered_sections = sorted(sections.iteritems(), key=lambda (k,v): (sections_order[k],v))
    
    return render_to_response("fab/manufacturer_opts.html", {
        'sections': ordered_sections,
    }, context_instance=RequestContext(request))
    
    