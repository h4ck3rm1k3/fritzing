from django import forms
from fritzing.apps.fab.models import FabOrder, FabOrderAddress, Manufacturer
from django.utils.translation import ugettext_lazy as _

class FabOrderForm(forms.ModelForm):
    manufacturer = forms.ModelChoiceField(queryset=Manufacturer.objects.all(),label=_("Company"))

    class Meta:
        model = FabOrder
        fields= (
            'manufacturer',
        )

class FabOrderAddressForm(forms.ModelForm):
    def __init__(self, *args, **kw):
        super(forms.ModelForm, self).__init__(*args, **kw)
        self.fields.keyOrder = [
            'first_name',
            'last_name',
            'company',
            'street',
            'city',
            'state',
            'country'
        ]

    class Meta:
        model = FabOrderAddress
