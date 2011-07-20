#include <iostream>
#include <xtl/error.h>

using namespace xtl;

int main()
{
    std::cout << error_scope::system_error() << std::endl;
}
