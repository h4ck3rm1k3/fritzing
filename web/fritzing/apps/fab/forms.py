from django import forms
from fritzing.apps.fab.models import FabOrder


class FabOrderForm(forms.ModelForm):

    class Meta:
        model = FabOrder

