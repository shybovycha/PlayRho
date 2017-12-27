/*
 * Copyright (c) 2017 Louis Langholtz https://github.com/louis-langholtz/PlayRho
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef PLAYRHO_COMMON_UNITVEC2_HPP
#define PLAYRHO_COMMON_UNITVEC2_HPP

/// @file
/// Declarations of the UnitVec2 class and free functions associated with it.

#include <PlayRho/Common/Settings.hpp>
#include <PlayRho/Common/InvalidArgument.hpp>
#include <iostream>
#include <utility>

namespace playrho
{
/// @brief 2-D unit vector.
/// @details This is a 2-dimensional directional vector.
class UnitVec2
{
public:
    /// @brief Value type used for the coordinate values of this vector.
    using value_type = Real;
    
    /// @brief Size type.
    using size_type = std::size_t;
    
    /// @brief Constant reference type.
    using const_reference = const value_type&;
    
    /// @brief Constant pointer type.
    using const_pointer = const value_type*;
    
    /// @brief Constant iterator type.
    using const_iterator = const value_type*;
    
    /// @brief Constant reverse iterator type.
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    
    /// @brief Gets the right-ward oriented unit vector.
    /// @note This is the value for the 0/4 turned (0 angled) unit vector.
    /// @note This is the reverse perpendicular unit vector of the bottom oriented vector.
    /// @note This is the forward perpendicular unit vector of the top oriented vector.
    static PLAYRHO_CONSTEXPR inline UnitVec2 GetRight() noexcept { return UnitVec2{1, 0}; }

    /// @brief Gets the top-ward oriented unit vector.
    /// @note This is the actual value for the 1/4 turned (90 degree angled) unit vector.
    /// @note This is the reverse perpendicular unit vector of the right oriented vector.
    /// @note This is the forward perpendicular unit vector of the left oriented vector.
    static PLAYRHO_CONSTEXPR inline UnitVec2 GetTop() noexcept { return UnitVec2{0, 1}; }

    /// @brief Gets the left-ward oriented unit vector.
    /// @note This is the actual value for the 2/4 turned (180 degree angled) unit vector.
    /// @note This is the reverse perpendicular unit vector of the top oriented vector.
    /// @note This is the forward perpendicular unit vector of the bottom oriented vector.
    static PLAYRHO_CONSTEXPR inline UnitVec2 GetLeft() noexcept { return UnitVec2{-1, 0}; }

    /// @brief Gets the bottom-ward oriented unit vector.
    /// @note This is the actual value for the 3/4 turned (270 degree angled) unit vector.
    /// @note This is the reverse perpendicular unit vector of the left oriented vector.
    /// @note This is the forward perpendicular unit vector of the right oriented vector.
    static PLAYRHO_CONSTEXPR inline UnitVec2 GetBottom() noexcept { return UnitVec2{0, -1}; }

    /// @brief Gets the non-oriented unit vector.
    static PLAYRHO_CONSTEXPR inline UnitVec2 GetZero() noexcept { return UnitVec2{0, 0}; }

    /// @brief Gets the 45 degree unit vector.
    /// @details This is the unit vector in the positive X and Y quadrant where X == Y.
    static PLAYRHO_CONSTEXPR inline UnitVec2 GetTopRight() noexcept
    {
        return UnitVec2{+SquareRootTwo/Real(2), +SquareRootTwo/Real(2)};
    }
    
    /// @brief Gets the -45 degree unit vector.
    /// @details This is the unit vector in the positive X and negative Y quadrant
    ///   where |X| == |Y|.
    static PLAYRHO_CONSTEXPR inline UnitVec2 GetBottomRight() noexcept
    {
        return UnitVec2{+SquareRootTwo/Real(2), -SquareRootTwo/Real(2)};
    }
    
    /// @brief Gets the default fallback.
    static PLAYRHO_CONSTEXPR inline UnitVec2 GetDefaultFallback() noexcept { return UnitVec2{}; }

    /// @brief Polar coordinate.
    /// @details This is a direction and magnitude pair defined by the unit vector class.
    /// @note A magnitude of 0 indicates that no conclusive direction could be determined.
    ///   The magnitude will otherwise be a normal value.
    using PolarCoord = std::pair<UnitVec2, Real>;

    /// @brief Gets the unit vector & magnitude from the given parameters.
    static PolarCoord Get(const Real x, const Real y,
                          const UnitVec2 fallback = GetDefaultFallback()) noexcept;

    /// @brief Gets the given angled unit vector.
    ///
    /// @note For angles that are meant to be at exact multiples of the quarter turn,
    ///   better accuracy will be had by using one of the four oriented unit
    ///   vector returning methods - for the right, top, left, bottom orientations.
    ///
    static UnitVec2 Get(const Angle angle) noexcept;

    PLAYRHO_CONSTEXPR inline UnitVec2() noexcept = default;
    
    /// @brief Gets the max size.
    PLAYRHO_CONSTEXPR inline size_type max_size() const noexcept { return size_type{2}; }
    
    /// @brief Gets the size.
    PLAYRHO_CONSTEXPR inline size_type size() const noexcept { return size_type{2}; }
    
    /// @brief Whether empty.
    /// @note Always false for N > 0.
    PLAYRHO_CONSTEXPR inline bool empty() const noexcept { return false; }
    
    /// @brief Gets a "begin" iterator.
    const_iterator begin() const noexcept { return const_iterator(m_elems); }
    
    /// @brief Gets an "end" iterator.
    const_iterator end() const noexcept { return const_iterator(m_elems + 2); }
    
    /// @brief Gets a "begin" iterator.
    const_iterator cbegin() const noexcept { return begin(); }
    
    /// @brief Gets an "end" iterator.
    const_iterator cend() const noexcept { return end(); }
    
    /// @brief Gets a reverse "begin" iterator.
    const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator{m_elems + 2};
    }
    
    /// @brief Gets a reverse "end" iterator.
    const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator{m_elems};
    }
    
    /// @brief Gets a reverse "begin" iterator.
    const_reverse_iterator rbegin() const noexcept
    {
        return crbegin();
    }
    
    /// @brief Gets a reverse "end" iterator.
    const_reverse_iterator rend() const noexcept
    {
        return crend();
    }
    
    /// @brief Gets a constant reference to the requested element.
    /// @note No bounds checking is performed.
    /// @warning Behavior is undefined if given a position equal to or greater than size().
    PLAYRHO_CONSTEXPR inline const_reference operator[](size_type pos) const noexcept
    {
        assert(pos < size());
        return m_elems[pos];
    }
    
    /// @brief Gets a constant reference to the requested element.
    /// @throws InvalidArgument if given a position that's >= size().
    PLAYRHO_CONSTEXPR inline const_reference at(size_type pos) const
    {
        if (pos >= size())
        {
            throw InvalidArgument("Vector::at: position >= size()");
        }
        return m_elems[pos];
    }
    
    /// @brief Direct access to data.
    PLAYRHO_CONSTEXPR inline const_pointer data() const noexcept
    {
        return m_elems;
    }
    
    /// @brief Gets the "X" value.
    PLAYRHO_CONSTEXPR inline auto GetX() const noexcept { return m_elems[0]; }

    /// @brief Gets the "Y" value.
    PLAYRHO_CONSTEXPR inline auto GetY() const noexcept { return m_elems[1]; }
    
    /// @brief Flips the X and Y values.
    PLAYRHO_CONSTEXPR inline UnitVec2 FlipXY() const noexcept { return UnitVec2{-GetX(), -GetY()}; }

    /// @brief Flips the X value.
    PLAYRHO_CONSTEXPR inline UnitVec2 FlipX() const noexcept { return UnitVec2{-GetX(), GetY()}; }

    /// @brief Flips the Y value.
    PLAYRHO_CONSTEXPR inline UnitVec2 FlipY() const noexcept { return UnitVec2{GetX(), -GetY()}; }

    /// @brief Rotates the unit vector by the given amount.
    ///
    /// @param amount Expresses the angular difference from the right-ward oriented unit
    ///   vector to rotate this unit vector by.
    ///
    /// @return Result of rotating this unit vector by the given amount.
    ///
    PLAYRHO_CONSTEXPR inline UnitVec2 Rotate(UnitVec2 amount) const noexcept
    {
        return UnitVec2{GetX() * amount.GetX() - GetY() * amount.GetY(),
                        GetY() * amount.GetX() + GetX() * amount.GetY()};
    }

    /// @brief Gets a vector counter-clockwise (reverse-clockwise) perpendicular to this vector.
    /// @details This returns the unit vector (-y, x).
    /// @return A counter-clockwise 90-degree rotation of this vector.
    /// @sa GetFwdPerpendicular.
    PLAYRHO_CONSTEXPR inline UnitVec2 GetRevPerpendicular() const noexcept
    {
        // See http://mathworld.wolfram.com/PerpendicularVector.html
        return UnitVec2{-GetY(), GetX()};
    }

    /// @brief Gets a vector clockwise (forward-clockwise) perpendicular to this vector.
    /// @details This returns the unit vector (y, -x).
    /// @return A clockwise 90-degree rotation of this vector.
    /// @sa GetRevPerpendicular.
    PLAYRHO_CONSTEXPR inline UnitVec2 GetFwdPerpendicular() const noexcept
    {
        // See http://mathworld.wolfram.com/PerpendicularVector.html
        return UnitVec2{GetY(), -GetX()};
    }

    /// @brief Negation operator.
    PLAYRHO_CONSTEXPR inline UnitVec2 operator-() const noexcept { return UnitVec2{-GetX(), -GetY()}; }

    /// @brief Positive operator.
    PLAYRHO_CONSTEXPR inline UnitVec2 operator+() const noexcept { return UnitVec2{+GetX(), +GetY()}; }

    /// @brief Gets the absolute value.
    PLAYRHO_CONSTEXPR inline UnitVec2 Absolute() const noexcept
    {
        return UnitVec2{Abs(GetX()), Abs(GetY())};
    }

private:
    
    /// @brief Initializing constructor.
    PLAYRHO_CONSTEXPR inline UnitVec2(value_type x, value_type y) noexcept : m_elems{x, y}
    {
        // Intentionally empty.
    }

    value_type m_elems[2] = { value_type{0}, value_type{0} }; ///< Element values.
};

/// @brief Gets the "X-axis".
PLAYRHO_CONSTEXPR inline UnitVec2 GetXAxis(UnitVec2 rot) noexcept { return rot; }

/// @brief Gets the "Y-axis".
/// @note This is the reverse perpendicular vector of the given unit vector.
PLAYRHO_CONSTEXPR inline UnitVec2 GetYAxis(UnitVec2 rot) noexcept { return rot.GetRevPerpendicular(); }

/// @brief Equality operator.
PLAYRHO_CONSTEXPR inline bool operator==(const UnitVec2 a, const UnitVec2 b) noexcept
{
    return (a.GetX() == b.GetX()) && (a.GetY() == b.GetY());
}

/// @brief Inequality operator.
PLAYRHO_CONSTEXPR inline bool operator!=(const UnitVec2 a, const UnitVec2 b) noexcept
{
    return (a.GetX() != b.GetX()) || (a.GetY() != b.GetY());
}

/// @brief Gets a vector counter-clockwise (reverse-clockwise) perpendicular to the
///   given vector.
/// @details This takes a vector of form (x, y) and returns the vector (-y, x).
/// @param vector Vector to return a counter-clockwise perpendicular equivalent for.
/// @return A counter-clockwise 90-degree rotation of the given vector.
/// @sa GetFwdPerpendicular.
PLAYRHO_CONSTEXPR inline UnitVec2 GetRevPerpendicular(const UnitVec2 vector) noexcept
{
    return vector.GetRevPerpendicular();
}

/// @brief Gets a vector clockwise (forward-clockwise) perpendicular to the given vector.
/// @details This takes a vector of form (x, y) and returns the vector (y, -x).
/// @param vector Vector to return a clockwise perpendicular equivalent for.
/// @return A clockwise 90-degree rotation of the given vector.
/// @sa GetRevPerpendicular.
PLAYRHO_CONSTEXPR inline UnitVec2 GetFwdPerpendicular(const UnitVec2 vector) noexcept
{
    return vector.GetFwdPerpendicular();
}

/// @brief Rotates a unit vector by the angle expressed by the second unit vector.
/// @return Unit vector for the angle that's the sum of the two angles expressed by
///   the input unit vectors.
PLAYRHO_CONSTEXPR inline UnitVec2 Rotate(const UnitVec2 vector, const UnitVec2& angle) noexcept
{
    return vector.Rotate(angle);
}

/// @brief Inverse rotates a vector.
PLAYRHO_CONSTEXPR inline UnitVec2 InverseRotate(const UnitVec2 vector, const UnitVec2& angle) noexcept
{
    return vector.Rotate(angle.FlipY());
}
    
/// @brief Gets an invalid value for the <code>UnitVec2</code> type.
template <> PLAYRHO_CONSTEXPR inline UnitVec2 GetInvalid() noexcept { return UnitVec2{}; }

/// @brief Determines if the given value is valid.
template <> PLAYRHO_CONSTEXPR inline bool IsValid(const UnitVec2& value) noexcept
{
    return IsValid(value.GetX()) && IsValid(value.GetY()) && (value != UnitVec2::GetZero());
}

/// @brief Gets the specified element of the given collection.
template <size_t I>
PLAYRHO_CONSTEXPR inline UnitVec2::value_type Get(UnitVec2 v) noexcept
{
    static_assert(I < 2, "Index out of bounds in playrho::Get<> (playrho::UnitVec2)");
    switch (I)
    {
        case 0: return v.GetX();
        case 1: return v.GetY();
    }
}

/// @brief Gets element 0 of the given collection.
template <>
PLAYRHO_CONSTEXPR inline UnitVec2::value_type Get<0>(UnitVec2 v) noexcept
{
    return v.GetX();
}

/// @brief Gets element 1 of the given collection.
template <>
PLAYRHO_CONSTEXPR inline UnitVec2::value_type Get<1>(UnitVec2 v) noexcept
{
    return v.GetY();
}

/// @brief Output stream operator.
inline ::std::ostream& operator<<(::std::ostream& os, const UnitVec2& value)
{
    return os << "UnitVec2(" << Get<0>(value) << "," << Get<1>(value) << ")";
}

} // namespace playrho

namespace std {

/// @brief Tuple size info for <code>UnitVec2</code>.
template<>
class tuple_size< playrho::UnitVec2 >: public std::integral_constant<size_t, 2> {};

/// @brief Tuple element type info for <code>UnitVec2</code>.
template<std::size_t I>
class tuple_element<I, playrho::UnitVec2>
{
public:
    /// @brief Type alias revealing the actual type of the element.
    using type = playrho::Real;
};

} // namespace std

#endif // PLAYRHO_COMMON_UNITVEC2_HPP
