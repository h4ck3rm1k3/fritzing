# -*- coding: utf-8 -*-
#
# File: model.py
#
# Copyright (c) InQuant GmbH
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

__author__    = """Stefan Eletzhofer <stefan.eletzhofer@inquant.de>"""
__docformat__ = 'plaintext'
__revision__  = "$Revision: 3823 $"
__version__   = '$Revision: 3823 $'[11:-2]

import random
import string
import hashlib
import datetime

from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import Table, Column, Integer, String, Boolean, DateTime, TIMESTAMP
from sqlalchemy import Text, Float, ForeignKey, Sequence
from sqlalchemy.orm import relationship
from sqlalchemy.ext.associationproxy import association_proxy

Base = declarative_base()

auth_user_groups = Table('auth_user_groups', Base.metadata,
    Column('user_id', Integer, ForeignKey('auth_user.id')),
    Column('group_id', Integer, ForeignKey('auth_group.id'))
)

# class Principal(Base):
#     __tablename__ = "principals"
#     
#     id = Column(Integer, Sequence("principals_id"), primary_key=True)
# 
# class RoleAssignment(Base):
#     __tablename__ = "role_assignments"
#     
#     id = Column(Integer, Sequence("role_assignment_id"), primary_key=True)
#     principal_id = Column(Integer, ForeignKey(Principal.id))
#     name = Column(String(64))
#     
#     def __init__(self, name):
#         self.name = name
#     
#     def __repr__(self):
#         return "<RoleAssignment id=%s principal_id=%d name=%s>" % (
#             self.id, self.principal_id, self.name)

class User(Base):
    __tablename__ = "auth_user"
    
    id = Column(
        Integer,
        # ForeignKey(Principal.id),
        # Sequence("principals_id"),
        primary_key=True)
    
    name = Column('username', String(30), unique=True)
    # firstname = Column('first_name', String(30), default=u"")
    # lastname = Column('last_name', String(30), default=u"")
    email = Column(String(75), default=u"")
    password = Column(String(128))
    is_staff = Column(Boolean(), default=0)
    is_active = Column(Boolean(), default=1)
    is_superuser = Column(Boolean(), default=0)
    last_login_time = Column('last_login', DateTime)
    date_created = Column('date_joined', DateTime)
    
    # roles
    # _roles =  relation(
    #     RoleAssignment, collection_class=set, cascade="all, delete, delete-orphan")
    # roles = association_proxy("_roles", "name")
    
    def __init__(self, username=None):
        self.username = username
        self.date_created = datetime.datetime.now()
    
    def generate_salt(self):
        return ''.join(random.sample(string.letters, 5))
    
    def encrypt(self, password, salt):
        return hashlib.sha1(salt+password).hexdigest()
    
    def set_password(self, password):
        salt = self.generate_salt()
        passhash = self.encrypt(password, salt)
        self.password = "sha1$%s$%s" % (salt, passhash)
    
    def check_password(self, password):
        (algorithm, salt, passhash) = self.password.split('$')
        return self.encrypt(password, salt) == passhash
    
    def __repr__(self):
        return "<User id=%d username=%s>" % (
            self.id, self.username)

class Group(Base):
    __tablename__ = "auth_group"
    
    id = Column(
        Integer,
        # ForeignKey(Principal.id),
        # Sequence("principals_id"),
        primary_key=True)
    
    name = Column(String(80), unique=True)
    users = relationship(User, secondary=auth_user_groups, backref="groups")
    
    # _roles =  relation(
    #     RoleAssignment, collection_class=set, cascade="all, delete, delete-orphan")
    # roles = association_proxy("_roles", "name")
    
    def __init__(self, name=None):
        self.name = name
    
    def __repr__(self):
        return "<Group id=%d name=%s>" % (self.id, self.name)

# vim: set ft=python ts=4 sw=4 expandtab :
