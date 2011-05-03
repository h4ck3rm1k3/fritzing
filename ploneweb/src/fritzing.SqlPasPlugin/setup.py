from setuptools import setup, find_packages
import os

version = '0.2'

setup(name='fritzing.SqlPasPlugin',
      version=version,
      description="Customised version of pas.plugins.sqlalchemy v0.2",
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
        'MySQL-python',
        'SQLAlchemy',
        'Products.PlonePAS',
        'z3c.saconfig',
        'zope.sqlalchemy'
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
