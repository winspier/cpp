#include "typelist.h"

#include <catch2/catch_test_macros.hpp>

#include <tuple>
#include <type_traits>

// clang-format off

TEST_CASE("Transform")
{
    static_assert(std::is_same_v<
        tl::transform<std::add_pointer_t, tl::type_list<int, double>>,
        tl::type_list<int *, double *>
    >);

    static_assert(std::is_same_v<
        tl::transform<std::vector, tl::type_list<int, double>>,
        tl::type_list<std::vector<int>, std::vector<double>>
    >);
}

TEST_CASE("Apply")
{
    static_assert(std::is_same_v<
        tl::apply<std::common_type_t, tl::type_list<int, double>>,
        double
    >);

    static_assert(std::is_same_v<
        tl::apply<std::common_type_t, std::tuple<int, double>>,
        double
    >);

    static_assert(std::is_same_v<
        tl::apply<
            std::tuple,
            tl::transform<
                std::add_pointer_t,
                tl::apply<tl::type_list, std::tuple<int, double>>
            >
        >,
        std::tuple<int *, double *>
    >);
}

TEST_CASE("Push back")
{
    static_assert(std::is_same_v<
        tl::push_back<float, tl::type_list<int, double>>,
        tl::type_list<int, double, float>
    >);
}

TEST_CASE("Concat2")
{
    static_assert(std::is_same_v<
        tl::concat2<tl::type_list<float>, tl::type_list<int, double>>,
        tl::type_list<float, int, double>
    >);
    static_assert(std::is_same_v<
        tl::concat2<tl::type_list<>, tl::type_list<>>,
        tl::type_list<>
    >);
}

TEST_CASE("Pop front")
{
    static_assert(std::is_same_v<
        tl::pop_front<tl::type_list<float, int, double>>,
        tl::type_list<int, double>
    >);
}

TEST_CASE("Pop back")
{
    static_assert(std::is_same_v<
        tl::pop_back<tl::type_list<float, int, double>>,
        tl::type_list<float, int>
    >);
}

TEST_CASE("Concat")
{
    static_assert(std::is_same_v<
        tl::concat<tl::type_list<double>, tl::type_list<float, int>, tl::type_list<char>>,
        tl::type_list<double, float, int, char>
    >);
    static_assert(std::is_same_v<
        tl::concat<>,
        tl::type_list<>
    >);
}

TEST_CASE("Concat fast")
{
    static_assert(std::is_same_v<
        tl::concat_fast<tl::type_list<double>, tl::type_list<float, int>, tl::type_list<char>>,
        tl::type_list<double, float, int, char>
    >);
    static_assert(std::is_same_v<
        tl::concat_fast<>,
        tl::type_list<>
    >);
}

TEST_CASE("Filter")
{
    static_assert(std::is_same_v<
        tl::filter<std::is_pointer, tl::type_list<int *, double, float *, long>>,
        tl::type_list<int *, float *>
    >);
    static_assert(std::is_same_v<
        tl::filter<std::is_pointer, tl::type_list<>>,
        tl::type_list<>
    >);
}

TEST_CASE("Get nth")
{
    static_assert(std::is_same_v<tl::get_nth<0, tl::type_list<int, double, float>>, int>);
    static_assert(std::is_same_v<tl::get_nth<1, tl::type_list<int, double, float>>, double>);
    static_assert(std::is_same_v<tl::get_nth<2, tl::type_list<int, double, float>>, float>);

    static_assert(std::is_same_v<tl::get_nth<0, tl::type_list<int>>, int>);
}

static_assert(std::is_same_v<
    std::make_index_sequence<5>,
    std::index_sequence<0, 1, 2, 3, 4>
>);

TEST_CASE("Map find")
{
    using map = tl::type_list<
        tl::type_list<int, int>,        // key = int
        tl::type_list<double, float>,   // key = double
        tl::type_list<char, int, long>, // key = char
        tl::type_list<void>             // key = void
    >;

    static_assert(std::is_same_v<
        tl::map_find<int, map>,
        tl::type_list<int, int>
    >);

    static_assert(std::is_same_v<
        tl::map_find<char, map>,
        tl::type_list<char, int, long>
    >);
}

TEST_CASE("Enumerate")
{
    using list = tl::type_list<int, double, long>;

    using map = tl::type_list<
        tl::type_list<tl::index<0>, int>,
        tl::type_list<tl::index<1>, double>,
        tl::type_list<tl::index<2>, long>
    >;

    static_assert(std::is_same_v<tl::enumerate<list>, map>);
}

TEST_CASE("Get nth (fast)")
{
    static_assert(std::is_same_v<tl::get_nth_fast<0, tl::type_list<int, double, float>>, int>);
    static_assert(std::is_same_v<tl::get_nth_fast<1, tl::type_list<int, double, float>>, double>);
    static_assert(std::is_same_v<tl::get_nth_fast<2, tl::type_list<int, double, float>>, float>);

    static_assert(std::is_same_v<tl::get_nth_fast<0, tl::type_list<int>>, int>);
}
