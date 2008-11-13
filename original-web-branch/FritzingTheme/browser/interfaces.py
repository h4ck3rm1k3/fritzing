from zope.publisher.interfaces.browser import IDefaultBrowserLayer

class IThemeSpecific(IDefaultBrowserLayer):
    """Marker interface that defines a Zope 3 skin layer bound to a Skin
       Selection in portal_skins.
       If you need to register a viewlet only for the "Fritzing"
       skin, this is the interface that must be used for the layer attribute
       in FritzingTheme/browser/configure.zcml.
    """