from django.shortcuts import render_to_response, get_object_or_404
from django.contrib.auth.decorators import login_required
from fritzing.apps.fab.forms import FabOrderForm, FabOrderAddressForm
from fritzing.apps.fab.models import *
from django.http import HttpResponseRedirect
from django.shortcuts import render_to_response
from django.template import RequestContext
from django.core.urlresolvers import reverse
from apps.projects.views import detail
import os

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

def _populate_options(order, manufacturer, post):
    choosen_onoff = []
    for name,value in post.items():
        if name.startswith('xor'):
            opt_pk = name.replace('xor-','')
            option = XOrOption.objects.get(pk=int(opt_pk))
            choice = XOrOptionChoice.objects.get(pk=int(int(value)))
            
            instance = FabOrderXOrOption(order=order,option=option,choice=choice)
            instance.save()
        elif name.startswith('onoff'):
            opt_pk = int(name.replace('onoff-',''))
            option = OnOffOption.objects.get(pk=opt_pk)
            
            instance = FabOrderOnOffOption(order=order,option=option,onoff=True)
            instance.save()
            
            choosen_onoff.append(opt_pk)
            
        elif name.startswith('intvalue'):
            opt_pk = name.replace('intvalue-','')
            option = IntegerValueOption.objects.get(pk=int(opt_pk))
            
            instance = FabOrderIntegerValueOption(order=order,option=option,value=int(value))
            instance.save()
    
    for onoff_opt in manufacturer.onoff_options.all():
        if onoff_opt.pk not in choosen_onoff:
            option = OnOffOption.objects.get(pk=onoff_opt.pk)
            
            instance = FabOrderOnOffOption(order=order,option=option,onoff=False)
            instance.save()

def _place_attachments(request,order,field_name,attach_class):
    for attachment_file in request.FILES.getlist(field_name):
        if attachment_file.name:
            at = attach_class(
                attachment=attachment_file.name,
                order=order,
                user=request.user)
            at.attachment.save(attachment_file.name, attachment_file)
            at.save()

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
            state=FabOrder.CHECKING,
            comments=request.POST['comments']
        )
        order.save()

        _populate_options(order, manufacturer, request.POST)
        
        _place_attachments(request,order,'fritz_file',FabOrderFritzingFileAttachment)
        _place_attachments(request,order,'other_files[]',FabOrderOtherAttachment)

        _send_creation_email(order)
        
        return details(request,order.pk,True)
    else:
        form = form_class()
    shipping_address_form = FabOrderAddressForm()
    billing_address_form = FabOrderAddressForm()
    
    return render_to_response("fab/fab_form.html", {
        'shipping_address_form': shipping_address_form,
        'billing_address_form': billing_address_form,
        'form': form,
    }, context_instance=RequestContext(request))

def _load_manuf_options(manufacturer,opts_name,sections):
    for opt in manufacturer[opts_name].all():
        section = opt.section
        if not section in sections:
            sections[section] = {}
        if not opts_name in sections[section]:
            sections[section][opts_name] = []
        sections[section][opts_name].append(opt)
        
def _load_fab_options(order,opts_name,sections):
    for opt in order[opts_name].all():
        section = opt.option.section
        if not section in sections:
            sections[section] = {}
        if not opts_name in sections[section]:
            sections[section][opts_name] = []
        sections[section][opts_name].append(opt)
        
def _sort_sections(sections):
    sections_order = {}
    for s in OptionsSection.objects.all():
        sections_order[s] = s.order
        
    return sorted(sections.iteritems(), key=lambda (k,v): (sections_order[k],v))

def _get_sections(entity_with_option, load_func):
    opts = {}
    opts['xor_options'] = entity_with_option.xor_options
    opts['onoff_options'] = entity_with_option.onoff_options
    opts['intvalue_options'] = entity_with_option.intvalue_options
    
    sections = {}
    load_func(opts,'xor_options',sections)
    load_func(opts,'onoff_options',sections)
    load_func(opts,'intvalue_options',sections)
    
    return _sort_sections(sections)

@login_required
def manufacturer_form(request, manufacturer_id):
    manufacturer = get_object_or_404(Manufacturer, pk=manufacturer_id)   
    ordered_sections = _get_sections(manufacturer,_load_manuf_options)
    
    return render_to_response("fab/manufacturer_opts.html", {
        'sections': ordered_sections,
        'manufacturer': manufacturer,
    }, context_instance=RequestContext(request))
    
def _label_for_state(order_state):
    for value,label in FabOrder.STATES:
        if value == order_state:
            return label

@login_required
def details(request, order_id, show_thanks_msg=False):
    order = get_object_or_404(FabOrder,pk=order_id)
    is_customer = order.user == request.user
    is_manufacturer = order.manufacturer.contact_person == request.user
    
    shipping_address_form = FabOrderAddressForm(instance=order.shipping_address)
    billing_address_form = FabOrderAddressForm(instance=order.billing_address)
    
    ordered_sections = _get_sections(order, _load_fab_options)
    
    curr_state = {}
    curr_state['value'] = order.state
    curr_state['label'] = _label_for_state(order.state)
    
    next_state = {} if order.state < FabOrder.SHIPPING else None
    if next_state == {}:
        next_state['value'] = order.state+1
        next_state['label'] = _label_for_state(order.state+1)
        
    cancel_state = {} if order.state < FabOrder.CANCELED else None
    if cancel_state == {}:
        cancel_state['value'] = FabOrder.CANCELED
        cancel_state['label'] = _label_for_state(FabOrder.CANCELED)
    
    return render_to_response("fab/details.html", {
        'order': order,
        'is_customer': is_customer,
        'is_manufacturer': is_manufacturer,
        'shipping_address_form': shipping_address_form,
        'billing_address_form': billing_address_form,
        'sections': ordered_sections,
        'curr_state': curr_state,
        'next_state': next_state,
        'cancel_state': cancel_state,
        'show_thanks_msg': show_thanks_msg,
    }, context_instance=RequestContext(request))
    
@login_required
def state_change(request):
    if request.POST:
        order_id = request.POST['order_id']
        state = request.POST['state']
        
        order = FabOrder.objects.get(pk=order_id)
        if request.user == order.manufacturer.contact_person:
            order.state = state
            order.save()
            # TODO: SEND EMAIL TO THE CUSTOMER
        
        return HttpResponseRedirect(reverse('faborder-details', args=[order.pk]))
    else:
        return HttpResponseRedirect(reverse('faborder-create'))


from Cheetah.Template import Template
from django.core.mail import send_mail

def _get_creation_mail_body(order):
    shipping_address_form = FabOrderAddressForm(instance=order.shipping_address)
    billing_address_form = FabOrderAddressForm(instance=order.billing_address)    
    ordered_sections = _get_sections(order, _load_fab_options)
    curr_state = _label_for_state(order.state)
    
    data = {
        'order' : order,
        'shipping_address_form': shipping_address_form,
        'billing_address_form': billing_address_form,
        'sections': ordered_sections,
        'curr_state': curr_state
    }
    
    path = os.path.join(os.path.dirname(__file__),"templates")
    page = Template(file=path+'/faborder_creation_email.tmpl', searchList=[data])
    return str(page)

def _send_creation_email(order):
    subject = "New Fritzing Fab Order #(%s)" % order.pk
    body = _get_creation_mail_body(order)
    to_email = order.manufacturer.email
    from_email = order.user_email
    
    print """
    sending email from %(from)s to %(to)s
    """ % {'from':from_email, 'to': to_email}
    
    return send_mail(subject, body, from_email, [to_email], fail_silently=False)
