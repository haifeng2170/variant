#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <sstream>
#include <utility>
#include <type_traits>
#include <boost/variant.hpp>
#include <boost/timer/timer.hpp>
#include "variant.hpp"

namespace test {

template <typename T>
struct string_to_number {};

template <>
struct string_to_number<double>
{
    double operator() (std::string const& str) const
    {
        return std::stod(str);
    }
};

template <>
struct string_to_number<int64_t>
{
    int64_t operator() (std::string const& str) const
    {
        return std::stoll(str);
    }
};

template <>
struct string_to_number<uint64_t>
{
    uint64_t operator() (std::string const& str) const
    {
        return std::stoull(str);
    }
};

template <>
struct string_to_number<bool>
{
    bool operator() (std::string const& str) const
    {
        bool result;
        std::istringstream(str) >> std::boolalpha >> result;
        return result;
    }
};

struct javascript_equal_visitor : util::static_visitor<bool>
{
    template <typename T>
    bool operator() (T lhs, T rhs) const
    {
        return lhs == rhs;
    }

    template <typename T, class = typename std::enable_if<std::is_arithmetic<T>::value>::type >
    bool operator() (T lhs, std::string const& rhs) const
    {
        return lhs == string_to_number<T>()(rhs);
    }

    template <typename T, class = typename std::enable_if<std::is_arithmetic<T>::value>::type >
    bool operator() (std::string const& lhs, T rhs) const
    {
        return string_to_number<T>()(lhs) == rhs;
    }

    template <typename T0, typename T1>
    bool operator() (T0 lhs, T1 rhs) const
    {
        return lhs == rhs;
    }
};

template <typename T>
struct javascript_equal
{
    javascript_equal(T const& lhs)
        : lhs_(lhs) {}

    bool operator() (T const& rhs) const
    {
        return util::apply_visitor(lhs_, rhs, test::javascript_equal_visitor());
    }
    T const& lhs_;
};

}

int main (/*int argc, char** argv*/)
{
    //typedef util::variant<int, std::string> variant_type;
    typedef util::variant<bool, int64_t, uint64_t, double, std::string> variant_type;
    variant_type v0(3.14159);
    variant_type v1(std::string("3.14159"));
    variant_type v2(1ULL);

    std::cerr << v0 << " == " << v1 << " -> "
              << std::boolalpha << util::apply_visitor(v0, v1, test::javascript_equal_visitor()) << std::endl;


    std::vector<variant_type> vec;

    vec.emplace_back(std::string("1"));
    vec.push_back(variant_type(2ULL));
    vec.push_back(variant_type(3ULL));
    vec.push_back(std::string("3.14159"));
    vec.emplace_back(3.14159);

    //auto itr = std::find_if(vec.begin(), vec.end(), [&v0](variant_type const& val) {
    //        return util::apply_visitor(v0, val, test::javascript_equal_visitor());
    //    });

    auto itr = std::find_if(vec.begin(), vec.end(),test::javascript_equal<variant_type>(v2));

    if (itr != std::end(vec))
    {
        std::cout << "found " << *itr << std::endl;
    }
    else
    {
        std::cout << "can't find " << v2 << '\n';
    }

    return EXIT_SUCCESS;
}
