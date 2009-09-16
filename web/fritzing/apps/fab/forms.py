from django import forms
from fritzing.apps.fab.models import FabOrder, FabOrderAddress


class FabOrderForm(forms.ModelForm):
    class Meta:
        model = FabOrder

class FabOrderAddressForm(forms.ModelForm):
    class Meta:
        model = FabOrderAddress