#include <Thoth/Utils/Env.hpp>
#include <print>

using Thoth::Utils::Env;
int main() {

    std::print("with    interp.: {}\n\n", Env("WHOAMI").value_or("Cant find value"));
    std::print("without interp.: {}\n\n", Env("NOT_WHOAMI").value_or("Cant find value"));

    std::print("Person:\n{}\n\n", Env("JSON").value_or("Cant find json"));
    std::print("certo? {}\n\n", Env("IDK").value_or("Cant find json"));

    std::print("You probably should try to clean you path, \"{}\" is way to much.\n\n",
        Env("PATH").value_or("ah sad this would be funny"));
    return 0;
}