from Common import *


def CowSummary(val, _dict):
    idx, active = ResolveVariant(val.GetNonSyntheticValue().GetChildMemberWithName("m_value"))
    labels = {
        0: Style("Ref", CL_WHITE_GRAY_I),
        1: Style("Own", CL_WHITE),
    }
    label = labels.get(idx, "Unknown")
    return f"{label} {active.GetSummary()}"


def KeysSummary(val, _dict):
    type_name = (
        "std::variant<int,std::basic_string<char,std::char_traits<char>,"
        "std::allocator<char> > >"
    )

    def is_key(obj):
        return obj.GetTypeName() == type_name

    def with_brackets(obj):
        return f"[{ResolveVariant(obj)[1].GetSummary()}]"

    return "".join(with_brackets(child) for child in val if is_key(child))


def register_dsa(debugger):
    cow_name = r"^Thoth::Dsa::Cow<.*>$"

    keys_name = (
        r"^(std::(array|vector|span)|Thoth::Dsa::LinearMap)<"
        r"std::variant<int\s*,\s*"
        r"std::basic_string<char.*>.*>$"
    )

    AddSummary(debugger, cow_name, "CowSummary", is_regex=True)
    AddSummary(debugger, keys_name, "KeysSummary", is_regex=True)


__all__ = [
    "CowSummary",
    "KeysSummary",
    "register_dsa",
]