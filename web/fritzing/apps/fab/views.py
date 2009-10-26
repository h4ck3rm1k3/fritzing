from django.shortcuts import render_to_response, get_object_or_404
from django.contrib.auth.decorators import login_required
from fritzing.apps.fab.forms import FabOrderForm, FabOrderAddressForm
from fritzing.apps.fab.models import *
from django.http import HttpResponseRedirect
from django.shortcuts import render_to_response
from django.template import RequestContext
from django.core.urlresolvers import reverse

def _populate_address(prefix, post):
    props = {}
    for name,value in post.items():
        if name.startswith(prefix):
            name = name.replace(prefix+'-','')
            props[name] = value
            
    address = FabOrderAddress(
        # I guess this can be automatically done, but I don't know how
        street=props['street'], 
        city=props['city'],
        state=props['state'],
        country=props['country'],
        first_name=props['first_name'],
        last_name=props['last_name'],
        company=props['company'] 
    )
    
    address.save()
    return address

def _populate_options(order, post):
    for name,value in post.items():
        if name.startswith('xor'):
            opt_pk = name.replace('xor-','')
            option = XOrOption.objects.get(pk=int(opt_pk))
            choice = XOrOptionChoice.objects.get(pk=int(int(value)))
            
            instance = FabOrderXOrOption(order=order,option=option,choice=choice)
            instance.save()
        elif name.startswith('onoff'):
            opt_pk = name.replace('onoff-','')
            option = OnOfOption.objects.get(pk=int(opt_pk))
            
            instance = FabOrderOnOffOption(order=order,option=option,onoff=True)
            instance.save()
        elif name.startswith('intvalue'):
            opt_pk = name.replace('intvalue-','')
            option = IntegerValueOption.objects.get(pk=int(opt_pk))
            
            instance = FabOrderIntegerValueOption(order=order,option=option,value=int(value))
            instance.save()


@login_required
def create(request, form_class=FabOrderForm):
    if request.method == 'POST':
        manufacturer = Manufacturer.objects.get(
            pk=int(request.POST['manufacturer'])
        )

        s_address = _populate_address("shipping",request.POST)
        b_address = _populate_address("billing",request.POST)
        order = FabOrder(
            shipping_address=s_address,
            billing_address=b_address,
            manufacturer=manufacturer,
            user=request.user,
            user_email=request.POST['email'],
            state=FabOrder.CHECKING
        )
        order.save()

        _populate_options(order, request.POST)
        
        return HttpResponseRedirect(reverse('faborder-details', args=[order.pk]))
    else:
        form = form_class()
    shipping_address_form = FabOrderAddressForm()
    billing_address_form = FabOrderAddressForm()
    
    return render_to_response("fab/fab_form.html", {
        'shipping_address_form': shipping_address_form,
        'billing_address_form': billing_address_form,
        'form': form,
    }, context_instance=RequestContext(request))


def load_options(manufacturer,opts_name,sections):
    for opt in manufacturer[opts_name].all():
        section = opt.section
        if not section in sections:
            sections[section] = {}
        if not opts_name in sections[section]:
            sections[section][opts_name] = []
        sections[section][opts_name].append(opt)

@login_required
def manufacturer_form(request, manufacturer_id):
    #if not request.user.email:
    #    return HttpResponseRedirect("")
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
        sections_order[s] = s.order
    
    ordered_sections = sorted(sections.iteritems(), key=lambda (k,v): (sections_order[k],v))
    
    return render_to_response("fab/manufacturer_opts.html", {
        'sections': ordered_sections,
    }, context_instance=RequestContext(request))

@login_required
def details(request,order_id):
    order = get_object_or_404(FabOrder,pk=order_id)
    is_customer = order.user == request.user
    is_manufacturer = order.manufacturer.contanct_person == request.user
    
    shipping_address_form = FabOrderAddressForm(instance=order.shipping_address)
    billing_address_form = FabOrderAddressForm(instance=order.billing_address)
    
    return render_to_response("fab/details.html", {
        'order': order,
        'is_customer': is_customer,
        'is_manufacturer': is_manufacturer,
        'shipping_address_form': shipping_address_form,
        'billing_address_form': billing_address_form,        
    }, context_instance=RequestContext(request))
    
