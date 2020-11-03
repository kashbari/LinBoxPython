/* -*- mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
// vim:sts=4:sw=4:ts=4:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
/*
 * Copyright (C) 2014 the FFLAS-FFPACK group
 *
 * Written by   Bastien Vialla<bastien.vialla@lirmm.fr>
 * Brice Boyer (briceboyer) <boyer.brice@gmail.com>
 * Romain Lebreton <romain.lebreton@lirmm.fr>
 *
 *
 * ========LICENCE========
 * This file is part of the library FFLAS-FFPACK.
 *
 * FFLAS-FFPACK is free software: you can redistribute it and/or modify
 * it under the terms of the  GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========
 *.
 */

#ifndef __FFLASFFPACK_fflas_ffpack_utils_simd128_int16_INL
#define __FFLASFFPACK_fflas_ffpack_utils_simd128_int16_INL

#ifndef __FFLASFFPACK_HAVE_SSE4_1_INSTRUCTIONS
#error "You need SSE instructions to perform 128 bits operations on int16"
#endif

/*
 * Simd128 specialized for int16_t
 */
template <> struct Simd128_impl<true, true, true, 2> : public Simd128i_base {

	/*
	* alias to 128 bit simd register
	*/
	using vect_t = __m128i;

	/*
	* define the scalar type corresponding to the specialization
	*/
	using scalar_t = int16_t;

	/*
	*  number of scalar_t in a simd register
	*/
	static const constexpr size_t vect_size = 8;

	/*
	*  alignement required by scalar_t pointer to be loaded in a vect_t
	*/
	static const constexpr size_t alignment = 16;

	/*
	* Check if the pointer p is a multiple of alignemnt
	*/
	template <class T> static constexpr bool valid(T *p) { return (int64_t)p % alignment == 0; }

	/*
	* Check if the number n is a multiple of vect_size
	*/
	template <class T> static constexpr bool compliant(T n) { return n % vect_size == 0; }

	/*
	* Converter from vect_t to a tab.
	* exple:
	*	Converter conv;
	*	conv.v = a;
	*	scalart_t x = conv.t[1]
	*/
	union Converter {
		vect_t v;
		scalar_t t[vect_size];
	};

	/*
	*  Broadcast 16-bit integer a to all elements of dst. This intrinsic may generate the vpbroadcastw.
	*  Return [x,x,x,x,x,x,x,x] int16_t
	*/
	static INLINE CONST vect_t set1(const scalar_t x) { return _mm_set1_epi16(x); }

	/*
	*  Set packed 16-bit integers in dst with the supplied values.
	*  Return [x0, ..., x7] int16_t
	*/
	static INLINE CONST vect_t set(const scalar_t x0, const scalar_t x1, const scalar_t x2, const scalar_t x3,
								   const scalar_t x4, const scalar_t x5, const scalar_t x6, const scalar_t x7) {
		return _mm_set_epi16(x7, x6, x5, x4, x3, x2, x1, x0);
	}

	/*
	*  Gather 16-bit integer elements with indexes idx[0], ..., idx[7] from the address p in vect_t.
	*  Return [p[idx[0]], ..., p[idx[7]]] int16_t
	*/
	template <class T> static INLINE PURE vect_t gather(const scalar_t *const p, const T *const idx) {
		return set(p[idx[0]], p[idx[1]], p[idx[2]], p[idx[3]], p[idx[4]], p[idx[5]], p[idx[6]], p[idx[7]]);
	}

	/*
	* Load 128-bits of integer data from memory into dst.
	* p must be aligned on a 16-byte boundary or a general-protection exception will be generated.
	* Return [p[0], ..., p[7]] int16_t
	*/
	static INLINE PURE vect_t load(const scalar_t *const p) {
		return _mm_load_si128(reinterpret_cast<const vect_t *>(p));
	}

	/*
	* Load 128-bits of integer data from memory into dst.
	* p does not need to be aligned on any particular boundary.
	* Return [p[0], ..., p[7]] int16_t
	*/
	static INLINE PURE vect_t loadu(const scalar_t *const p) {
		return _mm_loadu_si128(reinterpret_cast<const vect_t *>(p));
	}

	/*
	* Store 128-bits of integer data from a into memory.
	* p must be aligned on a 16-byte boundary or a general-protection exception will be generated.
	*/
	static INLINE void store(scalar_t *p, vect_t v) {
		_mm_store_si128(reinterpret_cast<vect_t *>(p), v);
	}

	/*
	* Store 128-bits of integer data from a into memory.
	* p does not need to be aligned on any particular boundary.
	*/
	static INLINE void storeu(scalar_t *p, vect_t v) {
		_mm_storeu_si128(reinterpret_cast<vect_t *>(p), v);
	}

	/*
	* Store 128-bits of integer data from a into memory using a non-temporal memory hint.
	* p must be aligned on a 16-byte boundary or a general-protection exception may be generated.
	*/
	static INLINE void stream(scalar_t *p, const vect_t v) {
		_mm_stream_si128(reinterpret_cast<vect_t *>(p), v);
	}

	/*
	* Shift packed 16-bit integers in a left by s while shifting in zeros, and store the results in vect_t.
	* Args   :	[a0, ..., a7] int16_t
	* Return :	[a0 << s, a1 << s, a2 << s, a3 << s, a4 << s, a5 << s, a6 << s, a7 << s] int16_t
	*/
	static INLINE CONST vect_t sll(const vect_t a, const int s) { return _mm_slli_epi16(a, s); }

	/*
	* Shift packed 16-bit integers in a right by s while shifting in zeros, and store the results in vect_t.
	* Args   :	[a0, ..., a7] int16_t
	* Return :	[a0 >> s, a1 >> s, a2 >> s, a3 >> s, a4 >> s, a5 >> s, a6 >> s, a7 >> s] int16_t
	*/
	static INLINE CONST vect_t srl(const vect_t a, const int s) { return _mm_srli_epi16(a, s); }

	/*
	* Shift packed 16-bit integers in a right by s while shifting in sign bits, and store the results in vect_t.
	* Args   :	[a0, ..., a7] int16_t
	* Return :	[a0 >> s, a1 >> s, a2 >> s, a3 >> s, a4 >> s, a5 >> s, a6 >> s, a7 >> s] int16_t
	*/
	static INLINE CONST vect_t sra(const vect_t a, const int s) { return _mm_srai_epi16(a, s); }

	/*
	* Shuffle 16-bit integers in a using the control in imm8, and store the results in dst.
	* Args   :	[a0, ..., a7] int16_t
	* Return :	[a[s[0..3]], ..., a[s[28..31]] int16_t
	*/
	template<uint32_t s>
	static INLINE CONST vect_t shuffle(const vect_t a) {
		//#pragma warning "The simd shuffle function is emulated, it may impact the performances.";
		Converter conv;
		conv.v = a;
		return set (conv.t[( s      & 0x0000000F)], conv.t[( s      & 0x000000F0)],
					conv.t[((s>> 8) & 0x0000000F)], conv.t[((s>> 8) & 0x000000F0)],
					conv.t[((s>>16) & 0x0000000F)], conv.t[((s>>16) & 0x000000F0)],
					conv.t[((s>>24) & 0x0000000F)], conv.t[((s>>24) & 0x000000F0)]);
	}

	/*
	* Unpack and interleave 16-bit integers from the low half of a and b, and store the results in dst.
	* Args   :	[a0, ..., a7] int16_t
				[b0, ..., b7] int16_t
	* Return :	[a0, b0, ..., a3, b3] int16_t
	*/
	static INLINE CONST vect_t unpacklo(const vect_t a, const vect_t b) { return _mm_unpacklo_epi16(a, b); }

	/*
	* Unpack and interleave 16-bit integers from the high half of a and b, and store the results in dst.
	* Args   :	[a0, ..., a7] int16_t
				[b0, ..., b7] int16_t
	* Return :	[a4, b4, ..., a7, b7] int16_t
	*/
	static INLINE CONST vect_t unpackhi(const vect_t a, const vect_t b) { return _mm_unpackhi_epi16(a, b); }

	/*
	* Blend packed 16-bit integers from a and b using control mask imm8, and store the results in dst.
	* Args   :	[a0, ..., a7] int16_t
				[b0, ..., b7] int16_t
	* Return :	[s[0]?a0:b0,   , s[7]?a7:b7] int16_t
	*/
	template<uint8_t s>
	static INLINE CONST vect_t blend(const vect_t a, const vect_t b) {
		return _mm_blend_epi16(a, b, s);
	}

	/*
	* Add packed 16-bits integer in a and b, and store the results in vect_t.
	* Args   :	[a0, ..., a7] int16_t
				[b0, ..., b7] int16_t
	* Return :	[a0+b0, ..., a7+b7]   int16_t
	*/
	static INLINE CONST vect_t add(const vect_t a, const vect_t b) { return _mm_add_epi16(a, b); }

	static INLINE vect_t addin(vect_t &a, const vect_t b) { return a = add(a, b); }

	/*
	* Subtract packed 16-bit integers in b from packed 16-bit integers in a, and store the results in vect_t.
	* Args   :	[a0, ..., a7] int16_t
				[b0, ..., b7] int16_t
	* Return :	[a0-b0, ..., a7-b7]  int16_t
	*/
	static INLINE CONST vect_t sub(const vect_t a, const vect_t b) { return _mm_sub_epi16(a, b); }

	static INLINE vect_t subin(vect_t &a, const vect_t b) { return a = sub(a, b); }

	/*
	* Multiply the packed 16-bit integers in a and b, producing intermediate 32-bit integers, and store the low 16 bits
	of the intermediate integers in vect_t.
	* Args   :	[a0, ..., a7] int16_t
				[b0, ..., b7] int16_t
	* Return :	[a0*b0 smod 2^16, ..., a7*b7 smod 2^16]	int16_t
	*	   where (a smod p) is the signed representant of a modulo p, that is -p/2 <= (a smod p) < p/2
	*/
	static INLINE CONST vect_t mullo(const vect_t a, const vect_t b) { return _mm_mullo_epi16(a, b); }

	static INLINE CONST vect_t mul(const vect_t a, const vect_t b) { return mullo(a, b); }

	/*
	* Multiply the packed 16-bit integers in a and b, producing intermediate 32-bit integers, and store the high 16
	bits of the intermediate integers in vect_t.
	* Args   :	[a0, ..., a7] int16_t
				[b0, ..., b7] int16_t
	* Return :	[Floor(a0*b0/2^16), ..., Floor(a7*b7/2^16)] int16_t
	*/
	static INLINE CONST vect_t mulhi(const vect_t a, const vect_t b) { return _mm_mulhi_epi16(a, b); }

	/*
	* Multiply the low 8-bit integers from each packed 16-bit element in a and b, and store the signed 16-bit results
	in vect_t.
	* Args   :	[a0, ..., a7] int16_t
				[b0, ..., b7] int16_t
	* Return :	[(a0 smod 2^8)*(b0 smod 2^8), ..., (a7 smod 2^8)*(b7 smod 2^8)]	int16_t
	*	   where (a smod p) is the signed representant of a modulo p, that is -p/2 <= (a smod p) < p/2
	*/
	static INLINE CONST vect_t mulx(const vect_t a, const vect_t b) {
		//#pragma warning "The simd mulx function is emulated, it may impact the performances."
		vect_t a1, b1, mask1, mask2;
		mask1 = set1(0x00FF);
		mask2 = set1(0x0080);
		a1 = add(a,mask2);
		a1 = vand(a1,mask1);
		a1 = sub(a1,mask2);
		b1 = add(b,mask2);
		b1 = vand(b1,mask1);
		b1 = sub(b1,mask2);
		return mul(a1,b1);
	}

	/*
	* Multiply the packed 16-bit integers in a and b, producing intermediate 32-bit integers,
	* keep the low 16 bits of the intermediate and add the low 16-bits of c.
	* Args   :	[a0, ..., a7]		int16_t
				[b0, ..., b7]		int16_t
				[c0, ..., c7]		int16_t
	* Return :	[(a0*b0+c0) smod 2^16, ..., (a7*b7+c7) smod 2^16]	int16_t
	*/
	static INLINE CONST vect_t fmadd(const vect_t c, const vect_t a, const vect_t b) { return add(c, mul(a, b)); }

	static INLINE vect_t fmaddin(vect_t &c, const vect_t a, const vect_t b) { return c = fmadd(c, a, b); }

	/*
	* Multiply the low 8-bit integers from each packed 16-bit element in a and b,
	* keep the signed 16-bit results and add the low 16-bits of c.
	* Args   :	[a0, ..., a7]		int16_t
				[b0, ..., b7]		int16_t
				[c0, ..., c7]		int16_t
	* Return :	[((a0 smod 2^8)*(b0 smod 2^8)+c0) smod 2^16, ...,
	*		 ((a7 smod 2^8)*(b7 smod 2^8)+c7) smod 2^16]	int16_t
	*/
	static INLINE CONST vect_t fmaddx(const vect_t c, const vect_t a, const vect_t b) { return add(c, mulx(a, b)); }

	static INLINE vect_t fmaddxin(vect_t &c, const vect_t a, const vect_t b) { return c = fmaddx(c, a, b); }

	/*
	* Multiply the packed 16-bit integers in a and b, producing intermediate 32-bit integers,
	* and substract the low 16 bits of the intermediate from elements of c.
	* Args   :	[a0, ..., a7]		int16_t
				[b0, ..., b7]		int16_t
				[c0, ..., c7]		int16_t
	* Return :	[(-a0*b0+c0) smod 2^16, ..., (-a7*b7+c7) smod 2^16]	int16_t
	*/
	static INLINE CONST vect_t fnmadd(const vect_t c, const vect_t a, const vect_t b) { return sub(c, mul(a, b)); }

	static INLINE vect_t fnmaddin(vect_t &c, const vect_t a, const vect_t b) { return c = fnmadd(c, a, b); }

	/*
	* Multiply the low 8-bit integers from each packed 16-bit element in a and b,
	* keep the signed 16-bit results and substract them from elements of c.
	* Args   :	[a0, ..., a7]		int16_t
				[b0, ..., b7]		int16_t
				[c0, ..., c7]		int16_t
	* Return :	[(-(a0 smod 2^8)*(b0 smod 2^8)+c0) smod 2^16, ...,
	*		 (-(a7 smod 2^8)*(b7 smod 2^8)+c7) smod 2^16]		int16_t
	*/
	static INLINE CONST vect_t fnmaddx(const vect_t c, const vect_t a, const vect_t b) { return sub(c, mulx(a, b)); }

	static INLINE vect_t fnmaddxin(vect_t &c, const vect_t a, const vect_t b) { return c = fnmaddx(c, a, b); }

	/*
	* Multiply packed 16-bit integers in a and b, producing intermediate 32-bit integers,
	* and substract elements of c to the low 16-bits of the intermediate.
	* Args   :	[a0, ..., a7]		int16_t
				[b0, ..., b7]		int16_t
				[c0, ..., c7]		int16_t
	* Return :	[(a0*b0-c0) smod 2^16, ..., (a7*b7-c7) smod 2^16]	int16_t
	*/
	static INLINE CONST vect_t fmsub(const vect_t c, const vect_t a, const vect_t b) { return sub(mul(a, b), c); }

	static INLINE vect_t fmsubin(vect_t &c, const vect_t a, const vect_t b) { return c = fmsub(c, a, b); }

	/*
	* Multiply the low 8-bit integers from each packed 16-bit element in a and b,
	* keep the signed 16-bit results and substract elements of c from them.
	* Args   :	[a0, ..., a7]		int16_t
				[b0, ..., b7]		int16_t
				[c0, ..., c7]		int16_t
	* Return :	[((a0 smod 2^8)*(b0 smod 2^8)-c0) smod 2^16, ...,
	*		 ((a7 smod 2^8)*(b7 smod 2^8)-c7) smod 2^16]		int16_t
	*/
	static INLINE CONST vect_t fmsubx(const vect_t c, const vect_t a, const vect_t b) { return sub(mulx(a, b), c); }

	static INLINE vect_t fmsubxin(vect_t &c, const vect_t a, const vect_t b) { return c = fmsubx(c, a, b); }

	/*
	* Compare packed 16-bits in a and b for equality, and store the results in vect_t.
	* Args   :	[a0, ..., a7]		int16_t
				[b0, ..., b7]		int16_t
	* Return :	[(a0==b0) ? 0xFFFF : 0, ..., (a7==b7) ? 0xFFFF : 0]			int16_t
	*/
	static INLINE CONST vect_t eq(const vect_t a, const vect_t b) { return _mm_cmpeq_epi16(a, b); }

	/*
	* Compare packed 16-bits in a and b for greater-than, and store the results in vect_t.
	* Args   :	[a0, ..., a7]		int16_t
				[b0, ..., b7]		int16_t
	* Return :	[(a0>b0) ? 0xFFFF : 0, ..., (a7>b7) ? 0xFFFF : 0]			int16_t
	*/
	static INLINE CONST vect_t greater(const vect_t a, const vect_t b) { return _mm_cmpgt_epi16(a, b); }

	/*
	* Compare packed 16-bits in a and b for lesser-than, and store the results in vect_t.
	* Args   :	[a0, ..., a7]		int16_t
				[b0, ..., b7]		int16_t
	* Return :	[(a0<b0) ? 0xFFFF : 0, ..., (a7<b7) ? 0xFFFF : 0]			int16_t
	*/
	static INLINE CONST vect_t lesser(const vect_t a, const vect_t b) { return _mm_cmplt_epi16(a, b); }

	/*
	* Compare packed 16-bits in a and b for greater or equal than, and store the results in vect_t.
	* Args   :	[a0, ..., a7]		int16_t
				[b0, ..., b7]		int16_t
	* Return :	[(a0>=b0) ? 0xFFFF : 0, ..., (a7>=b7) ? 0xFFFF : 0]			int16_t
	*/
	static INLINE CONST vect_t greater_eq(const vect_t a, const vect_t b) { return vor(greater(a, b), eq(a, b)); }

	/*
	* Compare packed 16-bits in a and b for lesser or equal than, and store the results in vect_t.
	* Args   :	[a0, ..., a7]		int16_t
				[b0, ..., b7]		int16_t
	* Return :	[(a0<=b0) ? 0xFFFF : 0, ..., (a7<=b7) ? 0xFFFF : 0]			int16_t
	*/
	static INLINE CONST vect_t lesser_eq(const vect_t a, const vect_t b) { return vor(lesser(a, b), eq(a, b)); }

	/*
	* Horizontally add 16-bits elements of a.
	* Args   :	[a0, a1, a2, a3, a4, a5, a6, a7]
	* Return :	a0+a1+a2+a3
	*/
	static INLINE CONST scalar_t hadd_to_scal(const vect_t a) {
		Converter conv;
		conv.v = a;
		return scalar_t(conv.t[0] + conv.t[1] + conv.t[2] + conv.t[3] + conv.t[4] + conv.t[5] + conv.t[6] + conv.t[7]);
	}

	static INLINE CONST vect_t round(const vect_t a) { return a; }

	static INLINE CONST vect_t signbits(const vect_t x) {
		vect_t signBits = sub(zero(), srl(x, 4*sizeof(scalar_t)-1));
		return signBits;
	}

	static INLINE vect_t mod(vect_t &C, const vect_t &P, const __m64 &INVP, const vect_t &NEGP, const vect_t &MIN,
							 const vect_t &MAX, vect_t &Q, vect_t &T) {
#ifdef __INTEL_COMPILER
		C = _mm_rem_epi16(C, P);
#else
		FFLASFFPACK_abort("not implemented");
#endif
		NORML_MOD(C, P, NEGP, MIN, MAX, Q, T);
		return C;
	}
};

/*
 * Simd128 specialized for uint16_t
 */
template <> struct Simd128_impl<true, true, false, 2> : public Simd128_impl<true, true, true, 2> {

	/*
	* define the scalar type corresponding to the specialization
	*/
	using scalar_t = uint16_t;

	/*
	 * Converter from vect_t to a tab.
	 * exple:
	 *	Converter conv;
	 *	conv.v = a;
	 *	scalart_t x = conv.t[1]
	 */
	union Converter {
		vect_t v;
		scalar_t t[vect_size];
	};

	/*
	*  Broadcast 16-bit unsigned integer a to all elements of dst. This intrinsic may generate the vpbroadcastw.
	*  Return [x,x,x,x,x,x,x,x] uint16_t
	*/
	static INLINE CONST vect_t set1(const scalar_t x) { return _mm_set1_epi16(x); }

	/*
	*  Broadcast 16-bit unsigned integer a to all elements of dst. This intrinsic may generate the vpbroadcastw.
	*  Return [x0, ..., x7] uint16_t
	*/
	static INLINE CONST vect_t set(const scalar_t x0, const scalar_t x1, const scalar_t x2, const scalar_t x3,
								   const scalar_t x4, const scalar_t x5, const scalar_t x6, const scalar_t x7) {
		return _mm_set_epi16(x7, x6, x5, x4, x3, x2, x1, x0);
	}

	/*
	*  Gather 16-bit unsigned integer elements with indexes idx[0], ..., idx[7] from the address p in vect_t.
	*  Return [p[idx[0]],..., p[idx[7]]] uint16_t
	*/
	template <class T> static INLINE PURE vect_t gather(const scalar_t *const p, const T *const idx) {
		return set(p[idx[0]], p[idx[1]], p[idx[2]], p[idx[3]], p[idx[4]], p[idx[5]], p[idx[6]], p[idx[7]]);
	}

	/*
	* Load 128-bits of unsigned integer data from memory into dst.
	* p must be aligned on a 32-byte boundary or a general-protection exception will be generated.
	* Return [p[idx[0]],..., p[idx[7]]] uint16_t
	*/
	static INLINE PURE vect_t load(const scalar_t *const p) {
		return _mm_load_si128(reinterpret_cast<const vect_t *>(p));
	}

	/*
	* Load 128-bits of unsigned integer data from memory into dst.
	* p does not need to be aligned on any particular boundary.
	* Return [p[idx[0]],..., p[idx[7]]] uint16_t
	*/
	static INLINE PURE vect_t loadu(const scalar_t *const p) {
		return _mm_loadu_si128(reinterpret_cast<const vect_t *>(p));
	}

	/*
	* Store 128-bits of unsigned integer data from a into memory.
	* p must be aligned on a 32-byte boundary or a general-protection exception will be generated.
	*/
	static INLINE void store(scalar_t *p, vect_t v) {
		_mm_store_si128(reinterpret_cast<vect_t *>(p), v);
	}

	/*
	* Store 128-bits of unsigned integer data from a into memory.
	* p does not need to be aligned on any particular boundary.
	*/
	static INLINE void storeu(scalar_t *p, vect_t v) {
		_mm_storeu_si128(reinterpret_cast<vect_t *>(p), v);
	}

	/*
	* Store 128-bits of unsigned integer data from a into memory using a non-temporal memory hint.
	* p must be aligned on a 16-byte boundary or a general-protection exception may be generated.
	*/
	static INLINE void stream(scalar_t *p, const vect_t v) {
		_mm_stream_si128(reinterpret_cast<vect_t *>(p), v);
	}

	/*
	* Shift packed 16-bit unsigned integers in a right by s while shifting in sign bits, and store the results in vect_t.
	 * Args   :	[a0, ..., a7]			uint16_t
	 * Return :	[Floor(a0/2^s), ..., Floor(a7/2^s)]	int16_t
	*/
	static INLINE CONST vect_t sra(const vect_t a, const int s) { return _mm_srli_epi16(a, s); }

	static INLINE CONST vect_t greater(vect_t a, vect_t b) {
		vect_t x;
		x = set1((static_cast<scalar_t>(1) << (sizeof(scalar_t) * 8 - 1)));
		a = sub(a,x);
		b = sub(b,x);
		return _mm_cmpgt_epi16(a, b);
	}

	static INLINE CONST vect_t lesser(vect_t a, vect_t b) {
		vect_t x;
		x = set1((static_cast<scalar_t>(1) << (sizeof(scalar_t) * 8 - 1)));
		a = sub(a,x);
		b = sub(b,x);
		return _mm_cmplt_epi16(a, b);
	}

	static INLINE CONST vect_t greater_eq(const vect_t a, const vect_t b) { return vor(greater(a, b), eq(a, b)); }

	static INLINE CONST vect_t lesser_eq(const vect_t a, const vect_t b) { return vor(lesser(a, b), eq(a, b)); }

	/*
	* Multiply the packed unsigned 16-bit integers in a and b, producing intermediate 32-bit integers,
	* and store the high 16 bits of the intermediate integers in vect_t.
	* Args   :	[a0, ..., a7] uint16_t
	*			[b0, ..., b7] uint16_t
	* Return :	[Floor(a0*b0/2^16), ..., Floor(a7*b7/2^16)] uint16_t
	*/
	static INLINE CONST vect_t mulhi(const vect_t a, const vect_t b) { return _mm_mulhi_epu16(a, b); }

	/*
	* Multiply the low unsigned 8-bit integers from each packed 16-bit element in a and b,
	* and store the signed 16-bit results in vect_t.
	* Args   :	[a0, ..., a7] uint16_t
	*			[b0, ..., b7] uint16_t
	* Return :	[(a0 mod 2^8)*(b0 mod 2^8), ..., (a7 mod 2^8)*(b7 mod 2^8)] uint16_t
	*/
	static INLINE CONST vect_t mulx(const vect_t a, const vect_t b) {
		//#pragma warning "The simd mulx function is emulated, it may impact the performances."
		vect_t a1, b1, mask1;
		mask1 = set1(0x00FF);
		a1 = vand(a,mask1);
		b1 = vand(b,mask1);
		return mul(a1,b1);
	}

	static INLINE CONST vect_t fmaddx(const vect_t c, const vect_t a, const vect_t b) { return add(c, mulx(a, b)); }

	static INLINE vect_t fmaddxin(vect_t &c, const vect_t a, const vect_t b) { return c = fmaddx(c, a, b); }

	static INLINE CONST vect_t fnmaddx(const vect_t c, const vect_t a, const vect_t b) { return sub(c, mulx(a, b)); }

	static INLINE vect_t fnmaddxin(vect_t &c, const vect_t a, const vect_t b) { return c = fnmaddx(c, a, b); }

	static INLINE CONST vect_t fmsubx(const vect_t c, const vect_t a, const vect_t b) { return sub(mulx(a, b), c); }

	static INLINE vect_t fmsubxin(vect_t &c, const vect_t a, const vect_t b) { return c = fmsubx(c, a, b); }

	/*
	* Horizontally add 16-bits elements of a.
	* Args   :	[a0, a1, a2, a3, a4, a5, a6, a7]
	* Return :	a0+a1+a2+a3
	*/
	static INLINE CONST scalar_t hadd_to_scal(const vect_t a) {
		Converter conv;
		conv.v = a;
		return scalar_t(conv.t[0] + conv.t[1] + conv.t[2] + conv.t[3] + conv.t[4] + conv.t[5] + conv.t[6] + conv.t[7]);
	}
}; //Simd128_impl<true,true,false,2>

#endif // __FFLASFFPACK_fflas_ffpack_utils_simd128_int16_INL
