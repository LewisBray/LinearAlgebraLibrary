#define CATCH_CONFIG_MAIN
#include "catch\\catch.hpp"

#include "matrix.hpp"

#include <iostream>

TEST_CASE("Constructors")
{
    lal::matrix m = { {1, 1, 1}, {1, 1, 1}, {1, 1, 1} };
    m = lal::map(m, [](const int x) { return 2 * x; });
    for (const int element : m)
        REQUIRE(element == 2);

    m[2][2] = 3;

    for (auto it = m.crbegin(); it != m.crend(); ++it)
        std::cout << *it << ' ';
    std::cout << std::endl;

    const lal::matrix<int, 3, 3> m2{ m };
    for (const int element : m2)
        std::cout << element << ' ';
    std::cout << std::endl;

    auto diag = lal::diagonalise(5, 7, 9, 11, 13);
    for (const int element : diag)
        std::cout << element << ' ';
    std::cout << std::endl;
}