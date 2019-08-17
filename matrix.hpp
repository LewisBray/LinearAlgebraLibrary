#ifndef LAL_MATRIX_HPP
#define LAL_MATRIX_HPP

#include <initializer_list>
#include <type_traits>
#include <stdexcept>
#include <iterator>
#include <utility>
#include <cstddef>
#include <tuple>
#include <cmath>

namespace lal
{
    template <typename T, std::size_t Rows, std::size_t Columns>
    class matrix
    {
    public:
        // Type definitions
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using iterator = value_type*;
        using const_iterator = const value_type*;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        using row_reference = value_type(&)[Columns];
        using const_row_reference = const value_type(&)[Columns];

        // Construction and assignment
        matrix() = default;
        ~matrix() = default;
        matrix(const matrix&) = default;
        matrix& operator=(const matrix&) = default;
        matrix(matrix&&) = default;
        matrix& operator=(matrix&&) = default;

        constexpr matrix(const T (&data)[Rows][Columns]) noexcept(std::is_nothrow_default_constructible_v<T> && std::is_nothrow_assignable_v<T&, T>)
        {
            for (std::size_t row = 0u; row < Rows; ++row)
                for (std::size_t column = 0u; column < Columns; ++column)
                    data_[row][column] = data[row][column];
        }

        constexpr matrix(T (&&data)[Rows][Columns]) noexcept(std::is_nothrow_default_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)
        {
            for (std::size_t row = 0u; row < Rows; ++row)
                for (std::size_t column = 0u; column < Columns; ++column)
                    data_[row][column] = std::move(data[row][column]);
        }

        constexpr matrix(std::initializer_list<T[Columns]>&& row_list)
        {
            if (row_list.size() != Rows)
                throw std::length_error("Too many elements used to initialise matrix class");

            int row_num = 0;
            for (const auto& row : row_list)
                std::move(std::begin(row), std::end(row), data_[row_num++]);
        }

        // Access
        constexpr row_reference at(const size_type pos)
        {
            if (pos >= Rows)
                throw std::out_of_range("Subscript out of range");

            return data_[pos];
        }

        constexpr const_row_reference at(const size_type pos) const
        {
            if (pos >= Rows)
                throw std::out_of_range("Subscript out of range");

            return data_[pos];
        }

        constexpr reference front() noexcept { return data_[0][0]; }
        constexpr const_reference front() const noexcept { return data_[0][0]; }

        constexpr reference back() noexcept { return data_[Rows - 1][Columns - 1]; }
        constexpr const_reference back() const noexcept { return data_[Rows - 1][Columns - 1]; }

        constexpr pointer data() noexcept { return &data_[0][0]; }
        constexpr const_pointer data() const noexcept { return &data_[0][0]; }

        constexpr row_reference operator[](const size_type pos) noexcept { return data_[pos]; }
        constexpr const_row_reference operator[](const size_type pos) const noexcept { return data_[pos]; }

        // Iterators
        constexpr iterator begin() noexcept { return &data_[0][0]; }
        constexpr const_iterator begin() const noexcept { return &data_[0][0]; }
        constexpr const_iterator cbegin() const noexcept { return &data_[0][0]; }

        constexpr iterator end() noexcept { return begin() + size(); }
        constexpr const_iterator end() const noexcept { return begin() + size(); }
        constexpr const_iterator cend() const noexcept { return cbegin() + size(); }

        constexpr reverse_iterator rbegin() { return std::make_reverse_iterator(end()); }
        constexpr const_reverse_iterator rbegin() const { return std::make_reverse_iterator(end()); }
        constexpr const_reverse_iterator crbegin() const { return std::make_reverse_iterator(cend()); }

        constexpr reverse_iterator rend() { return std::make_reverse_iterator(begin()); }
        constexpr const_reverse_iterator rend() const { return std::make_reverse_iterator(begin()); }
        constexpr const_reverse_iterator crend() const { return std::make_reverse_iterator(cbegin()); }

        // Properties
        constexpr bool empty() const noexcept { return size() != static_cast<size_type>(0); }
        constexpr size_type size() const noexcept { return Rows * Columns; }
        constexpr size_type max_size() const noexcept { return Rows * Columns; }

        constexpr size_type rows() const noexcept { return Rows; }
        constexpr size_type columns() const noexcept { return Columns; }

        // Algorithms
        void fill(const T& value) noexcept(std::is_nothrow_assignable_v<T, T>)
        {
            for (auto& element : *this)
                element = value;
        }

        void swap(matrix& other) noexcept(std::is_nothrow_swappable_v<T>)
        {
            std::swap(data_, other.data_);
        }

    private:
        T data_[Rows][Columns]{};
    };

    // Matrix specialisation type definitions
    template <typename T, std::size_t Dimensions>
    using square_matrix = matrix<T, Dimensions, Dimensions>;

    template <typename T, std::size_t Dimensions>
    using row_vector = matrix<T, 1u, Dimensions>;

    template <typename T, std::size_t Dimensions>
    using column_vector = matrix<T, Dimensions, 1u>;

    // Template deduction guides
    template <typename T, std::size_t Size>
    using const_array_reference = const T(&)[Size];

    template <typename... T>
    using first_parameter_t = std::tuple_element_t<0, std::tuple<T...>>;

    template <typename... T, std::size_t Columns>
    matrix(const_array_reference<T, Columns>...) -> matrix<first_parameter_t<T...>, sizeof...(T), Columns>;

    template <typename T, std::size_t Rows, std::size_t Columns>
    matrix(matrix<T, Rows, Columns>) -> matrix<T, Rows, Columns>;

    template <typename T, std::size_t Rows, std::size_t Columns>
    matrix(const T (&)[Rows][Columns]) -> matrix<T, Rows, Columns>;

    // Addition
    template <typename T, std::size_t Rows, std::size_t Columns>
    constexpr matrix<T, Rows, Columns> operator+=(matrix<T, Rows, Columns>& lhs, const matrix<T, Rows, Columns>& rhs) noexcept(noexcept(std::declval<T&>() += T{}))
    {
        auto r = rhs.begin();
        for (auto l = lhs.begin(); l != lhs.end(); ++l, ++r)
            *l += *r;

        return lhs;
    }

    template <typename T, std::size_t Rows, std::size_t Columns>
    constexpr matrix<T, Rows, Columns> operator+(const matrix<T, Rows, Columns>& lhs, const matrix<T, Rows, Columns>& rhs) noexcept(std::is_nothrow_constructible_v<T> && noexcept(std::declval<T&>() += T{}))
    {
        matrix<T, Rows, Columns> ret{ lhs };
        ret += rhs;

        return ret;
    }

    // Subtraction
    template <typename T, std::size_t Rows, std::size_t Columns>
    constexpr matrix<T, Rows, Columns>& operator-=(matrix<T, Rows, Columns>& lhs, const matrix<T, Rows, Columns>& rhs) noexcept(noexcept(std::declval<T&>() -= T{}))
    {
        auto r = rhs.begin();
        for (auto l = lhs.begin(); l != lhs.end(); ++l, ++r)
            *l -= *r;

        return lhs;
    }

    template <typename T, std::size_t Rows, std::size_t Columns>
    constexpr matrix<T, Rows, Columns> operator-(const matrix<T, Rows, Columns>& lhs, const matrix<T, Rows, Columns>& rhs) noexcept(std::is_nothrow_constructible_v<T> && noexcept(std::declval<T&>() -= T{}))
    {
        matrix<T, Rows, Columns> ret{ lhs };
        ret -= rhs;

        return ret;
    }

    // Multiplication
    template <typename T, std::size_t I, std::size_t J, std::size_t K>
    constexpr matrix<T, I, K> operator*(const matrix<T, I, J>& lhs, const matrix<T, J, K>& rhs) noexcept(std::is_nothrow_default_constructible_v<T> && noexcept(T{} * T{}))
    {
        matrix<T, I, K> ret{};
        for (std::size_t i = 0u; i < I; ++i)
            for (std::size_t k = 0u; k < K; ++k)
                for (std::size_t j = 0u; j < J; ++j)
                    ret[i][k] += lhs[i][j] * rhs[j][k];

        return ret;
    }

    template <typename T, std::size_t Rows, std::size_t Columns>
    constexpr matrix<T, Rows, Columns>& operator*=(matrix<T, Rows, Columns>& lhs, const square_matrix<T, Columns>& rhs) noexcept(noexcept(T{} * T{}))
    {
        lhs = lhs * rhs;
        return lhs;
    }

    template <typename T, std::size_t Rows, std::size_t Columns>
    constexpr matrix<T, Rows, Columns>& operator*=(matrix<T, Rows, Columns>& m, const T scalar) noexcept(noexcept(std::declval<T&>() *= T{}))
    {
        for (auto& element : m)
            element *= scalar;

        return m;
    }

    template <typename T, std::size_t Rows, std::size_t Columns>
    constexpr matrix<T, Rows, Columns> operator*(const matrix<T, Rows, Columns>& m, const T scalar) noexcept(std::is_nothrow_constructible_v<T> && noexcept(std::declval<T&>() *= T{}))
    {
        matrix<T, Rows, Columns> ret{ m };
        ret *= scalar;

        return ret;
    }

    template <typename T, std::size_t Rows, std::size_t Columns>
    constexpr matrix<T, Rows, Columns> operator*(const T scalar, const matrix<T, Rows, Columns>& m) noexcept(std::is_nothrow_constructible_v<T> && noexcept(std::declval<T&>() *= T{}))
    {
        return m * scalar;
    }

    // Hadamard product
    template <typename T, std::size_t Rows, std::size_t Columns>
    constexpr matrix<T, Rows, Columns>& operator%=(matrix<T, Rows, Columns>& lhs, const matrix<T, Rows, Columns>& rhs) noexcept(noexcept(std::declval<T&>() *= T{}))
    {
        auto r = rhs.begin();
        for (auto l = lhs.begin(); l != lhs.end(); ++l, ++r)
            *l *= *r;

        return lhs;
    }

    template <typename T, std::size_t Rows, std::size_t Columns>
    constexpr matrix<T, Rows, Columns> operator%(const matrix<T, Rows, Columns>& lhs, const matrix<T, Rows, Columns>& rhs) noexcept(std::is_nothrow_constructible_v<T> && noexcept(std::declval<T&>() *= T{}))
    {
        matrix<T, Rows, Columns> ret{ lhs };
        ret %= rhs;

        return ret;
    }

    // Scalar division
    template <typename T, std::size_t Rows, std::size_t Columns>
    constexpr matrix<T, Rows, Columns>& operator/=(matrix<T, Rows, Columns>& m, const T scalar) noexcept(noexcept(std::declval<T&>() /= T{}))
    {
        for (auto& element : m)
            element /= scalar;

        return m;
    }

    template <typename T, std::size_t Rows, std::size_t Columns>
    constexpr matrix<T, Rows, Columns> operator/(const matrix<T, Rows, Columns>& m, const T scalar) noexcept(std::is_nothrow_constructible_v<T> && noexcept(std::declval<T&>() /= T{}))
    {
        matrix<T, Rows, Columns> ret{ m };
        ret /= scalar;
        
        return ret;
    }

    // Equality
    template <typename T, std::size_t Rows, std::size_t Columns>
    constexpr bool operator==(const matrix<T, Rows, Columns>& lhs, const matrix<T, Rows, Columns>& rhs) noexcept(noexcept(T{} == T{}))
    {
        for (auto l = lhs.begin(), r = rhs.begin(); l != lhs.end(); ++l, ++r)
            if (*l != *r)
                return false;

        return true;
    }

    template <typename T, std::size_t Rows, std::size_t Columns>
    constexpr bool operator!=(const matrix<T, Rows, Columns>& lhs, const matrix<T, Rows, Columns>& rhs) noexcept(noexcept(T{} == T{}))
    {
        return !(lhs == rhs);
    }

    // Common matrix operations
    template <typename T, typename... Ts, typename = std::enable_if_t<std::conjunction_v<std::is_same<T, Ts>...>>>
    constexpr square_matrix<T, 1 + sizeof...(Ts)> diagonalise(T t, Ts... ts) noexcept(std::is_nothrow_default_constructible_v<T> && std::is_nothrow_assignable_v<T&, T>)
    {
        square_matrix<T, 1 + sizeof...(Ts)> ret{};
        ret[0][0] = t;
        std::size_t i = 1u;
        ((ret[i][i++] = ts), ...);

        return ret;
    }

    template <typename T, std::size_t Rows, std::size_t Columns>
    constexpr matrix<T, Columns, Rows> transpose(const matrix<T, Rows, Columns>& m) noexcept(std::is_nothrow_default_constructible_v<T> && std::is_nothrow_assignable_v<T&, T>)
    {
        matrix<T, Columns, Rows> ret{};
        for (std::size_t row = 0u; row < Rows; ++row)
            for (std::size_t column = 0u; column < Columns; ++column)
                ret[column][row] = m[row][column];

        return ret;
    }

    template <typename T, std::size_t Rows, std::size_t Columns>
    constexpr auto magnitude(const matrix<T, Rows, Columns>& m)
    {
        const matrix<T, Rows, Columns> m_squared = m % m;

        T sum{};
        for (auto element = m_squared.begin(); element != m_squared.end(); ++element)
            sum += *element;

        if constexpr (std::is_floating_point_v<T>)
            return std::sqrt(sum);
        else
            return static_cast<T>(std::sqrt(static_cast<double>(sum)));
    }

    template <typename T, std::size_t Rows, std::size_t Columns, typename Function>
    constexpr auto map(const matrix<T, Rows, Columns>& m, Function f) noexcept(std::is_nothrow_default_constructible_v<decltype(f(T{}))> && noexcept(f(T{})) && std::is_nothrow_assignable_v<decltype(f(T{}))&, decltype(f(T{}))>) -> matrix<decltype(f(T{})), Rows, Columns>
    {
        auto element = m.begin();
        matrix<decltype(f(m.front())), Rows, Columns> ret{};
        for (auto r = ret.begin(); r != ret.end(); ++r, ++element)
            *r = f(*element);

        return ret;
    }
}

#endif
