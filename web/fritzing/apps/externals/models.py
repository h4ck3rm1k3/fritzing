from django.db import models
import uuid
import os
import random

# borrowed code from http://code.activestate.com/recipes/576980/
# and http://www.codekoala.com/blog/2009/aes-encryption-python-using-pycrypto/

from Crypto.Cipher import AES
from Crypto.Util import randpool
import base64

def encrypt_string(plain_text, key_string):
   block_size = 16
   mode = AES.MODE_CBC
   plain_data = base64.urlsafe_b64decode(plain_text)
   pad = block_size - len(plain_data) % block_size
   print "pad = " + str(pad)
   data = plain_data + (pad * chr(pad))
   iv_bytes = randpool.RandomPool(512).get_bytes(block_size)
   key_bytes = base64.urlsafe_b64decode(key_string)
   encrypted_bytes = iv_bytes + AES.new(key_bytes, mode, iv_bytes).encrypt(data)
   encrypted_string = base64.urlsafe_b64encode(str(encrypted_bytes))
   return encrypted_string
   
def generate_key_string():
   key_size = 16
   key_bytes = randpool.RandomPool(512).get_bytes(key_size)
   return base64.urlsafe_b64encode(str(key_bytes))

def make_uuid():
    uuid_size = 32 + random.randint(1, 16)
    uuid_bytes = randpool.RandomPool(512).get_bytes(uuid_size)
    s = []
    for x in uuid_bytes:
        s.append(ord(x))	    
    print "uuid " + str(s)
    return base64.urlsafe_b64encode(str(uuid_bytes))

class UsernameUUIDModel(models.Model):
    username = models.CharField(max_length=255, primary_key=True,
                     unique=True, blank=False, null=False)
    uuid = models.CharField(max_length=255,
             default=make_uuid, editable=False,
             unique=True, blank=False, null=False)
    email = models.CharField(max_length=255,
               default='', null=False)
    ekey = models.CharField(max_length=255,
             default=generate_key_string, 
	     blank=False, null=False)
    
    def create_for_user(username, email):
        it = UsernameUUIDModel(username=username, email=email)
        it.save()
	s = encrypt_string(it.uuid, it.ekey)
	print s + '/n' + it.uuid
        return s
        
    def destroy_for_user(username):
        # if any, just one
        to_destroy = UsernameUUIDModel.objects.get(username=username)
        to_destroy.delete()
        
    def __unicode__(self):
        return self.username+":"+self.uuid+":"+self.email+":"+self.ekey
    
    create_for_user = staticmethod(create_for_user)
    destroy_for_user = staticmethod(destroy_for_user)
