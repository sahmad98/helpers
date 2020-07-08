#ifndef _FREQUENCY_HELPERS_HPP_
#define _FREQUENCY_HELPERS_HPP_

#ifdef __AVX512F__

#include <immintrin.h>

enum FreqLevel {
	CORE_LEVEL0,
	CORE_LEVEL1,
	CORE_LEVEL2,
};

__attribute__((noinline)) auto heavy_fma() {
	volatile __m512 result, temp;
	for (int i=0; i<32; i++) {
		result = _mm512_fmadd_ps(temp, temp, temp);
	}
	return result;
}

template<FreqLevel level>
__attribute__((noinline)) auto lock_freq() {
	if constexpr (level == FreqLevel::CORE_LEVEL1) {
		__m512i temp;
		volatile __m512i result = _mm512_xor_epi32(temp, temp);
		return result;
	} else if constexpr (level == FreqLevel::CORE_LEVEL2){
        // NOTE: Might require heavy FMA before changing the power
        // level so this need to call heavy fma. Measure and Use.
        heavy_fma();
		__m512 temp;
		volatile __m512 result = _mm512_fmadd_ps(temp, temp, temp);
		return result;
	}
}

#endif

#endif