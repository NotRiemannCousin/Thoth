import lldb

#region style

CL_GRAY         = "\u00feC"
CL_CLEAN        = "\u00feE"
CL_WHITE_GRAY_I = "\u00feK"
CL_NUMBER       = "\u00feN"
CL_STRING       = "\u00feS"
CL_WHITE        = "\u00feV"


def Style(message, color=CL_WHITE):
    return f"{color}{message}{CL_CLEAN}"

#endregion

def GetString(obj):
    summary = obj.GetSummary()
    return summary[3:-3] if summary else ""


def GetVector(obj):
    return list(obj)[2:-1]


def ResolveVariant(val):
    real_obj = val.GetNonSyntheticValue()

    while not real_obj.GetName().startswith("std::_Variant_base"):
        real_obj = real_obj.GetNonSyntheticValue().GetChildAtIndex(0)

    storage = real_obj.GetChildAtIndex(0)
    idx = real_obj.GetChildMemberWithName("_Which").GetValueAsUnsigned()

    if idx is None:
        return None, None

    for _ in range(idx):
        storage = storage.GetChildAtIndex(0).GetChildAtIndex(1)

    return idx, storage.GetChildAtIndex(0).GetChildAtIndex(0)




#region LLDB

CATEGORY_NAME = "Thoth"


def _CategoryFlags(is_regex):
    return "-x" if is_regex else ""


def AddSummary(debugger, type_name, function_name, is_regex=False):
    debugger.HandleCommand(
        f'type summary add -w {CATEGORY_NAME} {_CategoryFlags(is_regex)} '
        f'-F ThothLldb.{function_name} "{type_name}"'
    )


def AddSynthetic(debugger, type_name, class_name, is_regex=False):
    debugger.HandleCommand(
        f'type synthetic add -w {CATEGORY_NAME} {_CategoryFlags(is_regex)} '
        f'"{type_name}" --python-class ThothLldb.{class_name}'
    )


def ResetCategory(debugger):
    """Remove a categoria (se existir) pra evitar formatters duplicados
    quando o script é recarregado."""
    debugger.HandleCommand(f'type category delete {CATEGORY_NAME}')


def EnableCategory(debugger):
    debugger.HandleCommand(f'type category enable {CATEGORY_NAME}')

#endregion

__all__ = [
    "CATEGORY_NAME", "CL_GRAY", "CL_CLEAN", "CL_WHITE_GRAY_I", "CL_NUMBER",
    "CL_STRING", "CL_WHITE", "Style", "GetString",
    "GetVector", "ResolveVariant",
    "AddSummary", "AddSynthetic", "ResetCategory", "EnableCategory"
]