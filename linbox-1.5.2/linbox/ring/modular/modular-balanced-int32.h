/* Copyright (C) 2009 LinBox
 * Written by C Pernet
 * updated to compilable condition by Brice Boyer (briceboyer) <boyer.brice@gmail.com>
 *
 *
 *
 * ========LICENCE========
 * This file is part of the library LinBox.
 *
  * LinBox is free software: you can redistribute it and/or modify
 * it under the terms of the  GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========
 */


/*! @file field/modular/modular-balanced-int32.h
 * @ingroup field
 * @brief Balanced representation of <code>Z/mZ</code> over \c int32_t .
 */

#ifndef __LINBOX_modular_balanced_int32_H
#define __LINBOX_modular_balanced_int32_H


#include "linbox/linbox-config.h"
#include "linbox/integer.h"
#include "linbox/vector/vector-domain.h"
#include "linbox/ring/modular.h"
#include "linbox/field/field-interface.h"
#include "linbox/field/field-traits.h"
#include "linbox/util/field-axpy.h"
#include "linbox/util/debug.h"
#include "linbox/field/field-traits.h"
#include "linbox/field/modular/modular-int32.h"

#include <givaro/modular-balanced-int32.h>

#ifndef LINBOX_MAX_INT /* 2147483647 */
#define LINBOX_MAX_INT INT32_MAX
#endif


// Namespace in which all LinBox code resides
namespace LinBox
{

	template <>
	class FieldAXPY<Givaro::ModularBalanced<int32_t> > {
	public:

		typedef int32_t Element;
		typedef int64_t Abnormal;
		typedef Givaro::ModularBalanced<int32_t> Field;

		FieldAXPY (const Field &F) :
			_field (&F),_y(0),_times(0)
		{
		}


		FieldAXPY (const FieldAXPY &faxpy) :
			_field (faxpy._field), _y (0),_times(0)
		{}

		FieldAXPY<Givaro::ModularBalanced<int32_t> > &operator = (const FieldAXPY &faxpy)
		{
			_field = faxpy._field;
			_y = faxpy._y;
			_times = faxpy._times;
			return *this;
		}

		inline int64_t& mulacc (const Element &a, const Element &x)
		{
			int64_t t = (int64_t) a * (int64_t)   x;
			if (_times < blocksize) {
				++_times;
				return _y += t;
			}

			else {
				_times = 1;
				normalize();
				return _y += t;
			}
		}

		inline int64_t& accumulate (const Element &t)
		{
			if (_times < blocksize) {
				++_times;
				return _y += t;
			}

			else {
				_times = 1;
				normalize();
				return _y += t;
			}
		}

		inline Element& get (Element &y)
		{

			normalize();

			y = Element(_y);

			if (y > field().half_mod)
				y -= field().characteristic();
			else if (y < field().mhalf_mod)
				y += field().characteristic();

			return y;
		}

		inline FieldAXPY &assign (const Element y)
		{
			_y = y;
			return *this;
		}

		inline void reset()
		{
			_y = 0;
		}

		inline const Field & field() const { return *_field; }

	private:

		const Field *_field;
		int64_t _y;
		int32_t _times;
		static const int32_t blocksize = 32;

		inline void normalize()
		{
			_y = (int32_t)_y -(int32_t)(int64_t)((double) _y * field().modulusinv) * (int32_t)field().characteristic();
		}

	};


	template <>
	class DotProductDomain<Givaro::ModularBalanced<int32_t> > : public  VectorDomainBase<Givaro::ModularBalanced<int32_t> > {

	private:
		int32_t blocksize;

	public:
		typedef int32_t Element;
		DotProductDomain(){}
		DotProductDomain (const Givaro::ModularBalanced<int32_t> &F) :
			VectorDomainBase<Givaro::ModularBalanced<int32_t> > (F) ,blocksize(32)
		{ }

		using VectorDomainBase<Givaro::ModularBalanced<int32_t> >::field;
	protected:
		template <class Vector1, class Vector2>
		inline Element &dotSpecializedDD (Element &res, const Vector1 &v1, const Vector2 &v2) const
		{

			typename Vector1::const_iterator pv1,pv1e;
			typename Vector2::const_iterator pv2;

			int64_t y = 0;
			int64_t t;
			// int32_t times = 0;

			pv1 = pv1e = v1.begin();
			pv2 = v2.begin();

			for(size_t i = 0; i < v1.size() / (size_t)blocksize ;++i) {
				pv1e = pv1e + blocksize;
				for(;pv1 != pv1e;++pv1,++pv2) {
					t = (((int64_t) *pv1 ) * ((int64_t) *pv2 ));
					y += t;
				}
				normalize(y);
			}

			for(;pv1 != v1.end(); ++pv1, ++pv2) {
				t = (((int64_t) *pv1 ) * ((int64_t) *pv2 ));
				y += t;
			}

			normalize(y);
			res = (Element) y;

			if (res > field().half_mod) res -= field().characteristic();
			else if(res < field().mhalf_mod) res += field().characteristic();

			return res;

		}

		template <class Vector1, class Vector2>
		inline Element &dotSpecializedDSP (Element &res, const Vector1 &v1, const Vector2 &v2) const
		{

			typename Vector1::first_type::const_iterator i_idx, i_idxe;
			typename Vector1::second_type::const_iterator i_elt;

			int64_t y = 0;
			int64_t t;

			i_idx = i_idxe = v1.first.begin();
			i_elt = v1.second.begin();

			for(size_t i = 0; i < v1.first.size() / (size_t)blocksize ; ++i) {
				i_idxe = i_idxe + blocksize;
				for(;i_idx!= i_idxe;++i_idx, ++i_elt) {
					t = ( (int64_t) *i_elt ) * ( (int64_t) v2[*i_idx] );
					y += t;
				}
				normalize(y);
			}


			for(;i_idx!= v1.first.end();++i_idx, ++i_elt) {
				t = ( (int64_t) *i_elt ) * ( (int64_t) v2[*i_idx] );
				y += t;
			}

			normalize(y);

			res = (Element) y;
			if (res > field().half_mod) res -= field().characteristic();
			else if(res < field().mhalf_mod) res += field().characteristic();

			return res;
		}

		inline void normalize(int64_t& _y) const
		{
			_y = (int32_t)_y -(int32_t)(int64_t)((double) _y * field().modulusinv) * (int32_t)field().characteristic();
		}

	};
}

#include "linbox/randiter/modular-balanced.h"

#endif //__LINBOX_modular_balanced_int32_H


// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
