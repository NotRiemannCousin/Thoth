# código feito na pressa, dps arrumo (algum dia)

ID_NULL   = 0
ID_STRING = 1
ID_NUMBER = 2
ID_BOOL   = 3
ID_OBJECT = 4
ID_ARRAY  = 5

TYPE_NAME  = {
    ID_NULL   : "null",
    ID_STRING : "string",
    ID_NUMBER : "number",
    ID_BOOL   : "bool",
    ID_OBJECT : "object",
    ID_ARRAY  : "array",
}


# S -> as string
# I -> italic
CL_GRAY         = "\u00feC"
CL_CLEAN        = "\u00feE"
CL_WHITE_GRAY_I = "\u00feK"
CL_WHITE_I      = "\u00feN"
CL_YELLOW_S     = "\u00feS"
CL_WHITE        = "\u00feV"
def Style(message, color = CL_WHITE):
    return f"{color}{message}{CL_CLEAN}"

def GetJsonVariant(val):
    return val.GetNonSyntheticValue().GetChildMemberWithName("_value")

def GetString(obj): # obviamente está horrível, mudar dps
    return obj.GetSummary()[3:-3]

def GetVector(obj): # vc tb
    return list(obj)[2:-1]
def QuerySummary(query, _dict):
    data = query.GetValueForExpressionPath("._elements._data")

    def PrintSingle(key, value):
        # print(f"{GetString(key)}={GetString(value)}")
        return f"{GetString(key)}={GetString(value)}"
    def PrintMulti(child):
        key  = child.GetChildAtIndex(0)
        vals = child.GetChildAtIndex(1)

        return "&".join(PrintSingle(key, val) for val in GetVector(vals))


    acc = Style("&".join(PrintMulti(child) for child in GetVector(data)), CL_YELLOW_S)




    return acc

def UrlSummary(val, _dict):
    scheme   = GetString(val.GetChildMemberWithName("scheme"))
    user     = GetString(val.GetChildMemberWithName("user"))
    host     = GetString(val.GetChildMemberWithName("host"))
    port     = val.GetChildMemberWithName("port").GetValue()

    path     = GetString(val.GetChildMemberWithName("path"))

    query    = GetString(val.GetChildMemberWithName("query"))
    fragment = GetString(val.GetChildMemberWithName("fragment"))


    if port == None:
        port = ""
    elif scheme == "http" and port == "80":
        port = ""
    elif scheme == "https" and port == "443":
        port = ""

    if user:
        user = f"{user}@"
    if port:
        port = f":{port}"
    if query:
        query = f"?{query}"
    if fragment:
        fragment = f"#{fragment}"

    return Style(f"{scheme}://{user}{host}{port}{path}{query}{fragment}", CL_YELLOW_S)

def KeysSummary(val, _dict):
    TYPE = "std::variant<int,std::basic_string<char,std::char_traits<char>,std::allocator<char> > >"

    is_key        = lambda obj: obj.GetTypeName() == TYPE
    with_brackets = lambda obj: f"[{ResolveVariant(obj)[1].GetSummary()}]"

    return "".join([with_brackets(c) for c in val if is_key(c)])


def ResolveVariant(val):
    real_obj = val.GetNonSyntheticValue()


    while not real_obj.GetName().startswith("std::_Variant_base"):
        real_obj = real_obj.GetNonSyntheticValue().GetChildAtIndex(0)

    storage = real_obj.GetChildAtIndex(0) # `:  private _Variant_storage<...`
    idx     = real_obj.GetChildMemberWithName("_Which").GetValueAsUnsigned()

    if idx is None:
        return None, None

    for i in range(idx):
        storage = storage.GetChildAtIndex(0).GetChildAtIndex(1) # union -> _Tail

    return idx, storage.GetChildAtIndex(0).GetChildAtIndex(0) # union -> _Head


def JsonObjectSummary(val, _dict):
    obj = JsonObjectSyntheticProvider(val, dict)

    make_name = lambda i: Style(obj.get_child_at_index(i).GetName()[1:-1], CL_YELLOW_S)

    preview_limit = 5
    preview_size = min(obj.num_children(), preview_limit)
    preview = [f"{make_name(i)}:..." for i in range(preview_size)]
    preview = ", ".join(preview)

    if obj.num_children() > preview_limit:
        preview += " and more"

    return f"object <size={Style(obj.num_children())}> {{ {preview} }} "

def ArraySummary(val, _dict):
    arr = ArraySyntheticProvider(val, dict)
    return f"array[ size={Style(arr.num_children())} ]"


def CowSummary(val, _dict):
    idx, val = ResolveVariant(GetJsonVariant(val))

    labels = {
        0: Style("Ref", CL_WHITE_GRAY_I),
        1: Style("Own", CL_WHITE),
    }
    label = labels.get(idx, "Unknown")

    return f'{label} {val.GetSummary()}'



def JsonSummary(val, _dict):
    idx, val = ResolveVariant(GetJsonVariant(val))

    if idx is None:
        return f"Unknown json type"

    if idx == ID_NULL:
        return "null"

    return val.GetSummary()





class JsonObjectSyntheticProvider:
    """Thoth::Json's JsonObject synthetic provider."""
    def __init__(self, val, _dict):
        self._vals = None
        val.SetPreferSyntheticValue(False)

        if val.GetTypeName().startswith("std::unique_ptr"):
            val = val.GetChildMemberWithName("_Mypair")
            val = val.GetChildMemberWithName("_Myval2")
        val = val.GetChildMemberWithName("_pairs")
        val = val.GetChildMemberWithName("_data")

        val.SetPreferSyntheticValue(True)

        self._obj = val
        self.update()

    def update(self):
        self._vals = [self._obj.GetChildAtIndex(i) for i in range(2, self._obj.num_children - 1)]
        return

    def num_children(self):
        return len(self._vals)

    def get_child_at_index(self, index):
        el = self._vals[index]
        el.SetPreferSyntheticValue(False)

        first = el.GetChildMemberWithName("first")
        second = el.GetChildMemberWithName("second")

        name = f'[{first.GetSummary()[2:-2]}]'

        return el.CreateValueFromAddress(
            name,
            second.GetLoadAddress(),
            second.GetType()
        )

    def has_children(self):
        return True


class ForwardSyntheticProvider:
    """Thoth::Json's Array synthetic provider."""
    def __init__(self, val, _dict):
        self._obj = val

    def update(self):
        return

    def num_children(self):
        return self._obj.num_children

    def get_child_at_index(self, index):
        return self._obj.GetChildAtIndex(index)

    def has_children(self):
        return self._obj.HasChildren()


class ArraySyntheticProvider:
    """Thoth::Json's Array synthetic provider."""

    def __init__(self, val, _dict):
        val.SetPreferSyntheticValue(True)
        self._obj = val


    def update(self):
        return

    def num_children(self):
        return self._obj.num_children - 3

    def get_child_at_index(self, index):
        return self._obj.GetChildAtIndex(index + 2)

    def has_children(self):
        return True


class JsonSyntheticProvider:
    """Thoth::Json's Json synthetic provider."""

    def __init__(self, val, _dict):
        self._variant = val
        idx, val = ResolveVariant(GetJsonVariant(val))

        providers = {
            ID_OBJECT : JsonObjectSyntheticProvider,
            ID_ARRAY  : ArraySyntheticProvider,
        }

        provider = providers.get(idx, ForwardSyntheticProvider)
        self._payload = provider(val, _dict)

    def update(self):
        return

    def num_children(self):
        return self._payload.num_children()

    def get_child_at_index(self, index):
        return self._payload.get_child_at_index(index)

    def has_children(self):
        return self._payload.has_children()





def __lldb_init_module(debugger, _dict):
    JSON_NAME = "Thoth::NJson::Json"
    OBJ_NAME  = "^(Thoth::NJson::JsonObject|std::unique_ptr<Thoth::NJson::JsonObject,std::default_delete<Thoth::NJson::JsonObject> >)$"
    ARR_NAME  = "std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,Thoth::NJson::Json>,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,Thoth::NJson::Json> > >"
    STR_NAME  = "^Thoth::Dsa::Cow<.*>$"
    URL_NAME  = "Thoth::Http::Url"
    QUERY_NAME  = "Thoth::Http::QueryParams"

    KEYS_NAME = "^(std::(array|vector|span)|Thoth::Dsa::LinearMap)<std::variant<int,std::basic_string<char,std::char_traits<char>,std::allocator<char> > >.*>$"

    debugger.HandleCommand(f'type summary add -F ThothLldb.ArraySummary "{ARR_NAME}"')
    debugger.HandleCommand(f'type summary add -F ThothLldb.JsonSummary "{JSON_NAME}"')
    debugger.HandleCommand(f'type summary add -F ThothLldb.CowSummary -x "{STR_NAME}"')
    debugger.HandleCommand(f'type summary add -F ThothLldb.JsonObjectSummary -x "{OBJ_NAME}"')
    debugger.HandleCommand(f'type summary add -F ThothLldb.QuerySummary "{QUERY_NAME}"')
    debugger.HandleCommand(f'type summary add -F ThothLldb.UrlSummary "{URL_NAME}"')


    debugger.HandleCommand(f'type summary add -F ThothLldb.KeysSummary -x "{KEYS_NAME}"')

    # debugger.HandleCommand(f'type synthetic add "{ARR_NAME}" --python-class ThothLldb.ArraySyntheticProvider')
    debugger.HandleCommand(f'type synthetic add "{JSON_NAME}" --python-class ThothLldb.JsonSyntheticProvider')
    debugger.HandleCommand(f'type synthetic add -x "{OBJ_NAME}" --python-class ThothLldb.JsonObjectSyntheticProvider')
