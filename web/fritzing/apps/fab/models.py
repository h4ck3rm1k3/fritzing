from fritzing.apps.tools.models import TitleSlugDescriptionModel, AbstractAttachment
from django.contrib.auth.models import User
from django.db import models
from django_extensions.db.models import TimeStampedModel
from django.utils.translation import ugettext_lazy as _

class FabOrder(TimeStampedModel):
    CHECKING = 1
    WAITING_APPROVAL = 2
    PRODUCING = 3
    SHIPPING = 4
    STATES = (
        (CHECKING, "Checking"),
        (WAITING_APPROVAL, "Waiting for Customer Approval"),
        (PRODUCING, "Producing"),
        (SHIPPING, "Shipping"),
    )
    user = models.ForeignKey(User)
    shipping_address = models.ForeignKey('FabOrderAddress',related_name='orders_shipping')
    billing_address = models.ForeignKey('FabOrderAddress',related_name='orders_billing')
    manufacturer = models.ForeignKey('Manufacturer',related_name='orders')
    state = models.IntegerField(_('state'), choices=STATES)
    comments = models.TextField(blank=True,null=True, max_length=1000)
    
class Address(models.Model):
    street = models.CharField(_('street'), max_length=255)
    city = models.CharField(_('city'), max_length=255)
    state = models.CharField(_('state'), max_length=255, blank=True, null=True)
    country = models.CharField(_('country'), max_length=255)
    
    def __unicode__(self):
        return " %(street)s (%(country)s)" \
                % {'street' : self.street, 'country' : self.country}

    
class FabOrderAddress(Address):
    first_name = models.CharField(_('first name'), max_length=255)
    last_name = models.CharField(_('last name'), max_length=255)
    company = models.CharField(_('company'), max_length=255, blank=True, null=True)
    
    
class Manufacturer(TitleSlugDescriptionModel):
    location = models.CharField(_('location'), max_length=255)
    email = models.EmailField(_('email'))
    contanct_person = models.ForeignKey(User)
    phone_number = models.PositiveIntegerField(_('phone number'))
    address = models.OneToOneField(Address, verbose_name=_('address'))
    account_info = models.TextField(
        _('account info'),
        max_length=1000,
        help_text=_('IBAN and BIC or paypal info')
    )
    xor_options = models.ManyToManyField(
        'XOrOption',
        related_name='manufacturers',
        verbose_name=_('option with choices'),
        blank=True, null=True
    )
    onoff_options = models.ManyToManyField(
        'OnOffOption',
        related_name='manufacturers',
        verbose_name=_('boolean options'),
        blank=True, null=True
    )
    intvalue_options = models.ManyToManyField(
        'IntegerValueOption',
        related_name='manufacturers',
        verbose_name=_('integer input options'),
        blank=True, null=True
    )
   
    def __unicode__(self):
        return self.title 

# OPTIONS
class TextModel(models.Model):
    text = models.CharField(
        max_length=255,
        #help_text=_('Provide some hint text')
    )
    
    class Meta:
        abstract = True
        
    def __unicode__(self):
        return self.text
        
class Option(TextModel):
    section = models.ForeignKey('OptionsSection')
    
    class Meta:
        abstract = True
    
class OptionsSection(TextModel):
    help_text = models.CharField(
        max_length=255,
        blank=True,
        null=True
    )
    order = models.IntegerField()

class XOrOptionChoice(TextModel):
    owner_option = models.ForeignKey('XOrOption', related_name='choices')

class XOrOption(Option):
    class Meta:
        verbose_name = _('option with choices')
    #price = models.DecimalField(
    #    _('price'),
    #    blank=True,
    #    null=True,
    #    decimal_places=2,
    #    max_digits=5
    #)
    #porcentage = models.DecimalField(
    #    _('porcentage'),
    #    blank=True,
    #    null=True,
    #    decimal_places=2,
    #    max_digits=2
    #)
    
class OnOffOption(Option):
    class Meta:
        verbose_name = _('boolean option')
        
class IntegerValueOption(Option):
    default_value = models.IntegerField(blank=True, null=True)
    
    class Meta:
        verbose_name = _('integer input option')
    
class FabOrderXOrOption(models.Model):
    order = models.ForeignKey(FabOrder, related_name='xor_options')
    option = models.ForeignKey(XOrOption)
    choice = models.ForeignKey(XOrOptionChoice)
    
class FabOrderOnOffOption(models.Model):
    order = models.ForeignKey(FabOrder, related_name='onoff_options')
    option = models.ForeignKey(OnOffOption)
    onoff = models.BooleanField()
    
class FabOrderIntegerValueOption(models.Model):
    order = models.ForeignKey(FabOrder, related_name='intvalue_options')
    option = models.ForeignKey(IntegerValueOption)
    value = models.IntegerField()

def faborder_attachment_path(filename):
    pass

# ATTACHMENTS
class FritzingFileAttachment():
    order = models.OneToOneField(FabOrder,
        verbose_name=_('order'), related_name='fritzing_attachment')
    
    def attachment_path(self, filename):
        return faborder_attachment_path(filename)

class OtherAttachment():
    order = models.ForeignKey(FabOrder,
        verbose_name=_('order'), related_name='other_attachments')
    
    def attachment_path(self, filename):
        return faborder_attachment_path(filename)
