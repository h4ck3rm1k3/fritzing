import urllib

from zope.app.component.hooks import getSite

from Products.CMFCore.utils import getToolByName
from Products.CMFCore import permissions

from email.MIMEText import MIMEText
from email.Utils import formataddr
from smtplib import SMTPRecipientsRefused

from fritzing.fab.interfaces import IFabOrder


def canDelete(self, item):
    portal_membership = getToolByName(item, 'portal_membership')
    if portal_membership.checkPermission(permissions.DeleteObjects, item) \
        and portal_membership.checkPermission(permissions.DeleteObjects, item.aq_parent):
        return True
    return False

def isStateId(self, item, state_id):
    return getStateId(self, item) == state_id


def getStateId(self, item, portal_workflow = None):
    """
    invoke like this:
    getStateId(None, item) or getStateId(None, item, portal_workflow)
    first argument isn't used, pass whatever you like
    """
    if portal_workflow == None:
        portal_workflow = getToolByName(item, 'portal_workflow')
    state_id = portal_workflow.getInfoFor(item, 'review_state')
    return state_id


def getStateTitle(self, item, portal_workflow = None):
    """
    invoke like this:
    getStateTitle(None, item) or getStateTitle(None, item, portal_workflow)
    first argument isn't used, pass whatever you like
    """
    if portal_workflow == None:
        portal_workflow = getToolByName(item, 'portal_workflow')
    state_id = getStateId(self, item, portal_workflow)
    state_title = portal_workflow.getTitleForStateOnType(state_id, item.portal_type)
    return state_title


def encodeFilename(self, filename):
    if filename is None:
        return None
    else:
        if isinstance(filename, unicode):
            filename = filename.encode('utf-8')
        return urllib.quote_plus(filename)


def recalculatePrices(faborder):
    faborder.priceNetto = faborder.area * faborder.pricePerSquareCm
    faborder.priceQualityChecksNetto = faborder.numberOfQualityChecks * 10.0
    faborder.priceTotalNetto = faborder.priceNetto + faborder.priceQualityChecksNetto
    
    # shipping and taxes
    faborders = faborder.aq_parent
    faborder.priceShipping = faborders.shippingWorld
    faborder.taxesPercent = faborders.taxesWorld
    if faborder.shipTo == u'germany':
        faborder.priceShipping = faborders.shippingGermany
        faborder.taxesPercent = faborders.taxesGermany
    elif faborder.shipTo == u'eu':
        faborder.priceShipping = faborders.shippingEU
        faborder.taxesPercent = faborders.taxesEU
    
    faborder.taxes = faborder.priceTotalNetto * faborder.taxesPercent / 100.0
    faborder.priceTotalBrutto = faborder.priceTotalNetto + faborder.taxes + faborder.priceShipping


def sendStatusMail(context):
    """Sends notification on the order status to the orderer and faborders.salesEmail
    """
    mail_text = u""
    charset = 'utf-8'
    
    portal = getSite()
    mail_template = portal.mail_order_status_change
    faborders = context.aq_parent
    
    from_address = faborders.salesEmail
    from_name = "Fritzing Fab"
    user  = context.getOwner()
    to_address = user.getProperty('email')
    to_name = user.getProperty('fullname')
    
    mail_text = mail_template(
        to_name = to_name,
        state_id = getStateId(False, context),
        state_title = getStateTitle(False, context),
        faborder = context,
        ship_to = IFabOrder['shipTo'].vocabulary.getTerm(context.shipTo).title,
        )
    
    try:
        host = getToolByName(context, 'MailHost')
        # send our copy:
        host.secureSend(
            message = MIMEText(mail_text, 'plain', charset), 
            mfrom = formataddr((from_name, from_address)),
            mto = formataddr((from_name, from_address)),
            charset = charset,
        )
        # send notification for the orderer:
        host.secureSend(
            message = MIMEText(mail_text, 'plain', charset), 
            mfrom = formataddr((from_name, from_address)),
            mto = formataddr((to_name, to_address)),
            charset = charset,
        )
    except SMTPRecipientsRefused:
        # Don't disclose email address on failure
        raise SMTPRecipientsRefused('Recipient address rejected by server')
