from Common import *


ID_NULL   = 0
ID_STRING = 1
ID_NUMBER = 2
ID_BOOL   = 3
ID_OBJECT = 4
ID_ARRAY  = 5

def GetJsonVariant(val):
    return val.GetNonSyntheticValue().GetChildMemberWithName("m_value")


#region summaries
def JsonObjectSummary(val, _dict):
    obj = JsonObjectSyntheticProvider(val, _dict)
    make_name = lambda i: Style(obj.get_child_at_index(i).GetName()[1:-1], CL_STRING)

    preview_limit = 5
    preview_size = min(obj.num_children(), preview_limit)
    preview = [f"{make_name(i)}:..." for i in range(preview_size)]
    preview = ", ".join(preview)

    if obj.num_children() > preview_limit:
        preview += " and more"

    return f"object <size={Style(obj.num_children(), CL_NUMBER)}> {{ {preview} }} "


def ArraySummary(val, _dict):
    val.SetPreferSyntheticValue(True)
    size = len(GetVector(val))
    return f"array[ size={Style(size, CL_NUMBER)} ]"


def CowSummary(val, _dict):
    idx, active = ResolveVariant(GetJsonVariant(val))
    labels = {
        0: Style("Ref", CL_WHITE_GRAY_I),
        1: Style("Own"),
    }
    label = labels.get(idx, "Unknown")
    return f"{label} {active.GetSummary()}"


def NumberSummary(val, _dict):
    idx, active = ResolveVariant(val)
    if idx is None or active is None or not active.IsValid():
        return "(invalid number)"

    label = f"(unknown number alternative {idx})"
    if idx == 0:
        label = "(i64)"
    if idx == 1:
        label = "(u64)"
    if idx == 2:
        label = "(double)"

    return f"{Style(label, CL_GRAY)} {active.GetSummary()}"


def JsonSummary(val, _dict):
    idx, active = ResolveVariant(GetJsonVariant(val))
    if idx is None:
        return "Unknown json type"

    summary = Style("null", CL_NUMBER) if idx == ID_NULL else active.GetSummary()

    return f"{Style('(json)', CL_GRAY)} {summary}"


def JsonPtrSummary(val, _dict):
    val.SetPreferSyntheticValue(False)
    ptr = val.GetChildMemberWithName("_Mypair").GetChildMemberWithName("_Myval2")
    if ptr is None or not ptr.IsValid():
        return "<invalid JsonObject pointer>"

    if ptr.GetValueAsUnsigned() == 0:
        return "nullptr"

    return f"{Style('(ptr)', CL_GRAY)} {JsonObjectSummary(ptr.Dereference(), _dict)}"


#endregion

#region synthetic providers

def _json_object_children(json_object_value):
    """Filhos [key] de um Thoth::NJson::JsonObject cru (não sintético)."""
    children = []
    data = json_object_value.GetChildMemberWithName("m_pairs").GetChildMemberWithName("m_data")

    if not data.IsValid():
        return children

    data.SetPreferSyntheticValue(True)

    for element in data.children[2:-1]:
        element.SetPreferSyntheticValue(True)
        key = element.GetChildAtIndex(0).GetSummary()[2:-2]
        value = element.GetChildAtIndex(1)

        if not value.IsValid():
            continue

        children.append(value.CreateValueFromAddress(f"[{key}]", value.GetLoadAddress(), value.GetType()))

    return children


class JsonObjectSyntheticProvider:
    """Synthetic provider para Thoth::NJson::JsonObject: só os pares [key]."""

    def __init__(self, val, _dict):
        self.raw_view = val.GetNonSyntheticValue()
        self.children = _json_object_children(self.raw_view)

    def update(self):
        self.children = _json_object_children(self.raw_view)

    def num_children(self):
        return len(self.children)

    def get_child_at_index(self, index):
        return self.children[index]

    def get_child_index(self, name):
        for i, child in enumerate(self.children):
            if child.GetName() == name:
                return i
        return -1

    def has_children(self):
        return bool(self.children)


class ObjectSyntheticProvider:
    """Synthetic provider para std::unique_ptr<JsonObject>: desembrulha o
    ponteiro (GetChildMemberWithName já auto-deref) e expõe os pares [key]
    do JsonObject apontado."""

    def __init__(self, val, _dict):
        self.raw_view = val.GetNonSyntheticValue()
        self.children = self._generate_children()

    def _generate_children(self):
        ptr_val = self.raw_view.GetChildMemberWithName("_Mypair").GetChildMemberWithName("_Myval2")
        if not ptr_val.IsValid():
            return []
        return _json_object_children(ptr_val)

    def update(self):
        self.children = self._generate_children()

    def num_children(self):
        return len(self.children)

    def get_child_at_index(self, index):
        return self.children[index]

    def get_child_index(self, name):
        for i, child in enumerate(self.children):
            if child.GetName() == name:
                return i
        return -1

    def has_children(self):
        return bool(self.children)


class JsonSyntheticProvider:
    """Synthetic provider para Thoth::NJson::Json.

    - object: a alternativa ativa já é um unique_ptr<JsonObject>, tipo que
      já tem um `type synthetic add` registrado (ObjectSyntheticProvider) —
      só habilitamos SetPreferSyntheticValue e deixamos o LLDB despachar
      pro provider certo sozinho, do jeito nativo.
    - array: std::vector cru, sem synthetic próprio registrado; reusamos
      GetVector() (o mesmo helper usado em Http.py/Errors.py) pra cortar
      o padding de 2 filhos na frente e 1 no fim.
    - scalar (string/number/bool/null): filhos naturais do valor, sem
      mexer em nada.
    """

    def __init__(self, val, _dict):
        idx, active = ResolveVariant(GetJsonVariant(val))
        self._active = active
        self._array_items = None

        if not active.IsValid():
            return

        active.SetPreferSyntheticValue(True)
        if idx == ID_ARRAY:
            self._array_items = GetVector(active)

    def update(self):
        return

    def num_children(self):
        if self._array_items is not None:
            return len(self._array_items)

        return self._active.GetNumChildren() if self._active.IsValid() else 0

    def get_child_at_index(self, index):
        if self._array_items is not None:
            return self._array_items[index]
        return self._active.GetChildAtIndex(index)

    def has_children(self):
        return self.num_children() > 0

#endregion

def register_json(debugger):
    JSON_NAME = "Thoth::NJson::Json"
    JSON_OBJ_NAME = "Thoth::NJson::JsonObject"
    JSON_PTR_NAME = f"std::unique_ptr<{JSON_OBJ_NAME},std::default_delete<{JSON_OBJ_NAME}> >"

    ARR_NAME = "std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,Thoth::NJson::Json>,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,Thoth::NJson::Json> > >"
    STR_NAME = "^Thoth::Dsa::Cow<.*>$"
    NUM_NAME = "Thoth::NJson::Number"

    AddSummary(debugger, ARR_NAME, "ArraySummary")
    AddSummary(debugger, STR_NAME, "CowSummary", is_regex=True)
    AddSummary(debugger, NUM_NAME, "NumberSummary")

    AddSummary(debugger, JSON_PTR_NAME, "JsonPtrSummary")
    AddSummary(debugger, JSON_OBJ_NAME, "JsonObjectSummary")
    AddSummary(debugger, JSON_NAME, "JsonSummary")

    AddSynthetic(debugger, JSON_NAME, "JsonSyntheticProvider")
    AddSynthetic(debugger, JSON_OBJ_NAME, "JsonObjectSyntheticProvider")
    AddSynthetic(debugger, JSON_PTR_NAME, "ObjectSyntheticProvider")
__all__ = [
    "JsonObjectSummary", "ArraySummary", "CowSummary", "NumberSummary", "JsonSummary",
    "JsonPtrSummary", "JsonObjectSyntheticProvider", "ObjectSyntheticProvider",
    "JsonSyntheticProvider", "register_json"
]