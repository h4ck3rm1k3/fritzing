from django.db import models

class PublicProjectManager(models.Manager):
    """
    Custom manager for the Project model, providing shortcuts for
    filtering by entry status.
    """
    def featured(self):
        return self.filter(featured__exact=True)

    def get_query_set(self):
        return super(PublicProjectManager, self).get_query_set().filter(public__exact=True)

    def latest_featured(self):
        try:
            return self.featured()[0]
        except IndexError:
            return None
