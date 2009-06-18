from fabric.api import env, run, hosts

env.project = 'fritzing'
env.hosts = ['fritzing.org']
env.user = 'fritzing'
env.app_dir = '~/virtualenv/src/fritzing'
env.web_root = '~/httpdocs'

def svn_status():
    "Updates the fritzing code from the Subversion repository."
    return run("cd %(app_dir)s && svn status" % env)

def svn_update():
    "Updates the fritzing code from the Subversion repository."
    run("cd %(app_dir)s && svn update" % env)

def req_update():
    "Updates the requirements by using pip on the server."
    run("cd %(app_dir)s & ~/virtualenv/bin/pip install -r %(app_dir)s/%(project)s/requirements.txt" % env)

def django_restart():
    "Restarts the Django proccess."
    run("touch %(app_dir)s/%(project)s/deploy/%(project)s.wsgi" % env)
