// SPDX-License-Identifier: MIT
/* Copyright 2025 Alexandro Sanchez Bach <alexandro@phi.nz> */

#ifndef ANE_BITFIELD_H_
#define ANE_BITFIELD_H_

#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace ane {

/**
 * Bitfield
 * ========
 * Bitfield of a given underlying integral type.
 * @tparam  T  Integral type
 * @tparam  N  Field index within its container
 * @tparam  S  Field length within its container
 */
template <typename T, std::size_t N, std::size_t S>
struct Bitfield {
private:
    T data;

    constexpr static std::size_t bits_used = S;
    constexpr static std::size_t bits_data = sizeof(T) * CHAR_BIT;
    constexpr static std::size_t bits_free = bits_data - bits_used;

public:
    // Bounds
    constexpr static T max = std::numeric_limits<T>::max() >> bits_free;
    constexpr static T min = std::numeric_limits<T>::min() >> bits_free;

    constexpr static T mask  = ((UINT64_C(1) << S) - 1) << N;
    constexpr static T shift = N;

    operator T() const {
        if constexpr (std::is_signed_v<T>) {
            return ((data & mask) << (bits_free - shift)) >> bits_free;
        } else {
            return (data & mask) >> shift;
        }
    }
    template <typename U>
    explicit operator U() const {
        return static_cast<U>(T(*this));
    }

    Bitfield& operator=(T value) {
        assert(value >= min);
        assert(value <= max);

        data = (data & ~mask) | ((value << shift) & mask);
        return *this;
    }

    Bitfield& operator+=(const T rhs) {
        return *this = T(*this) + rhs;
    }

    Bitfield& operator-=(const T rhs) {
        return *this = T(*this) - rhs;
    }

private:
    // Static checks
    static_assert(std::is_integral_v<T>, "Invalid type: Underlying type should be integral");
    static_assert(std::is_unsigned_v<T> || S > 1, "Invalid size: Field of size one cannot be signed");
    static_assert(N + S <= sizeof(T) * 8, "Invalid mask: Field exceeds data bounds");
    static_assert(S > 0, "Invalid mask: Field length should be positive");
};

/**
 * Bitrange
 * ========
 * Alias for a Bitfield given its start and end bit-indices.
 * @tparam  T  Integral type
 * @tparam  S  First bit index
 * @tparam  E  Last bit index
 */
template <typename T, std::size_t S, std::size_t E>
using Bitrange = Bitfield<T, S, E - S + 1>;

/**
 * Bit
 * ===
 * Alias for a one-bit Bitfield.
 * @tparam  T  Integral type
 * @tparam  N  Bit index within its container
 */
template <typename T, std::size_t N>
using Bit = Bitfield<T, N, 1>;

} // namespace ane

#endif // !ANE_BITFIELD_H_