from imagekit.specs import ImageSpec
from imagekit import processors
    
class HeaderThumbnail(processors.Resize):
    width = 400
    height = 160
    crop = True

class SidebarThumbnail(processors.Resize):
    width = 100
    height = 75
    crop = True

class EnhanceSmall(processors.Adjustment):
    contrast = 1.2
    sharpness = 1.1

class Header(ImageSpec):
    pre_cache = True
    quality = 100
    access_as = 'header'
    processors = [HeaderThumbnail, EnhanceSmall]

class Sidebar(ImageSpec):
    pre_cache = True
    quality = 100
    access_as = 'sidebar'
    processors = [SidebarThumbnail, EnhanceSmall]
