#include <array>
#include <Thoth/Utils/Env.hpp>
#include <Thoth/String/UnicodeView.hpp>
#include <filesystem>
#include <unordered_map>
#include <fstream>

using namespace std;

struct ValueBehaviour {
    string_view pattern{ " " };
    bool isMultiline{};
    bool useInterpolation{ true };
};


static std::optional<string> S_Interpolate(const string& str) {
    constexpr auto execCommand = [](const std::string& command) -> string {
#ifdef _WIN32
        const std::string cmd{ std::format(R"(powershell -NoProfile -Command "{}" 2>&1)", command) };
        std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
        // isso vai dar merda com o escaping de " mas tanto faz
#else
        const std::string cmd{ std::format("{} 2>&1", command) };
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
#endif

        if (!pipe) return {};

        std::string result;
        std::array<char, 256> buf{};

        while (fgets(buf.data(), buf.size(), pipe.get()))
            result += buf.data();
        if (result.back() == '\r' || result.back() == '\n')
            result.pop_back();

        // ReSharper disable once CppDFALocalValueEscapesFunction
        return result;
        // Vou falar em pt-br porque preguiça. O Clion tá dizendo que "result += buf.data();" é inseguro pq buf pode
        // acabar escapando. Sinceramente parece-me uma ilusão, creio que seja porque ele nn consegue inferir que se
        // fgets é sucedida o buffer é sempre null terminated. Em td caso, quero dormir.
    };


    string retBuffer, cmdBuffer;
    retBuffer.reserve(str.size());

    size_t lastIdx{}, idx{};
    size_t bracketLevel{};

    const auto computeEscapedChar = [&] {
        if(++idx == str.size())
            return false;

        string& buffer{ bracketLevel == 0 ? retBuffer : cmdBuffer };

        switch(str[idx]) {
            case '\\': buffer += '\\'; break;
            case 't':  buffer += '\t'; break;
            case 'n':  buffer += '\n'; break;
            case 'r':  buffer += '\r'; break;
            default: return false;
        }
        return true;
    };
    const auto computeOpenBracket = [&] {
        if (bracketLevel++ == 0)
            lastIdx = idx + 1;
    };
    const auto computeCloseBracket = [&] {
        if (--bracketLevel < 0)
            return false;
        if (bracketLevel == 0) {
            cmdBuffer.assign_range(str.substr(lastIdx, idx - lastIdx));

            retBuffer += execCommand(cmdBuffer);
        }
        return true;
    };

    for (; idx != str.size(); ++idx) {
        if (bracketLevel == 0)
            lastIdx = idx;

        idx = str.find_first_of(R"(\{})", idx);

        if (idx == string::npos)
            break;

        if (bracketLevel == 0)
            retBuffer.append_range(str.substr(lastIdx, idx - lastIdx));

        switch (str[idx]) {
            case '\\': if (!computeEscapedChar())  return std::nullopt; break;
            case '{':  computeOpenBracket();                            break;
            case '}':  if (!computeCloseBracket()) return std::nullopt; break;
            default: return std::nullopt;
        }
    }

    retBuffer.append_range(str.substr(lastIdx));

    return retBuffer;
}





struct Hash {
    using is_transparent = void;
    std::size_t operator()(const std::string& s) const { return std::hash<std::string>{}(s);      }
    std::size_t operator()(std::string_view   s) const { return std::hash<std::string_view>{}(s); }
};

struct Equal {
    using is_transparent = void;
    bool operator()(const std::string& a, const std::string& b) const { return a == b; }
    bool operator()(const std::string& a, std::string_view   b) const { return a == b; }
    bool operator()(std::string_view   a, const std::string& b) const { return a == b; }
    bool operator()(std::string_view   a, std::string_view   b) const { return a == b; }
};
// Ok this is stupid, unordered_map<string, string>'s find should work with string_view.

using Map = unordered_map<string, string, Hash, Equal>;

static optional<Map> S_GetMap() {
    Map vars;

    ifstream file(".env", ios::binary);
    if (!file)
        return vars;

    string key, val;
    string line;

    line.reserve(256);


    ValueBehaviour behaviour{};

    const auto getBehaviour = [&] {
        constexpr ValueBehaviour behaviours[] {
            { R"(''')", true,  false },
            { R"(""")", true,  true },
            { R"(')",   false, false },
            { R"(")",   false, true },
        };
        line.erase(0, line.find_first_not_of(' '));

        const auto it{
            ranges::find_if(behaviours, [&](const auto& str) {
                return line.starts_with(str);
            }, &ValueBehaviour::pattern)
        };

        if (it != std::end(behaviours))
            behaviour = *it;

        if (behaviour.pattern != " ")
            line.erase(0, behaviour.pattern.size());
    };
    const auto removeInlineEndingSequence = [&] {
        size_t idx{};

        if (behaviour.pattern == " ") {
            idx = line.find(behaviour.pattern);
        } else {
            do {
                idx = line.find(behaviour.pattern, idx);
            } while (idx != 0 && idx != string::npos && line[idx - 1] == '\\');


            if (idx == string::npos)
                return false;
        }


        if (idx != string::npos)
            line.erase(idx);
        return true;
    };

    const auto getValue = [&] {
        bool firstLoop{ true };

        do {
            getline(file, line);
            if (!file.eof())
                line.pop_back();


            if (firstLoop) getBehaviour();
            else           val += '\n';

            firstLoop = false;

            if (behaviour.isMultiline) {
                if (line.starts_with(behaviour.pattern))
                    break;
            } else
                if (!removeInlineEndingSequence())
                    return false;

            val.append_range(line);

        } while (behaviour.isMultiline);

        return true;
    };

    const auto isSpace        = [](const char c){ return c == ' '; };
    const auto isAllowedInKey = [](const char c){ return !isalnum(c) && c != '_'; };

    while (!file.eof()) {
        char c{ ' ' };
        while (file.get(c) && c != '=' && c != '\r' && c != '\n')
            key.push_back(c);

        if (key[0] == '#') {
            getline(file, key);
            continue;
        }

        if (isspace(c) && ranges::all_of(key, isSpace))
            continue;

        if (c != '=')
            return std::nullopt;
        key.erase(0, key.find_first_not_of(' '));



        if (isdigit(key[0]) || ranges::any_of(key, isAllowedInKey))
            return std::nullopt;


        getValue();

        if (behaviour.useInterpolation)
            val = S_Interpolate(val).value_or(""); // Maybe this will cause a serious bug someday but anyway

        vars.emplace(std::move(key), std::move(val));
    }

    return vars;
}


std::optional<const std::string_view> Thoth::Utils::Env(std::string_view envkey) {
    static auto envVars = S_GetMap();

    if (!envVars)
        return std::nullopt;

    const auto it{ envVars->find(envkey) };

    if (it == envVars->end())
        return std::nullopt;

    return it->second;
}
std::optional<const std::u8string_view> Thoth::Utils::Utf8Env(std::string_view envkey) {
    static auto envVars = S_GetMap();

    if (!envVars)
        return std::nullopt;

    const auto it{ envVars->find(envkey) };

    if (it == envVars->end())
        return std::nullopt;

    return u8string_view{ reinterpret_cast<const char8_t*>(it->second.data()), it->second.size() };
    // This is also ridiculous, but im too lazy to make a new UtfString now so please use UTF8 as the codepage
    // and everything should be fine. Also, please C++, fix char8_t, char16_t and char32_t or trow it away.
    // almost nothing on STL support it.
}


// sinceramente isso ficou uma merda insegura mas eu tô fazendo isso só pra ter mesmo então fds