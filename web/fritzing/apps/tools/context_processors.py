from pages.models import Page

def pages(request):
    """
    Return all pages that are in the first and second navigation level
    ordered after the tree_id.
    """
    qs = Page.objects.navigation(request.site).filter(
        level__lte='1').order_by('tree_id')
    return {'all_pages': qs}
