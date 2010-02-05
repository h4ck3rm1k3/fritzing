from django.db import models
import uuid
import os

# borrowed code from http://code.activestate.com/recipes/576980/
# and http://www.codekoala.com/blog/2009/aes-encryption-python-using-pycrypto/

from Crypto.Cipher import AES
from Crypto.Util import randpool
import base64

def encrypt_string(plain_text, key_string):
   block_size = 16
   mode = AES.MODE_CBC
   pad = block_size - len(plain_text) % block_size
   print "pad = " + str(pad)
   data = plain_text + (pad * chr(pad))
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
    return str(uuid.uuid4())

class UsernameUUIDModel(models.Model):
    username = models.CharField(max_length=255, primary_key=True,
                     unique=True, blank=False, null=False)
    uuid = models.CharField(max_length=36,
             default=make_uuid, editable=False,
             unique=True, blank=False, null=False)
    email = models.CharField(max_length=127,
               default='', null=False)
    key = models.CharField(max_length=128,
             default=generate_key_string, 
	     blank=False, null=False)
    
    def create_for_user(username, email):
        it = UsernameUUIDModel(username=username, email=email)
        it.save()
	print encrypt_string(it.uuid, it.key)
        return it.uuid
        
    def destroy_for_user(username):
        # if any, just one
        to_destroy = UsernameUUIDModel.objects.get(username=username)
        to_destroy.delete()
        
    def __unicode__(self):
        return self.username+":"+self.uuid+":"+self.email+":"+self.key
    
    create_for_user = staticmethod(create_for_user)
    destroy_for_user = staticmethod(destroy_for_user)
