#include <Thoth/NJson/Json.hpp>


namespace NJson = Thoth::NJson;

int main() {
    NJson::JsonObject sla{
                {"info", NJson::Array{
                    "AiKatherine",
                    "KG/M",
                    "NB",
                    20,
                    1.71,
                    false,
                    NJson::NullV,
               }},
                {"isAdmin", true},
                {"isPremium", true},
                {"links", NJson::JsonObject{
                    {"youtube", "https://www.youtube.com/@LastArchimedes"},
                    {"github", "https://github.com/NotRiemannCousin"},
                }}
    };

    auto j = NJson::Parse(R"(
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

    return 0;
}
