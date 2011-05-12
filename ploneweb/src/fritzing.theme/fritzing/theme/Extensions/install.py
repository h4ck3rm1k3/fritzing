from Products.CMFCore.utils import getToolByName

def uninstall(portal, reinstall=False):
    """ Uninstall this product.
        
        This external method is need, because portal_quickinstaller doesn't
        know what GenericProfile profile to apply when uninstalling a product.
    """
    setup_tool = getToolByName(portal, 'portal_setup')
    if reinstall:
        return "Ran all reinstall steps."
    else:
        setup_tool.runAllImportStepsFromProfile('profile-fritzing.theme:uninstall')
        return "Ran all uninstall steps."