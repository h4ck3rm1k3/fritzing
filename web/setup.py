from setuptools import setup, find_packages

version = '0.1'

setup(name='fritzing.theme',
      version=version,
      description="An installable theme for Plone 3.0",
      long_description="""\
""",
      # Get more strings from http://www.python.org/pypi?%3Aaction=list_classifiers
      classifiers=[
        "Framework :: Plone",
        "Framework :: Zope2",
        "Framework :: Zope3",
        "Programming Language :: Python",
        "Topic :: Software Development :: Libraries :: Python Modules",
        ],
      keywords='fritzing theme',
      author='Andr\x82 Kn\x94rig',
      author_email='info@fritzing.org',
      url='http://fritzing.org',
      license='GPL',
      packages=find_packages(exclude=['ez_setup']),
      namespace_packages=['fritzing'],
      include_package_data=True,
      zip_safe=False,
      install_requires=[
          'setuptools',
          # -*- Extra requirements: -*-
      ],
      entry_points="""
      # -*- Entry points: -*-
      """,
      )
