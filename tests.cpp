#define CATCH_CONFIG_MAIN
#include "catch\\catch.hpp"

#include "matrix.hpp"

#include <string_view>
#include <algorithm>
#include <optional>
#include <numeric>
#include <utility>
#include <random>
#include <chrono>
#include <string>
#include <array>

// struct with defined move operations to test matrix move operators
struct point
{
    std::size_t x = 0;
    std::size_t y = 0;

    point() = default;
    ~point() = default;
    point(const point&) = default;
    point& operator=(const point&) = default;

    point(const std::size_t p1, const std::size_t p2) noexcept
        : x{ p1 }
        , y{ p2 }
    {}

    point(point&& other) noexcept
        : x{ other.x }
        , y{ other.y }
    {
        other.x = 0;
        other.y = 0;
    }

    point& operator=(point&& other) noexcept
    {
        x = other.x;
        y = other.y;
        other.x = 0;
        other.y = 0;

        return *this;
    }

    constexpr bool operator==(const point& other) const noexcept
    {
        return x == other.x && y == other.y;
    }
};

struct throws_when_default_constructed
{
    int value = 0;

    throws_when_default_constructed()
        : value{ randomNumber(-50, 50) }
    {
        if (value < 0)
            throw std::exception{};
    }

private:
    int randomNumber(const int low, const int high)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(low, high);

        return dis(gen);
    }
};

struct throws_when_copy_assigned
{
    int value = 0;

    throws_when_copy_assigned& operator=(const throws_when_copy_assigned& other)
    {
        value = other.value;
        if (value < 0)
            throw std::exception{};

        return *this;
    }
};

struct throws_when_move_assigned
{
    int value = 0;

    throws_when_move_assigned& operator=(throws_when_move_assigned&& other)
    {
        value = std::move(other.value);
        if (value < 0)
            throw std::exception{};

        return *this;
    }
};

using throws_when_default_constructed_matrix = lal::matrix<throws_when_default_constructed, 5, 55>;
using throws_when_copy_assigned_matrix = lal::matrix<throws_when_copy_assigned, 110, 12>;
using throws_when_move_assigned_matrix = lal::matrix<throws_when_move_assigned, 13, 12>;

TEST_CASE("Construction", "[construction]")
{
    SECTION("Default constructor")
    {
        REQUIRE(noexcept(lal::matrix<long long, 5, 12>{}));
        REQUIRE(!noexcept(throws_when_default_constructed_matrix{}));
        
        // Default construction should call default constructor for template type
        const lal::matrix<std::string, 3, 8> m;
        REQUIRE(m.rows() == 3u);
        REQUIRE(m.columns() == 8u);
        REQUIRE(m.size() == 3u * 8u);
        REQUIRE(m.max_size() == m.size());
        for (const std::string& string : m)
            REQUIRE(string == std::string{});
    }

    SECTION("initializer_list move constructor and corresponding template deduction guide")
    {
        REQUIRE(!noexcept(lal::matrix{ { 0.0, 1.0 }, { 1.0, 2.0 }, { 2.0, 3.0 } }));
        
        const lal::matrix m{ { 0.0, 1.0 }, { 1.0, 2.0 }, { 2.0, 3.0 } };
        REQUIRE(m.rows() == 3u);
        REQUIRE(m.columns() == 2u);
        REQUIRE(m.size() == 3u * 2u);
        REQUIRE(m.max_size() == m.size());
        for (std::size_t row = 0u; row < m.rows(); ++row)
            for (std::size_t column = 0u; column < m.columns(); ++column)
                REQUIRE(m[row][column] == static_cast<double>(row + column));
    }

    SECTION("matrix copy constructor")
    {
        REQUIRE(!noexcept(lal::matrix{ std::declval<throws_when_copy_assigned_matrix&>() }));

        const lal::matrix m1{ { 1.0f, 2.0f, 3.0f }, { 4.0f, 5.0f, 6.0f } };
        REQUIRE(noexcept(lal::matrix{ m1 }));
        const lal::matrix m2{ m1 };
        REQUIRE(m1.rows() == m2.rows());
        REQUIRE(m1.columns() == m2.columns());
        REQUIRE(m1.size() == m2.size());
        REQUIRE(m1.max_size() == m2.max_size());
        for (auto el1 = m1.begin(), el2 = m2.begin(); el1 != m1.end(); ++el1, ++el2)
            REQUIRE(*el1 == *el2);
    }

    SECTION("matrix move constructor")
    {
        REQUIRE(!noexcept(lal::matrix{ std::declval<throws_when_move_assigned_matrix>() }));

        lal::matrix m1{ { point{ 1, 1 } }, { point{ 2, 2 } }, { point{ 3, 3 } } };
        const lal::matrix m2{ std::move(m1) };
        REQUIRE(m1.rows() == m2.rows());
        REQUIRE(m1.columns() == m2.columns());
        REQUIRE(m1.size() == m2.size());
        REQUIRE(m1.max_size() == m2.max_size());
        for (std::size_t row = 0u; row < m1.rows(); ++row)
        {
            for (std::size_t column = 0u; column < m1.columns(); ++column)
            {
                REQUIRE(m2[row][column] == point{ row + 1, row + 1 });
                REQUIRE(m1[row][column] == point{});
            }
        }
    }

    using trivial_array = int[2][3];
    using throws_when_default_constructed_array = throws_when_default_constructed[5][7];

    SECTION("2-dimensional array copy constructor and corresponding template deduction guide")
    {
        using throws_when_copy_assigned_array = throws_when_copy_assigned[1][9];

        REQUIRE(noexcept(lal::matrix{ trivial_array{} }));
        REQUIRE(!noexcept(lal::matrix{ throws_when_default_constructed_array{} }));
        REQUIRE(!noexcept(lal::matrix{ throws_when_copy_assigned_array{} }));

        const int data[2][3] = { { 1, 2, 3 }, { 4, 5, 6 } };
        const lal::matrix m{ data };
        REQUIRE(m.rows() == 2u);
        REQUIRE(m.columns() == 3u);
        REQUIRE(m.size() == 2u * 3u);
        REQUIRE(m.max_size() == m.size());
        for (std::size_t row = 0u; row < m.rows(); ++row)
            for (std::size_t column = 0u; column < m.columns(); ++column)
                REQUIRE(m[row][column] == data[row][column]);
    }

    SECTION("2-dimensional array move constructor and corresponding template deduction guide")
    {
        using throws_when_move_assigned_array = throws_when_move_assigned[15][11];
        REQUIRE(noexcept(lal::matrix{ std::move(trivial_array{}) }));
        REQUIRE(!noexcept(lal::matrix{ throws_when_default_constructed_array{} }));
        REQUIRE(!noexcept(lal::matrix{ throws_when_move_assigned_array{} }));

        point points[2][2] = { { { 0, 0 }, { 0, 1 } }, { { 1, 0 }, { 1, 1 } } };
        const lal::matrix m{ std::move(points) };
        REQUIRE(m.rows() == 2u);
        REQUIRE(m.columns() == 2u);
        REQUIRE(m.size() == 2u * 2u);
        REQUIRE(m.max_size() == m.size());
        for (std::size_t row = 0u; row < m.rows(); ++row)
        {
            for (std::size_t column = 0u; column < m.columns(); ++column)
            {
                REQUIRE(m[row][column] == point{ row, column });
                REQUIRE(points[row][column] == point{});
            }
        }
    }
}

TEST_CASE("Assignment", "[assignment]")
{
    SECTION("matrix copy assignment")
    {
        REQUIRE(!noexcept(std::declval<throws_when_copy_assigned_matrix&>() = throws_when_copy_assigned_matrix{}));

        const lal::matrix m1{ { "Hello, ", "world", "!" } };
        REQUIRE(noexcept(std::declval<std::remove_const_t<decltype(m1)>&>() = m1));
        const lal::matrix m2 = m1;
        REQUIRE(m1.rows() == m2.rows());
        REQUIRE(m1.columns() == m2.columns());
        REQUIRE(m1.size() == m2.size());
        REQUIRE(m1.max_size() == m2.max_size());
        for (auto el1 = m1.begin(), el2 = m2.begin(); el1 != m1.end(); ++el1, ++el2)
            REQUIRE(*el1 == *el2);
    }

    SECTION("matrix move assignment")
    {
        REQUIRE(!noexcept(std::declval<throws_when_move_assigned_matrix&>() = std::declval<throws_when_move_assigned_matrix>()));

        lal::matrix m1{ { point{ 0u, 0u }, point{ 1u, 1u }, point{ 2u, 2u }, point{ 3u, 3u }, point{ 4u, 4u } } };
        REQUIRE(noexcept(std::declval<decltype(m1)&>() = std::declval<decltype(m1)>()));
        const lal::matrix m2{ std::move(m1) };
        REQUIRE(m1.rows() == 1u);
        REQUIRE(m1.columns() == 5u);
        REQUIRE(m1.size() == 1u * 5u);
        REQUIRE(m1.size() == m1.max_size());
        for (std::size_t row = 0u; row < m1.rows(); ++row)
        {
            for (std::size_t column = 0u; column < m1.columns(); ++column)
            {
                REQUIRE(m1[row][column] == point{});
                REQUIRE(m2[row][column] == point{ column, column });
            }
        }
    }
}

TEST_CASE("Access", "[access]")
{
    SECTION("Range checking access")
    {
        REQUIRE(!noexcept(std::declval<lal::matrix<int, 4, 5>>().at(4)));

        const lal::matrix<char, 5, 18> m;
        const auto& row = m.at(3);
        for (auto i = std::begin(row); i != std::begin(row) + m.columns(); ++i)
            REQUIRE(*i == char{});

        try
        {
            const auto& bad = m.at(m.rows() + 8);
        }
        catch (const std::out_of_range& error)
        {
            REQUIRE(error.what() == std::string_view{ "Subscript out of range" });
        }
        catch (...)
        {
            REQUIRE(false);
        }
    }

    SECTION("First element access")
    {
        lal::matrix<wchar_t, 14, 7> m;
        m.front() = L't';
        REQUIRE(noexcept(m.front()));
        REQUIRE(m.front() == L't');
        REQUIRE(noexcept(m[0][0]));
        REQUIRE(m[0][0] == L't');
        for (auto element = m.begin() + 1; element != m.end(); ++element)
            REQUIRE(*element == wchar_t{});
    }

    SECTION("Last element access")
    {
        const auto m = []() {
            lal::matrix<int, 15, 3> n;
            std::iota(n.begin(), n.end(), 0);
            return n;
        }();

        REQUIRE(noexcept(m.back()));
        REQUIRE(m.back() == static_cast<int>(m.size()) - 1);
        REQUIRE(m[m.rows() - 1][m.columns() - 1] == static_cast<int>(m.size()) - 1);
    }

    SECTION("Raw data array access")
    {
        lal::matrix<std::optional<double>, 24, 9> m;
        REQUIRE(noexcept(m.data()));
        auto* const p = m.data();
        for (std::size_t i = 0u; i < m.size(); ++i)
            p[i] = -18.0;
        for (const auto& element : m)
            REQUIRE(element.value() == -18.0);
    }
}

TEST_CASE("Iterators", "[iterators]")
{
    // Standard iterators are checked extensively throughout
    // all tests and so we only check the reverse ones

    SECTION("Reverse iterators")
    {
        const lal::matrix m = { { -6.0, -5.0, -4.0 }, { -3.0, -2.0, -1.0 } };
        for (auto [element, value] = std::make_pair(m.rbegin(), -1.0); element != m.rend(); ++element, --value)
            REQUIRE(*element == value);
    }
}

TEST_CASE("Algorithms", "[algorithms]")
{
    SECTION("Fill")
    {
        REQUIRE(!noexcept(std::declval<throws_when_copy_assigned_matrix>().fill(throws_when_copy_assigned{})));
        
        lal::matrix<std::array<double, 3>, 15, 82> m;
        REQUIRE(noexcept(m.fill(std::array{ 1.0, 2.0, 3.0 })));
        m.fill(std::array{ 1.0, 2.0, 3.0 });
        for (const auto& element : m)
            REQUIRE(element == std::array{ 1.0, 2.0, 3.0 });
    }

    SECTION("Swap")
    {
        REQUIRE(!noexcept(std::declval<throws_when_move_assigned_matrix>().swap(std::declval<throws_when_move_assigned_matrix&>())));
        
        lal::matrix<long double, 14, 52> m1;
        std::iota(m1.begin(), m1.end(), 0.0l);

        decltype(m1) m2;
        std::transform(m1.begin(), m1.end(), m2.begin(),
            [](const long double value) { return 2.0l * value; });
        
        REQUIRE(noexcept(m1.swap(m2)));
        m1.swap(m2);
        for (auto el1 = m1.begin(), el2 = m2.begin(); el1 != m1.end(); ++el1, ++el2)
            REQUIRE(*el1 == 2.0l * *el2);
    }
}

TEST_CASE("Addition", "[addition]")
{
    struct throws_when_added
    {
        int value = 0;

        constexpr throws_when_added operator+(const throws_when_added other)
        {
            const throws_when_added ret{ value + other.value };
            if (ret.value < 0)
                throw std::exception{};

            return ret;
        }

        constexpr throws_when_added& operator+=(const throws_when_added other)
        {
            *this = *this + other;
            return *this;
        }
    };

    using throws_when_added_matrix = lal::matrix<throws_when_added, 4, 15>;

    SECTION("Addition assignment operator")
    {
        REQUIRE(!noexcept(std::declval<throws_when_added_matrix&>() += throws_when_added_matrix{}));
        
        lal::matrix<int, 5, 15> m1;
        m1.fill(1);

        decltype(m1) m2;
        m2.fill(2);

        REQUIRE(noexcept(m1 += m2));
        m1 += m2;
        for (const int element : m1)
            REQUIRE(element == 3);
    }

    SECTION("Addition operator")
    {
        REQUIRE(!noexcept(throws_when_added_matrix{} + throws_when_added_matrix{}));

        constexpr auto f = [](const std::size_t x, const std::size_t y) noexcept {
            return 3 * x + 4 * y + x * y + 23;
        };

        lal::matrix<std::size_t, 11, 33> m1;
        for (std::size_t row = 0u; row < m1.rows(); ++row)
            for (std::size_t column = 0u; column < m1.columns(); ++column)
                m1[row][column] = f(row, column);

        decltype(m1) m2;
        std::transform(m1.begin(), m1.end(), m2.begin(),
            [](const std::size_t x) { return x * 12; });

        REQUIRE(noexcept(m1 + m2));
        const auto m3 = m1 + m2;
        for (std::size_t row = 0u; row < m1.rows(); ++row)
            for (std::size_t column = 0u; column < m1.columns(); ++column)
                REQUIRE(m3[row][column] == 13 * m1[row][column]);
    }
}

TEST_CASE("Subtraction", "[subtraction]")
{
    struct throws_when_subtracted
    {
        int value = 0;

        constexpr throws_when_subtracted operator-(const throws_when_subtracted other)
        {
            const throws_when_subtracted ret{ value + other.value };
            if (ret.value < 0)
                throw std::exception{};

            return ret;
        }

        constexpr throws_when_subtracted& operator-=(const throws_when_subtracted other)
        {
            *this = *this - other;
            return *this;
        }
    };

    using throws_when_subtracted_matrix = lal::matrix<throws_when_subtracted, 15, 48>;

    SECTION("Subtraction assignment operator")
    {
        REQUIRE(!noexcept(std::declval<throws_when_subtracted_matrix&>() -= throws_when_subtracted_matrix{}));

        lal::matrix<double, 13, 13> m1;
        m1.fill(-18.0);

        decltype(m1) m2;
        for (std::size_t row = 0u; row < m2.rows(); ++row)
            for (std::size_t column = 0u; column < m2.columns(); ++column)
                m2[row][column] = m1[row][column] + static_cast<double>(row + column);

        REQUIRE(noexcept(m1 -= m2));
        m1 -= m2;
        for (std::size_t row = 0u; row < m1.rows(); ++row)
            for (std::size_t column = 0u; column < m1.columns(); ++column)
                REQUIRE(m1[row][column] == -static_cast<double>(row + column));
    }

    SECTION("Subtraction operator")
    {
        REQUIRE(!noexcept(throws_when_subtracted_matrix{} - throws_when_subtracted_matrix{}));

        lal::matrix m1{ { 4, 8, 9 }, { 1, 9, 4 }, { 2, 2, 2 }, { 9, 0, -1 } };

        decltype(m1) m2;
        m2.fill(-10);

        REQUIRE(noexcept(m1 - m2));
        const auto result = m1 - m2;

        const decltype(m1) answer{ { 14, 18, 19 }, { 11, 19, 14 }, { 12, 12, 12 }, { 19, 10, 9 } };
        for (auto [r, a] = std::make_pair(result.begin(), answer.begin()); r != result.end(); ++r, ++a)
            REQUIRE(*r == *a);
    }
}

TEST_CASE("Multiplication", "[multiplication]")
{
    struct throws_when_multiplied
    {
        int value = 0;

        constexpr throws_when_multiplied& operator+=(const throws_when_multiplied other)
        {
            value += other.value;
            return *this;
        }

        constexpr throws_when_multiplied operator*(const throws_when_multiplied other)
        {
            const throws_when_multiplied ret{ value * other.value };
            if (ret.value < 0)
                throw std::exception{};

            return ret;
        }

        constexpr throws_when_multiplied& operator*=(const throws_when_multiplied other)
        {
            *this = *this * other;
            return *this;
        }
    };

    using throws_when_multiplied_matrix = lal::matrix<throws_when_multiplied, 4, 4>;

    SECTION("Multiplication operator")
    {
        REQUIRE(!noexcept(throws_when_multiplied_matrix{} * throws_when_multiplied_matrix{}));

        const lal::matrix m1{ { 2, 1, 4 }, { 0, 1, 1 } };
        const lal::matrix m2{ { 6, 3, -1, 0}, { 1, 1, 0, 4 }, { -2, 5, 0, 2 } };
        
        REQUIRE(noexcept(m1 * m2));
        const auto result = m1 * m2;
        const lal::matrix answer{ { 5, 27, -2, 12 }, { -1, 6, 0, 6 } };
        REQUIRE(result.rows() == answer.rows());
        REQUIRE(result.columns() == answer.columns());
        for (auto r = result.begin(), a = answer.begin(); r != result.end(); ++r, ++a)
            REQUIRE(*r == *a);
    }

    SECTION("Multiplication assignment operator")
    {
        REQUIRE(!noexcept(std::declval<throws_when_multiplied_matrix&>() *= throws_when_multiplied_matrix{}));

        lal::matrix m1{ { 5, -11 }, { 4, 0 }, { -2, 1 } };
        const lal::matrix m2{ { -3, -5 }, { -1, 4 } };

        REQUIRE(noexcept(m1 *= m2));
        m1 *= m2;
        const lal::matrix answer{ { -4, -69 }, { -12, -20 }, { 5, 14 } };
        REQUIRE(m1.rows() == answer.rows());
        REQUIRE(m2.columns() == answer.columns());
        for (std::size_t row = 0u; row < m1.rows(); ++row)
            for (std::size_t column = 0u; column < m1.columns(); ++column)
                REQUIRE(m1[row][column] == answer[row][column]);
    }

    SECTION("Scalar multiplication assignment operator")
    {
        REQUIRE(!noexcept(std::declval<throws_when_multiplied_matrix&>() *= throws_when_multiplied{}));

        lal::matrix<float, 13, 2> m;
        m.fill(19.0f);

        REQUIRE(noexcept(m *= 2.0f));
        m *= 2.0f;
        for (const float element : m)
            REQUIRE(element == 38.0f);
    }

    SECTION("Scalar multiplication operator")
    {
        REQUIRE(!noexcept(throws_when_multiplied_matrix{} * throws_when_multiplied{}));

        lal::matrix<double, 18, 19> m1;
        m1.fill(-1.0);

        REQUIRE(noexcept(-22.0 * m1));
        const auto m2 = -22.0 * m1;
        for (const double element : m2)
            REQUIRE(element == 22.0);

        REQUIRE(noexcept(m1 * 34.2));
        const auto m3 = m1 * 34.2;
        for (const double element : m3)
            REQUIRE(element == -34.2);
    }

    SECTION("Hadamard multiplication assignment operator")
    {
        REQUIRE(!noexcept(std::declval<throws_when_multiplied_matrix&>() %= throws_when_multiplied_matrix{}));

        lal::matrix<long long, 3, 13> m1;
        m1.fill(1ll);

        decltype(m1) m2;
        m2.fill(123456789ll);

        REQUIRE(noexcept(m1 %= m2));
        m1 %= m2;
        for (const long long element : m1)
            REQUIRE(element == 123456789ll);
    }

    SECTION("Hadamard multiplication operator")
    {
        REQUIRE(!noexcept(throws_when_multiplied_matrix{} % throws_when_multiplied_matrix{}));

        const lal::matrix m1{ { 2, 2, 2 }, { 5, 5, 5 }, { -1, -1, -1 } };
        const decltype(m1) m2{ { 3, 3, 3 }, { 2, 2, 2 }, { 4, 4, 4 } };

        REQUIRE(noexcept(m1 % m2));
        const auto m3 = m1 % m2;
        for (std::size_t column = 0u; column < m3.columns(); ++column)
        {
            REQUIRE(m3[0][column] == 6);
            REQUIRE(m3[1][column] == 10);
            REQUIRE(m3[2][column] == -4);
        }
    }
}

TEST_CASE("Division", "[division]")
{
    struct throws_when_divided
    {
        int value = 0;

        constexpr throws_when_divided operator/(const throws_when_divided other)
        {
            const throws_when_divided ret{ value / other.value };
            if (ret.value < 0)
                throw std::exception{};

            return ret;
        }

        constexpr throws_when_divided& operator/=(const throws_when_divided other)
        {
            *this = *this / other;
            return *this;
        }
    };

    using throws_when_divided_matrix = lal::matrix<throws_when_divided, 3, 18>;

    SECTION("Scalar division assignment operator")
    {
        REQUIRE(!noexcept(std::declval<throws_when_divided_matrix&>() /= throws_when_divided{}));

        lal::matrix m{ { 1.0 }, { 1.0 }, { 1.0 }, { 1.0 } };

        REQUIRE(noexcept(m /= 5.0));
        m /= 5.0;
        for (const double element : m)
            REQUIRE(element == 1.0 / 5.0);
    }

    SECTION("Scalar division operator")
    {
        REQUIRE(!noexcept(throws_when_divided_matrix{} / throws_when_divided{}));

        const lal::matrix m1{ { 4u, 4u }, { 5u, 5u }, { 6u, 6u }, { 7u, 7u } };

        REQUIRE(noexcept(m1 / m1.rows()));
        const auto m2 = m1 / m1.rows();
        for (const unsigned element : m2)
            REQUIRE(element == 1u);
    }
}

TEST_CASE("Equality", "[equality]")
{
    struct throws_when_equated
    {
        int value = 0;

        constexpr bool operator==(const throws_when_equated rhs)
        {
            if (value < 0 || rhs.value < 0)
                throw std::exception{};

            return value == rhs.value;
        }
    };

    using throws_when_equated_matrix = lal::matrix<throws_when_equated, 5, 55>;

    SECTION("Equality operator")
    {
        REQUIRE(!noexcept(throws_when_equated_matrix{} == throws_when_equated_matrix{}));

        const lal::matrix m1 = { { 3, 2, 1 }, { 6, 5, 4 }, { 9, 8, 7 }, { 12 , 11, 10 } };
        const auto m2{ m1 };
        REQUIRE(noexcept(m1 == m2));
        REQUIRE(m1 == m2);
    }

    SECTION("Inequality operator")
    {
        REQUIRE(!noexcept(throws_when_equated_matrix{} != throws_when_equated_matrix{}));

        const lal::matrix m1 = { { 7, 7, 7, 7, 7, 7, 7 }, { 2, 2, 2, 2, 2, 2, 2 } };
        lal::matrix m2{ m1 };
        m2[1][4] = 3;
        REQUIRE(noexcept(m1 != m2));
        REQUIRE(m1 != m2);
    }
}

TEST_CASE("Diagonalisation", "[diagonalisation]")
{
    REQUIRE(!noexcept(lal::diagonalise(throws_when_default_constructed{})));
    REQUIRE(!noexcept(lal::diagonalise(throws_when_copy_assigned{})));

    REQUIRE(noexcept(lal::diagonalise(0u, 1u, 4u, 9u, 16u, 25u)));
    const auto m = lal::diagonalise(0u, 1u, 4u, 9u, 16u, 25u);
    REQUIRE(m.rows() == 6u);
    REQUIRE(m.columns() == 6u);
    for (std::size_t row = 0u; row < m.rows(); ++row)
        for (std::size_t column = 0u; column < m.columns(); ++column)
            if (row == column)
                REQUIRE(m[row][column] == row * column);
            else
                REQUIRE(m[row][column] == 0u);
}

TEST_CASE("Transposition", "[transposition]")
{
    REQUIRE(!std::is_nothrow_default_constructible_v<throws_when_default_constructed_matrix>);
    REQUIRE(!noexcept(lal::transpose(throws_when_default_constructed_matrix{})));
    REQUIRE(!noexcept(lal::transpose(throws_when_copy_assigned_matrix{})));

    const lal::matrix m1 = { { 1, 2 }, { 3, 4 }, { 5, 6 }, { 7, 8 } };
    REQUIRE(noexcept(lal::transpose(m1)));
    const auto m2 = lal::transpose(m1);
    REQUIRE(m2.rows() == m1.columns());
    REQUIRE(m2.columns() == m1.rows());
    for (std::size_t row = 0u; row < m2.rows(); ++row)
        for (std::size_t column = 0u; column < m2.columns(); ++column)
            REQUIRE(m2[row][column] == m1[column][row]);
}

TEST_CASE("Magnitude", "[magnitude]")
{
    const lal::matrix m1{ { 0 }, { 1 }, { 2 }, { 3 }, { 4 } };
    REQUIRE(!noexcept(lal::magnitude(m1)));
    REQUIRE(lal::magnitude(m1) == 5);   // magnitude is of same type as matrix template type

    const lal::matrix m2{ { 0.0, 1.0 }, { 2.0, 3.0 }, { 4.0, 5.0 } };
    REQUIRE(!noexcept(lal::magnitude(m2)));
    REQUIRE(lal::magnitude(m2) == std::sqrt(55.0));
}

TEST_CASE("Mapping", "[mapping]")
{
    constexpr auto l = [](const int) -> int { throw std::exception{}; };
    REQUIRE(!noexcept(lal::map(lal::matrix<int, 5, 14>{}, l)));

    const lal::matrix<std::string_view, 2, 2> m1 = { { "Test", "writing" }, { "takes", "time" } };
    const auto f = [](const std::string_view sv) noexcept { return sv.length(); };
    
    REQUIRE(noexcept(lal::map(m1, f)));
    const auto m2 = lal::map(m1, f);
    REQUIRE(m2.rows() == m1.rows());
    REQUIRE(m2.columns() == m1.columns());
    REQUIRE(m2 == lal::matrix{ { 4u, 7u }, { 5u, 4u } });
}
