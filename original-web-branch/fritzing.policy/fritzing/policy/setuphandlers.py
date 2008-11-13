from Products.CMFCore.utils import getToolByName

def setupGroups(portal):
	acl_users = getToolByName(portal, 'acl_users')
	if not acl_users.searchGroups(name='core'):
		portal_groups = getToolByName(portal, 'portal_groups')
		portal_groups.addGroup('core', roles=['Manager'])

def importVarious(context):
	"""Miscellaneous steps import handle
	"""
	if context.readDataFile('fritzing.policy_various.txt') is None:
		return
	portal = context.getSite()
	setupGroups(portal)