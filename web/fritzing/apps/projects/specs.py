from imagekit.specs import ImageSpec
from imagekit import processors
    
class HeaderThumbnail(processors.Resize):
    width = 400

class SidebarThumbnail(processors.Resize):
    width = 100
    height = 75

class OverviewThumbnail(processors.Resize):
    width = 150

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

class Overview(ImageSpec):
    pre_cache = True
    quality = 100
    access_as = 'overview'
    processors = [OverviewThumbnail, EnhanceSmall]
