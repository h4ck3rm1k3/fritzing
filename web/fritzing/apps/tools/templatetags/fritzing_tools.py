from django.db.models import get_model
from django import template
from django.utils.text import truncate_html_words

from template_utils.nodes import GenericContentNode

from datetime import *

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
    
class TimedObjectsNode(OrderedContentNode):
    today_or_now = {'today':date.today(), 'now':datetime.now()}
    
    def operators():
        return {'<':'lt', '<=':'lte', '=':'eq', '>=':'gte', '>':'gt'}
    operators = staticmethod(operators)
    
    def __init__(self, model, num, date_field, operator, relative_to, varname ):
        self.model, self.num, self.date_field, self.operator, self.relative_to, self.varname = \
            model, int(num), date_field, operator, relative_to, varname
        super(TimedObjectsNode, self).__init__(model, self.num, date_field, varname)

    def _get_query_set(self):
        qs = super(TimedObjectsNode, self)._get_query_set()

        date_object = self.today_or_now[self.relative_to]
        filter_string = eval("r'%s__%s'" % (self.date_field,self.operators()[self.operator]))
        kwargs = {filter_string: date_object}

        retval = qs.filter(**kwargs)[:self.num]
        return retval


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

@register.tag('get_latest_timed_objects')
def do_latest_timed_objects(parser, token):
    """
    Retrieves the latest ``num`` objects from a given model, filtering the query set
    by a date field, related to today or now objects
    

    Syntax::

        {% get_latest_timed_objects [app_name].[model_name] [num] [date_field] [comparison_operator] today|now as [varname] %}

    Example::

        {% get_latest_timed_objects comments.freecomment 5 pub_date >= today as latest_comments %}

    """
    bits = token.contents.split()
    if len(bits) != 8:
        raise template.TemplateSyntaxError("'%s' tag takes eight arguments" % bits[0])
    if bits[4] not in TimedObjectsNode.operators().keys():
        raise template.TemplateSyntaxError("forth argument tag must be an operator" % bits[0])
    if bits[5] not in ('today','now'):
        raise template.TemplateSyntaxError("fifth argument to '%s' tag must be 'today' or 'now'" % bits[0])
    if bits[6] != 'as':
        raise template.TemplateSyntaxError("sixth argument to '%s' tag must be 'as'" % bits[0])
    return TimedObjectsNode(bits[1], bits[2], bits[3], bits[4], bits[5], bits[7])
 

@register.filter
def truncatehtml(string, length):
    return truncate_html_words(string, length)
truncatehtml.is_safe = True
