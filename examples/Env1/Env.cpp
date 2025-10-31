#include <Thoth/Utils/Env.hpp>
#include <print>

using Thoth::Utils::Env;
int main() {

    std::print("{}\n\n", Env("DREAMS").value_or("Keep"));
    std::print("{}\n\n", Env("THATS_WHY").value_or("Tryin'"));
    return 0;
}