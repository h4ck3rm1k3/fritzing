from django.db import models
import uuid

def make_uuid():
    return str(uuid.uuid4())

class UsernameUUIDModel(models.Model):
    username = models.CharField(max_length=255, primary_key=True,
                unique=True, blank=False, null=False)
    uuid = models.CharField(max_length=36,
            default=make_uuid, editable=False,
            unique=True, blank=False, null=False)
    
    def create_for_user(username):
        it = UsernameUUIDModel(username=username)
        it.save()
        return it.uuid
        
    def destroy_for_user(username):
        # if any, just one
        to_destroy = UsernameUUIDModel.objects.get(username=username)
        to_destroy.delete()
        
    def __unicode__(self):
        return self.username+":"+self.uuid
    
    create_for_user = staticmethod(create_for_user)
    destroy_for_user = staticmethod(destroy_for_user)
