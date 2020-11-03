/* linbox/algorithms/gauss-solve.inl
 * Copyright (C) LinBox 2008
 *
 * Written by Jean-Guillaume Dumas <Jean-Guillaume.Dumas@imag.fr>
 * Time-stamp: <13 Nov 17 17:00:53 Jean-Guillaume.Dumas@imag.fr>
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

#ifndef __LINBOX_gauss_solve_INL
#define __LINBOX_gauss_solve_INL

// #include "linbox/algorithms/gauss.h"
#include "linbox/algorithms/triangular-solve.h"
#include "linbox/blackbox/permutation.h"

namespace LinBox
{


	template <class _Field>
	template <class _Matrix, class Perm, class Vector1, class Vector2> inline Vector1&
	GaussDomain<_Field>::solve(Vector1& x, Vector1& w, unsigned long Rank, const Perm& Q, const _Matrix& L, const _Matrix& U, const Perm& P, const Vector2& b)  const
	{
            // Q L U P x = b
		Vector2 y(U.field(),U.rowdim()), v(U.field(),U.rowdim());

            // L U P x = y = Q^T b
		Q.applyTranspose(y, b);

            // U P x = v = L^{-1} y
		lowerTriangularUnitarySolve(v, L, y);

            // P x = w = U^{-1} v, only for the upper part of w
		upperTriangularSolve(w, U, v);

            // x = P^T w = P^T U^{-1} L^{-1} Q^T b
		return P.applyTranspose(x, w);
	}

	template <class _Field>
	template <class _Matrix, class Vector1, class Vector2> inline Vector1&
	GaussDomain<_Field>::solvein(Vector1& x, _Matrix& A, const Vector2& b)  const
	{

		typename Field::Element Det;
		unsigned long Rank;
		_Matrix L(field(), A.rowdim(), A.rowdim());
		Permutation<Field> Q(field(),(int)A.rowdim());
		Permutation<Field> P(field(),(int)A.coldim());

		this->QLUPin(Rank, Det, Q, L, A, P, A.rowdim(), A.coldim() );

            // Sets solution values to 0 for coldim()-Rank columns
            // Therefore, prune unnecessary elements
            // in those last columns of U
		for(typename _Matrix::RowIterator row=A.rowBegin();
		    row != A.rowEnd(); ++row) {
			if (row->size()) {
				size_t ns=0;
				for(typename _Matrix::Row::iterator it = row->begin();
				    it != row->end(); ++it, ++ns) {
					if (it->first >= Rank) {
						row->resize(ns);
						break;
					}
				}
			}
		}

		Vector1 w(A.field(),A.coldim());

		for(typename Vector1::iterator it=w.begin()+(ptrdiff_t)Rank;it!=w.end();++it)
			field().assign(*it,field().zero);

		return this->solve(x, w, Rank, Q, L, A, P, b);
	}

	template <class _Field>
	template <class _Matrix, class Vector1, class Vector2, class Random> inline Vector1&
	GaussDomain<_Field>::solvein(Vector1& x, _Matrix& A, const Vector2& b, Random& generator)  const
	{
		typename Field::Element Det;
		unsigned long Rank;
		_Matrix L(field(), A.rowdim(), A.rowdim());
		Permutation<Field> Q(field(),(int)A.rowdim());
		Permutation<Field> P(field(),(int)A.coldim());

		this->QLUPin(Rank, Det, Q, L, A, P, A.rowdim(), A.coldim() );

        Vector1 w(A.field(),A.coldim());
		for(typename Vector1::iterator it=w.begin()+(ptrdiff_t)Rank;it!=w.end();++it)
			generator.random( *it );

		return this->solve(x, w, Rank, Q, L, A, P, b);
	}


} // namespace LinBox

#endif // __LINBOX_gauss_solve_INL


// Local Variables:
// mode: C++
// tab-width: 4
// indent-tabs-mode: nil
// c-basic-offset: 4
// End:
// vim:sts=4:sw=4:ts=4:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s

