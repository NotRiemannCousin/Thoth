#include <print>
#include <Thoth/NJson/Json.hpp>
#include <Thoth/NJson/Utils.hpp>


namespace NJson = Thoth::NJson;

int main() {
    const NJson::JsonObject obj{
                {"info", NJson::MakeArray(
                    "AiKatherine",
                    "KG/M",
                    "NB",
                    20,
                    1.71,
                    false,
                    NJson::NullV
               )},
                {"isAdmin", true},
                {"isPremium", true},
                {"links", {{
                    {"youtube", "https://www.youtube.com/@LastArchimedes"},
                    {"github", "https://github.com/NotRiemannCousin"},
                }}}
    };

    const auto parsed = NJson::Json::Parse(R"(
    {
        "info" : [
            "AiKatherine",
            "KG/M",
            "NB",
            20,
            1.71,
            false,
            null
       ],
        "isAdmin" : true,
        "isPremium" : true,
        "links" : {
            "youtube" : "https://www.youtube.com/@LastArchimedes",
            "github" : "https://github.com/NotRiemannCousin"
        }
    }
)");

    std::print("{}", parsed && *parsed == obj);

    return 0;
}
