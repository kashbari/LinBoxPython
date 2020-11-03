/* linbox/solutions/getentry.inl
 * Copyright(C) LinBox
 *  Evolved from an earlier one by Bradford Hovinen <hovinen@cis.udel.edu>
 *  -bds
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========
 *.
 */

#ifndef __LINBOX_getentry_INL
#define __LINBOX_getentry_INL

#include <vector>

#include "linbox/util/debug.h"
#include "linbox/vector/vector-domain.h"
#include "linbox/matrix/dense-matrix.h"
#include "linbox/matrix/sparse-matrix.h"
#include "linbox/matrix/sparse-matrix.h"
#include "linbox/blackbox/scalar-matrix.h"
#include "linbox/blackbox/compose.h"
#include "linbox/blackbox/diagonal.h"
#include "linbox/solutions/methods.h"
#include "linbox/solutions/solution-tags.h"
#include "linbox/vector/blas-vector.h"

namespace LinBox
{

	template <class BB>
	typename BB::Field::Element& getEntry(typename BB::Field::Element& x, const BB& A, const size_t i, const size_t j)
	{
		typename GetEntryCategory<BB>::Tag t;
		return getEntry(x, A, i, j, t);
	}

	// General case category of BBs requires use of apply.
	template<class BB> struct GetEntryCategory{ typedef SolutionTags::Generic Tag; };
	// Look for local specializations of this in blackbox/*

	// To ignore methods
	template <class BB, class Method>
	typename BB::Field::Element& getEntry(typename BB::Field::Element& x, const BB& A, const size_t i, const size_t j, Method & m)
	{
		return getEntry(x, A, i, j);
	}

	// Generic BBs require use of apply.
	template <class BB>
	typename BB::Field::Element& getEntry(typename BB::Field::Element& x, const BB& A, const size_t i, const size_t j, SolutionTags::Generic t)
	{
		typedef typename BB::Field Field;
		typedef BlasVector<Field> Vector ;

		const Field& F = A.field();
		Vector v(F,A.coldim(), F.zero), w(F,A.rowdim(), F.zero);
		F.assign(v[j],F.one); // use standard basis ?
		A.apply (w, v);
		return F.assign(x, w[i]);
	}

	// some BBs have their own.
	template <class BB>
	typename BB::Field::Element& getEntry(typename BB::Field::Element& x, const BB& A, const size_t i, const size_t j, SolutionTags::Local t )
	{
		return A.getEntry(x, i, j);
	}

	// Compose< Diagonal, BB > specialization
	template <class Field, class Trait, class BB>
	typename Field::Element& getEntry(typename Field::Element& x, const Compose<Diagonal<Field, Trait>, BB>& A, const size_t i, const size_t j)
	{
		typename Field::Element y;
		getEntry(y, *(A.getLeftPtr()), i, i);
		getEntry(x, *(A.getRightPtr()), i, j);
		return A.field().mulin(x, y);
	}

	// Compose< BB, Diagonal > specialization
	template <class BB, class Field, class Trait>
	typename Field::Element& getEntry(typename Field::Element& x, const Compose<BB, Diagonal<Field, Trait> >& A, const size_t i, const size_t j)
	{
		typename Field::Element y;
		getEntry(y, *(A.getLeftPtr()), i, j);
		getEntry(x, *(A.getRightPtr()), j, j);
		return A.field().mulin(x, y);
	}

	// Compose< Diagonal, Diagonal > specialization
	template <class Field, class T1, class T2>
	typename Field::Element& getEntry(typename Field::Element& x, const Compose<Diagonal<Field,T1>, Diagonal<Field, T2> >& A, const size_t i, const size_t j)
	{
		if (i != j) return A.field().assign(x, A.field().zero);
        typename Field::Element y;
        getEntry(y, *(A.getLeftPtr()), i, i);
        getEntry(x, *(A.getRightPtr()), j, j);
        return A.field().mulin(x, y);
	}

}

#endif // __LINBOX_getentry_INL

// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
