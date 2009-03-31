from fritzing.apps.parts.core import partomatic
from fritzing.apps.parts.core.defs import SCRIPTS_DEFS
import os

_GEN_FILES_FOLDER_PREFIX = os.path.join(os.path.dirname(__file__),"core/output")

def get_params_def(script_id):
    obj = SCRIPTS_DEFS[script_id] 
    return obj


def gen_files(script_id, config):
    return partomatic.web_generate(script_id, config, _GEN_FILES_FOLDER_PREFIX)
