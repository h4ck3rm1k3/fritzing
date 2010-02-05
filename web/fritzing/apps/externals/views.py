from django.shortcuts import render_to_response
from django.template import RequestContext

from fritzing.apps.externals.models import UsernameUUIDModel

def index(request):
    username = "jonathan"
    email = "jonathan@jonathan.com"
    uuid = UsernameUUIDModel.create_for_user(username, email)
    
    return render_to_response("externals/index.html", {
        'uuid': uuid,
    }, context_instance=RequestContext(request))
    