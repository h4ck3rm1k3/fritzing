from setuptools import setup, find_packages
import os

version = '0.1'

setup(name='fritzing.fab',
      version=version,
      description="Fritzing Fab, a pcb production service",
      long_description=open("README.txt").read() + "\n" +
                       open(os.path.join("docs", "HISTORY.txt")).read(),
      # Get more strings from
      # http://pypi.python.org/pypi?%3Aaction=list_classifiers
      classifiers=[
        "Framework :: Plone",
        "Programming Language :: Python",
        ],
      keywords='',
      author='Fritzing',
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
          'plone.app.z3cform',
          'plone.directives.form',
          'plone.namedfile',
          'plone.namedfile[blobs]',
      ],
      extras_require={
          'tests': ['collective.testcaselayer',]
      },
      entry_points="""
      # -*- Entry points: -*-

      [z3c.autoinclude.plugin]
      target = plone
      """,
      )
