from Common import *


def QuerySummary(query, _dict):
    data = query.GetValueForExpressionPath(".m_elements.m_data")

    def print_single(key, value):
        return f"{GetString(key)}={GetString(value)}"

    def print_multi(child):
        key = child.GetChildAtIndex(0)
        values = child.GetChildAtIndex(1)
        return "&".join(print_single(key, value) for value in GetVector(values))

    return Style(
        "&".join(print_multi(child) for child in GetVector(data)),
        CL_STRING,
    )


def UrlSummary(val, _dict):
    return Style(GetString(val.GetChildMemberWithName("m_rawUrl")), CL_YELLOW_S)


def register_http(debugger):
    AddSummary(debugger, "Thoth::Http::Url", "UrlSummary")
    AddSummary(debugger, "Thoth::Http::QueryParams", "QuerySummary")


__all__ = ["QuerySummary", "UrlSummary", "register_http"]