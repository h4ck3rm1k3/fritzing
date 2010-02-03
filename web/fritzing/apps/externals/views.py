from django.shortcuts import render_to_response
from django.template import RequestContext

from fritzing.apps.externals.models import UsernameUUIDModel

def index(request):
    username = "jonathan"
    uuid = UsernameUUIDModel.create_for_user(username)
    
    return render_to_response("externals/index.html", {
        'username': username,
        'uuid': uuid,
    }, context_instance=RequestContext(request))
    