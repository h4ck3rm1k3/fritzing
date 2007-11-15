Description

    FritzingTheme is a product that adds a new style to a Plone 3.0.x portal.
    It adds a new skin selection to the 'portal_skins' tool
    (called Fritzing), and registers a custom stylesheet (called 
    fritzing.css) with the 'portal_css' tool.

    FritzingTheme is based on DIYPloneStyle 3.0, a skeleton product
    ready for building new graphical designs for Plone.

Installation

    On the file system: place FritzingTheme in the Products directory
    of your Zope instance and restart the server.

    In the Plone Web Interface: as portal manager, go to 'Portal > Site Setup
    > Add-on Products'.
    Select 'DIYPloneStyle' and click the *Install* button.

    Uninstall -- Can be done from the same page.

Selecting a skin

    Depending on the values given in the skins tool profile (see
    profiles/default/skins.xml), the skin will be selected (or not) as default
    one while installing the product. If you need to switch from a default
    skin to another, go to the 'Site Setup' page, and choose 'Skins' (as
    portal manager). You can also decide from that page if members can choose
    their preferred skin and, in that case, if the skin cookie should be
    persistent.

    Note -- Don't forget to perform a full refresh of the page or reload all
    images (not from browser cache) after selecting a skin. In Firefox, you
    can do so by pressing the 'shift' key while reloading the page. In IE, use
    the key combination <Ctrl-F5>.

Written by

    John Doe <john.doe@dev.null>
