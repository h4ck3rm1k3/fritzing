from pages.models import Page

def pages(request):
    return {
        'all_pages': Page.objects.navigation(request.site).order_by("tree_id")
    }
