from fritzing.apps.tools.models import TitleSlugDescriptionModel, AbstractAttachment
from django.contrib.auth.models import User
from django.db import models
from django_extensions.db.models import TimeStampedModel
from django.utils.translation import ugettext_lazy as _
import os
from fritzing import settings

class FabOrder(TimeStampedModel):
    CHECKING = 1
    WAITING_APPROVAL = 2
    PRODUCING = 3
    SHIPPING = 4
    CANCELED = 5
    
    checking_pair = (CHECKING, "Checking")
    waiting_approval_pair = (WAITING_APPROVAL, "Waiting for Customer Approval")
    producing_pair = (PRODUCING, "Producing")
    shipping_pair = (SHIPPING, "Shipping")
    canceled_pair = (CANCELED, "Canceled")
    
    STATES = (
        checking_pair,
        waiting_approval_pair,
        producing_pair,
        shipping_pair,
        canceled_pair,
    )
    
    user = models.ForeignKey(User)
    user_email = models.EmailField(blank=True)
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
    contact_person = models.ForeignKey(User)
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

def faborder_attachment_path(order_id,filename):
    path = "orders"
    return os.path.join(settings.USER_FILES_FOLDER, path, str(order_id), filename)

# ATTACHMENTS
class GenericAttachment(models.Model):
    def attachment_path(self, filename):
        return faborder_attachment_path(self.order.pk,filename)

    title = models.CharField(_('title'), max_length=255, blank=True,
        null=True, help_text=_('Leave empty to populate with filename'))
    attachment = models.FileField(upload_to=attachment_path, max_length=512)
    
    def save(self, force_insert=False, force_update=False):
        if not self.pk and not self.title:
            self.title = self.attachment.name
        super(GenericAttachment, self).save(force_insert, force_update)

    @property
    def filename(self):
        if self.attachment:
            filename = os.path.split(self.attachment.name)
            return filename[1]
        return None
    
class FabOrderFritzingFileAttachment(GenericAttachment):
    order = models.OneToOneField(FabOrder,
        verbose_name=_('order'), related_name='fritzing_attachment')
    user = models.ForeignKey(User,
        blank=True, null=True, related_name='faborder_fritzing_attachment')
    
class FabOrderOtherAttachment(GenericAttachment):    
    order = models.ForeignKey(FabOrder,
        verbose_name=_('order'), related_name='other_attachments')
    user = models.ForeignKey(User,
        blank=True, null=True, related_name='faborder_other_attachments')

