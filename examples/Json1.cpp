#include <Thoth/Json/Json.hpp>


namespace Json = Thoth::Json;

int main() {
    Json::Json sla{
                {"info", Json::Array{
                    "AiKatherine",
                    "KG/M",
                    "NB",
                    20,
                    1.71,
                    false,
                    Json::NullV,
               }},
                {"isAdmin", true},
                {"isPremium", true},
                {"links", Json::Json{
                    {"youtube", "https://www.youtube.com/@LastArchimedes"},
                    {"github", "https://github.com/NotRiemannCousin"},
                }}
    };

    auto j = Json::Json::Parse(R"(
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
