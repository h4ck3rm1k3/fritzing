#
# Skeleton FritzingThemeTestCase
#

from Products.PloneTestCase import PloneTestCase

PloneTestCase.installProduct('FritzingTheme')
PloneTestCase.setupPloneSite(products=['FritzingTheme'])


class TestSomething(PloneTestCase.PloneTestCase):


    def afterSetUp(self):
        pass

    def testSomething(self):
        # Test something
        self.assertEqual(1+1, 2)


def test_suite():
    from unittest import TestSuite, makeSuite
    suite = TestSuite()
    suite.addTest(makeSuite(TestSomething))
    return suite
