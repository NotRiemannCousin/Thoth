from Common import ResetCategory, EnableCategory
from source.Dsa    import *
from source.Http   import *
from source.Json   import *
from source.Errors import *


def __lldb_init_module(debugger, _dict):
    ResetCategory(debugger)

    register_dsa(debugger)
    register_http(debugger)
    register_json(debugger)
    register_errors(debugger)

    EnableCategory(debugger)