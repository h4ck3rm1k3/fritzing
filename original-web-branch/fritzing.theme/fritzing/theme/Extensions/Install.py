# Product installation script -
# Use this instead of the extension profile directly, because otherwise
# we cannot install the dependent products
import transaction
from Products.CMFCore.utils import getToolByName

PRODUCT_DEPENDENCIES = ('NuPlone', )
EXTENSION_PROFILES = ('fritzing.theme:default',)

def install(self, reinstall=False):
	qi = getToolByName(self, 'portal_quickinstaller')
	setup = getToolByName(self, 'portal_setup')
	for product in PRODUCT_DEPENDENCIES:
		if reinstall and qi.isProductInstalled(product):
			qi.reinstallProducts([product])
			transaction.savepoint()
		elif not qi.isProductInstalled(product):
			qi.installProduct(product)
			transaction.savepoint()
			
	for extensionId in EXTENSION_PROFILES:
		setup.runAllImportStepsFromProfile(
			'profile-%s' % extensionId, purge_old=False)
		productName = extensionId.split(':')[0]
		qi.notifyInstalled(productName)
		transaction.savepoint()

