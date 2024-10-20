#pragma once

#include <limits>

// from: https://gist.github.com/pps83/3210a2f980fd02bb2ba2e5a1fc4a2ef0

// Note, bsf/bsr are used by default.
// Enable /arch:AVX2 compilation for better optimizations

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>

static __forceinline int __builtin_clz(unsigned x)
{
#if defined(_M_ARM) || defined(_M_ARM64) || defined(_M_HYBRID_X86_ARM64) || defined(_M_ARM64EC)
    return (int)_CountLeadingZeros(x);
#elif defined(__AVX2__) || defined(__LZCNT__)
    return (int)_lzcnt_u32(x);
#else
    unsigned long r;
    _BitScanReverse(&r, x);
    return (int)(r ^ 31);
#endif
}

static __forceinline int __builtin_clzll(unsigned long long x)
{
#if defined(_M_ARM) || defined(_M_ARM64) || defined(_M_HYBRID_X86_ARM64) || defined(_M_ARM64EC)
    return (int)_CountLeadingZeros64(x);
#elif defined(_WIN64)
#if defined(__AVX2__) || defined(__LZCNT__)
    return (int)_lzcnt_u64(x);
#else
    unsigned long r;
    _BitScanReverse64(&r, x);
    return (int)(r ^ 63);
#endif
#else
    int l = __builtin_clz((unsigned)x) + 32;
    int h = __builtin_clz((unsigned)(x >> 32));
    return !!((unsigned)(x >> 32)) ? h : l;
#endif
}

static __forceinline int __builtin_clzl(unsigned long x)
{
    return sizeof(x) == 8 ? __builtin_clzll(x) : __builtin_clz((unsigned)x);
}
#endif // defined(_MSC_VER) && !defined(__clang__)

template <typename _Tp>
constexpr int __countl_zero(_Tp __x) noexcept
{
    constexpr auto _Nd = std::numeric_limits<_Tp>::digits;

    if (__x == 0) return _Nd;

    constexpr auto _Nd_ull = std::numeric_limits<unsigned long long>::digits;
    constexpr auto _Nd_ul  = std::numeric_limits<unsigned long>::digits;
    constexpr auto _Nd_u   = std::numeric_limits<unsigned>::digits;

    if constexpr (_Nd <= _Nd_u) {
        constexpr int __diff = _Nd_u - _Nd;
        return __builtin_clz(__x) - __diff;
    } else if constexpr (_Nd <= _Nd_ul) {
        constexpr int __diff = _Nd_ul - _Nd;
        return __builtin_clzl(__x) - __diff;
    } else if constexpr (_Nd <= _Nd_ull) {
        constexpr int __diff = _Nd_ull - _Nd;
        return __builtin_clzll(__x) - __diff;
    } else // (_Nd > _Nd_ull)
    {
        static_assert(_Nd <= (2 * _Nd_ull), "Maximum supported integer size is 128-bit");

        unsigned long long __high = __x >> _Nd_ull;
        if (__high != 0) {
            constexpr int __diff = (2 * _Nd_ull) - _Nd;
            return __builtin_clzll(__high) - __diff;
        }
        constexpr auto     __max_ull = std::numeric_limits<unsigned long long>::max();
        unsigned long long __low     = __x & __max_ull;
        return (_Nd - _Nd_ull) + __builtin_clzll(__low);
    }
}