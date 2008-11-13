def setupVarious(context):
    """Import steps that are not handled by GS import/export handlers can be
    defined in the setupVarious() function.
    See Products.GenericSetup.context.BaseContext to see what you can do with
    ``context`` (the function argument).
    For instance, it is possible to get the Plone Site object:
    ``site = context.getSite()``
    """
    site = context.getSite()

    # Put your own import stuff here