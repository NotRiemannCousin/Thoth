#pragma once
#include <Test0.hpp>
#include <Test1.hpp>
#include <Test2.hpp>
#include <Test3.hpp>
#include <Test4.hpp>


int main() {
    if (const auto members{ Test0::GetMembers(4001234) }) std::println("{}", *members);
    if (const auto members{ Test1::GetMembers(4001234) }) std::println("{}", *members);
    if (const auto members{ Test2::GetMembers(4001234) }) std::println("{}", *members);
    if (const auto members{ Test3::GetMembers(4001234) }) std::println("{}", *members);
    if (const auto members{ Test4::GetMembers(4001234) }) std::println("{}", *members);

    return 0;
}


// ReSharper disable All



struct MyClass{
    void SingleDefinition(int x, int y);

    void Overloaded(int x, int y);
    void Overloaded(double x, double y);
};


// irei voltar em lambdas logo mais
constexpr auto overloadedFunc{ [](MyClass* self, auto x, auto y){ return self->Overloaded(x, y); } };



