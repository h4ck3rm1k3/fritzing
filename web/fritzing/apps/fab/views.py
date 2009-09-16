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

