/*
 * Copyright (c) 2020 Louis Langholtz https://github.com/louis-langholtz/PlayRho
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

#ifndef PLAYRHO_COMMON_ARRAYALLOCATOR_HPP
#define PLAYRHO_COMMON_ARRAYALLOCATOR_HPP

#include <utility>
#include <vector>
#include <type_traits>

namespace playrho {

/// @brief Array allocator.
template <typename T>
class ArrayAllocator
{
public:
    /// @brief Element type.
    using value_type = T;

    /// @brief Size type.
    using size_type = typename std::vector<value_type>::size_type;

    /// @brief Reference type alias.
    using reference = typename std::vector<value_type>::reference;

    /// @brief Constant reference type alias.
    using const_reference = typename std::vector<value_type>::const_reference;

    /// @brief Gets the index of the given pointer.
    /// @return -1 if the given pointer is not within the range of the allocator's allocation,
    ///    otherwise return index of pointer within allocator.
    size_type GetIndex(value_type* ptr) const
    {
        const auto i = ptr - m_data.data();
        return static_cast<size_type>(((i >= 0) && (static_cast<size_type>(i) < m_data.size()))? i: -1);
    }

    /// @brief Allocates an entry in the array with the given constructor parameters.
    template< class... Args >
    size_type Allocate(Args&&... args)
    {
        if (!m_free.empty())
        {
            const auto index = m_free.back();
            m_data[index] = value_type{std::forward<Args>(args)...};
            m_free.pop_back();
            return index;
        }
        const auto index = m_data.size();
        m_data.emplace_back(std::forward<Args>(args)...);
        return index;
    }

    /// @brief Allocates an entry in the array with the given instance.
    size_type Allocate(const value_type& copy)
    {
        if (!m_free.empty())
        {
            const auto index = m_free.back();
            m_data[index] = copy;
            m_free.pop_back();
            return index;
        }
        const auto index = m_data.size();
        m_data.push_back(copy);
        return index;
    }

    /// @brief Frees the specified index entry.
    void Free(size_type index)
    {
        if (index != static_cast<size_type>(-1))
        {
            m_data[index] = value_type{};
            m_free.push_back(index);
        }
    }

    /// @brief Array index operator.
    reference operator[](size_type pos)
    {
        return m_data[pos];
    }

    /// @brief Constant array index operator.
    const_reference operator[](size_type pos) const
    {
        return m_data[pos];
    }

    /// @brief Bounds checking indexed array accessor.
    reference at(size_type pos)
    {
        return m_data.at(pos);
    }

    /// @brief Bounds checking indexed array accessor.
    const_reference at(size_type pos) const
    {
        return m_data.at(pos);
    }

    /// @brief Gets the size of this instance in number of elements.
    size_type size() const noexcept
    {
        return m_data.size();
    }

    /// @brief Gets the maximum theoretical size this instance can have in number of elements.
    size_type max_size() const noexcept
    {
        return m_data.max_size();
    }

    /// @brief Gets the number of elements currently free.
    size_type free() const noexcept
    {
        return m_free.size();
    }

    /// @brief Reserves the given number of elements from dynamic memory.
    void reserve(size_type value)
    {
        m_data.reserve(value);
    }

    /// @brief Clears this instance's free pool and allocated pool.
    void clear() noexcept
    {
        m_data.clear();
        m_free.clear();
    }

private:
    std::vector<value_type> m_data; ///< Array data (both used & free).
    std::vector<typename std::vector<value_type>::size_type> m_free; ///< Indices of free elements.
};

/// @brief Gets the number of elements that are used in the specified structure.
/// @return Size of the specified structure minus the size of its free pool.
template <typename T>
typename ArrayAllocator<T>::size_type used(const ArrayAllocator<T>& array) noexcept
{
    return array.size() - array.free();
}

} // namespace playrho

#endif // PLAYRHO_COMMON_ARRAYALLOCATOR_HPP
