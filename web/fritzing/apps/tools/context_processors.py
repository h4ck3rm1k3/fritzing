from pages.models import Page

def pages(request):
    return {
        'all_pages': Page.objects.on_site(request.site)
    }
