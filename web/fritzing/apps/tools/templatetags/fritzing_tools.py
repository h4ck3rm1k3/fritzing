from django.db.models import get_model
from django import template
from django.utils.text import truncate_html_words

from template_utils.nodes import GenericContentNode

register = template.Library()

class RelatedContentNode(GenericContentNode):
    """
    A subclass of GenericContentNode that performs a select on the related
    entities of the queryset.
    """
    def _get_query_set(self):
        return self.query_set.select_related()

class OrderedContentNode(RelatedContentNode):
    """
    A subclass of GenericContentNode that performs a select on the related
    entities of the queryset.
    """
    def __init__(self, model, num, order_by, varname):
        if ',' in order_by:
            order_by = order_by.split(',')
        if isinstance(order_by, basestring):
            order_by = [order_by]
        self.order_by = order_by
        super(OrderedContentNode, self).__init__(model, num, varname)

    def _get_query_set(self):
        qs = super(OrderedContentNode, self)._get_query_set()
        return qs.order_by(*self.order_by)

@register.tag('get_latest_objects_with_relatives')
def do_latest_objects_with_relatives(parser, token):
    """
    Retrieves the latest ``num`` objects from a given model, in that
    model's default ordering, while selecting and stores them in a context variable.

    Syntax::

        {% get_latest_objects_with_relatives [app_name].[model_name] [num] as [varname] %}

    Example::

        {% get_latest_objects_with_relatives comments.freecomment 5 order_by pub_date as latest_comments %}

    """
    bits = token.contents.split()
    if len(bits) not in (5, 7):
        raise template.TemplateSyntaxError("'%s' tag takes four or six arguments" % bits[0])
    if not bits[3] in ('as', 'order_by'):
        raise template.TemplateSyntaxError("third argument to '%s' tag must be 'as' or 'order_by'" % bits[0])
    if bits[3] == 'order_by':
        if bits[5] != 'as':
            raise template.TemplateSyntaxError("fifth argument to '%s' tag must be 'as'" % bits[0])
        return OrderedContentNode(bits[1], bits[2], bits[4], bits[6])
    return RelatedContentNode(bits[1], bits[2], bits[4])

@register.filter
def truncatehtml(string, length):
    return truncate_html_words(string, length)
truncatehtml.is_safe = True