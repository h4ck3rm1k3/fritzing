from django.shortcuts import render_to_response
from django.template import RequestContext

from fritzing.apps.externals.models import UsernameUUIDModel
from django.contrib.auth.decorators import login_required
from fritzing import settings

@login_required
def index(request):
        # username = "jonathan"
        # email = "jonathan@jonathan.com"
        if request.user.is_authenticated():
                username = request.user.username
                email = request.user.email
        else:
                username = ""
                email = ""
        uuid = UsernameUUIDModel.create_for_user(username, email)
    
        return render_to_response("externals/index.html", 
                {'uuid': uuid, "grails_server":settings.GRAILS_SERVER}, context_instance=RequestContext(request))
    