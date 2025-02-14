#pragma once

#include <type_traits>

namespace details {
template <typename... Types>
struct traits {
  static constexpr bool copy_constructible = std::conjunction_v<std::is_copy_constructible<Types>...>;
  static constexpr bool trivial_copy_ctor = std::conjunction_v<std::is_trivially_copy_constructible<Types>...>;
  static constexpr bool basic_copy_ctor = copy_constructible && !trivial_copy_ctor;
  static constexpr bool nothrow_copy_ctor = std::conjunction_v<std::is_nothrow_copy_constructible<Types>...>;

  static constexpr bool move_constructible = std::conjunction_v<std::is_move_constructible<Types>...>;
  static constexpr bool trivial_move_ctor = std::conjunction_v<std::is_trivially_move_constructible<Types>...>;
  static constexpr bool basic_move_ctor =
      std::conjunction_v<std::is_move_constructible<Types>...> && !trivial_move_ctor;
  static constexpr bool nothrow_move_ctor = std::conjunction_v<std::is_nothrow_move_constructible<Types>...>;

  static constexpr bool trivial_copy_assign =
      std::conjunction_v<std::is_trivially_copy_assignable<Types>...> && trivial_copy_ctor;
  static constexpr bool basic_copy_assign =
      std::conjunction_v<std::is_copy_assignable<Types>...> && copy_constructible && !trivial_copy_assign;
  static constexpr bool nothrow_copy_assign =
      std::conjunction_v<std::is_nothrow_copy_assignable<Types>...> && nothrow_copy_ctor;

  static constexpr bool trivial_move_assign =
      std::conjunction_v<std::is_trivially_move_assignable<Types>...> && trivial_move_ctor;
  static constexpr bool basic_move_assign =
      std::conjunction_v<std::is_move_assignable<Types>...> && move_constructible && !trivial_move_assign;
  static constexpr bool nothrow_move_assign =
      std::conjunction_v<std::is_nothrow_move_assignable<Types>...> && nothrow_move_ctor;

  static constexpr bool trivial_dtor = std::conjunction_v<std::is_trivially_destructible<Types>...>;
};
} // namespace details
