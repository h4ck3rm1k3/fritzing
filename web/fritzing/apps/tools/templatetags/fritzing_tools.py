from django.db.models import get_model
from django import template
from template_utils.nodes import GenericContentNode

class RelatedContentNode(GenericContentNode):
    """
    A subclass of GenericContentNode that performs a select on the related
    entities of the queryset.
    """
    def _get_query_set(self):
        return self.query_set.select_related()

def do_latest_objects_with_relatives(parser, token):
    """
    Retrieves the latest ``num`` objects from a given model, in that
    model's default ordering, while selecting and stores them in a context variable.

    Syntax::

        {% get_latest_objects_with_relatives [app_name].[model_name] [num] as [varname] %}

    Example::

        {% get_latest_objects_with_relatives comments.freecomment 5 as latest_comments %}

    """
    bits = token.contents.split()
    if len(bits) != 5:
        raise template.TemplateSyntaxError("'%s' tag takes four arguments" % bits[0])
    if bits [3] != 'as':
        raise template.TemplateSyntaxError("third argument to '%s' tag must be 'as'" % bits[0])
    return RelatedContentNode(bits[1], bits[2], bits[4])

register = template.Library()
register.tag('get_latest_objects_with_relatives', do_latest_objects_with_relatives)
