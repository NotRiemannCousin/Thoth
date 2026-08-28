from Common import *


def _value(value, color=CL_WHITE):
    return Style(value.GetValue(), color) if value is not None and value.IsValid() else "<unavailable>"


def _unsigned(value):
    try:
        return value.GetValueAsUnsigned()
    except Exception:
        return None


def _type_name(value):
    full = value.GetTypeName() or ""
    return full.rsplit("::", 1)[-1]


def _key(value):
    if value is None or not value.IsValid():
        return Style("<unavailable>", CL_GRAY)

    try:
        idx, active = ResolveVariant(value)
    except Exception:
        idx, active = None, None

    if active is None or not active.IsValid():
        return Style("<unavailable>", CL_GRAY)

    if idx == 0:  # int
        return _value(active, CL_NUMBER)

    return Style(f'"{GetString(active)}"', CL_STRING)

def _enum(value):
    name = value.GetValue()
    if name:
        return name
    return f"{_type_name(value)}({_value(value)})"


def JsonParseErrorSummary(value, _dict):
    idx = _value(value.GetChildMemberWithName("idx"), CL_NUMBER)
    c   = _value(value.GetChildMemberWithName("c"), CL_STRING)
    return f'{Style(f"({_type_name(value)})", CL_GRAY)} idx={idx}, char={c}'


def JsonGetErrorSummary(value, _dict):
    return f'{Style(f"({_type_name(value)})", CL_GRAY)} key={_key(value.GetChildMemberWithName("key"))}'


def JsonFindErrorSummary(value, _dict):
    path = value.GetChildMemberWithName("currentPath")
    keys = []
    if path is not None and path.IsValid():
        keys = [_key(child) for child in GetVector(path)]
    return (
        f'{Style(f"({_type_name(value)})", CL_GRAY)} '
        f'key={_key(value.GetChildMemberWithName("key"))}, path=[{", ".join(keys)}]'
    )


def JsonSearchErrorSummary(value, _dict):
    return f'{Style(f"({_type_name(value)})", CL_GRAY)} No object matches predicate'


def JsonWrongTypeErrorSummary(value, _dict):
    expected = _unsigned(value.GetChildMemberWithName("idxExpected"))
    got = _unsigned(value.GetChildMemberWithName("idxGot"))
    expected_name = _JSON_TYPES[expected] if expected is not None and expected < len(_JSON_TYPES) else "?"
    got_name = _JSON_TYPES[got] if got is not None and got < len(_JSON_TYPES) else "?"
    return (
        f'{Style(f"({_type_name(value)})", CL_GRAY)} '
        f'expected={Style(expected_name, CL_STRING)}, got={Style(got_name, CL_STRING)}'
    )


def UrlParseErrorSummary(value, _dict):
    return f'{Style(f"({_type_name(value)})", CL_GRAY)} {_enum(value)}'


def MessageParseErrorSummary(value, _dict):
    return f'{Style(f"({_type_name(value)})", CL_GRAY)} {_enum(value)}'


def GenericErrorSummary(value, _dict):
    error = _value(value.GetChildMemberWithName("error"), CL_STRING)
    return f'{Style(f"({_type_name(value)})", CL_GRAY)} {error}'


def ThothErrorSummary(value, _dict):
    index, active = ResolveVariant(value)
    if index is None or active is None or not active.IsValid():
        return "ThothError <invalid>"
    return active.GetSummary() or f'{Style(f"({_type_name(active)})", CL_GRAY)} {active.GetValue()}'


class ThothErrorSyntheticProvider:
    def __init__(self, value, _dict):
        self._value = value
        self._name = None
        self._active = None
        self.update()

    def update(self):
        _, active = ResolveVariant(self._value)
        if active is not None and active.IsValid():
            self._name = _type_name(active)
            self._active = active
        else:
            self._name = None
            self._active = None

    def num_children(self):
        return 1 if self._active is not None else 0

    def get_child_at_index(self, index):
        self._active.SetPreferSyntheticValue(False)
        return self._active.CreateValueFromAddress(self._name, self._active.GetLoadAddress(), self._active.GetType())

    def has_children(self):
        return self._active is not None


_JSON_TYPES = ("null", "string", "number", "bool", "object", "array")


def register_errors(debugger):
    summaries = {
        "Thoth::NJson::JsonParseError"      : "JsonParseErrorSummary",
        "Thoth::NJson::JsonGetError"        : "JsonGetErrorSummary",
        "Thoth::NJson::JsonFindError"       : "JsonFindErrorSummary",
        "Thoth::NJson::JsonSearchError"     : "JsonSearchErrorSummary",
        "Thoth::NJson::JsonWrongTypeError"  : "JsonWrongTypeErrorSummary",
        "Thoth::Http::UrlParseErrorEnum"    : "UrlParseErrorSummary",
        "Thoth::Http::MessageParseErrorEnum": "MessageParseErrorSummary",
        "Thoth::GenericError"               : "GenericErrorSummary",
    }

    for type_name, function_name in summaries.items():
        AddSummary(debugger, type_name, function_name)

    AddSummary(debugger, "Thoth::ThothError", "ThothErrorSummary")
    AddSynthetic(debugger, "Thoth::ThothError", "ThothErrorSyntheticProvider")


__all__ = [
    "JsonParseErrorSummary", "JsonGetErrorSummary", "JsonFindErrorSummary",
    "JsonSearchErrorSummary", "JsonWrongTypeErrorSummary", "UrlParseErrorSummary",
    "MessageParseErrorSummary", "GenericErrorSummary", "ThothErrorSummary",
    "ThothErrorSyntheticProvider", "register_errors",
]