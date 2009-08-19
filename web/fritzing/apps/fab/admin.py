from django.contrib import admin
from fritzing.apps.fab.models import *


class AddressAdmin(admin.ModelAdmin):
    pass
admin.site.register(Address, AddressAdmin)

class OptionsSectionAdmin(admin.ModelAdmin):
    pass
admin.site.register(OptionsSection, OptionsSectionAdmin)

class XOrOptionChoiceInline(admin.StackedInline):
    model = XOrOptionChoice
    
class XOrOptionAdmin(admin.ModelAdmin):
    inlines = [
        XOrOptionChoiceInline
    ]
    
admin.site.register(XOrOption, XOrOptionAdmin)

class OnOffOptionAdmin(admin.ModelAdmin):
    pass
admin.site.register(OnOffOption, OnOffOptionAdmin)

class IntegerValueOptionAdmin(admin.ModelAdmin):
    pass
admin.site.register(IntegerValueOption, OnOffOptionAdmin)

class ManufacturerAdmin(admin.ModelAdmin):
    pass
admin.site.register(Manufacturer, ManufacturerAdmin)

