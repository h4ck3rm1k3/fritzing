from django import forms
from fritzing.apps.fab.models import FabOrder, FabOrderAddress


class FabOrderForm(forms.ModelForm):
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
        
    def props(self):
        retval = {}
        for f1 in self.fields:
            print self.fields[f1].value
            #retval[f.label] = f.value
        return retval

    class Meta:
        model = FabOrderAddress
