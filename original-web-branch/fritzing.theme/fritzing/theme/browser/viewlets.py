from Products.Five.browser.pagetemplatefile import ViewPageTemplateFile
from plone.app.layout.viewlets.common import ViewletBase


class FooterViewlet(ViewletBase):
    render = ViewPageTemplateFile('footer.pt')

    def update(self):
        self.foo = 'bar'
        

class ColophonViewlet(ViewletBase):
    render = ViewPageTemplateFile('colophon.pt')

    def update(self):
        self.foo = 'bar'