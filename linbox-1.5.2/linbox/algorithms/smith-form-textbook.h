/* 
 * Copyright (C) 2014 Gavin Harrison,
 *
 * Written by Gavin Harrison <gmh33@drexel.edu>,
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

#include "linbox/matrix/dense-matrix.h"

#ifndef __LINBOX_smith_form_textbook_domain_H
#define __LINBOX_smith_form_textbook_domain_H

namespace LinBox
{
	template<class MatrixDomain>
	class SmithFormTextbookDomain {
	public:
		typedef typename MatrixDomain::Field Field;
		typedef typename Field::Element Element;
		typedef BlasMatrix<Field> Rep;

	private:
		MatrixDomain _MD;

	public:
		const Field &field() const { return _MD.field(); }
		SmithFormTextbookDomain(const MatrixDomain &MD) : _MD(MD) {}
		SmithFormTextbookDomain(const SmithFormTextbookDomain &D) : _MD(D._MD) {}

	private:
		void swapRows(Rep &A, int n, int a, int b) const
		{
			for (size_t i = n; i < A.coldim(); i++)
			{
				Element tmp1, tmp2;

				A.getEntry(tmp1, a, i);
				A.getEntry(tmp2, b, i);

				A.setEntry(a, i, tmp2);
				A.setEntry(b, i, tmp1);
			}
		}

		void swapCols(Rep &A, int n, int a, int b) const
		{
			for (size_t i = n; i < A.rowdim(); i++)
			{
				Element tmp1, tmp2;

				A.getEntry(tmp1, i, a);
				A.getEntry(tmp2, i, b);

				A.setEntry(i, a, tmp2);
				A.setEntry(i, b, tmp1);
			}
		}

		// Ensures that if a=b then s=u=v=1 and t=0 to avoid an infinite loop
		void dxgcd(Element &s, Element &t, Element &u, Element &v, const Element &a, const Element &b) const
		{
			if (field().areEqual(a,b))
			{
				field().assign(s, field().one);
				field().assign(t, field().zero);
				field().assign(u, field().one);
				field().assign(v, field().one);
				return;
			}

			Element g;
			field().dxgcd(g,s,t,u,v,a,b);
		}

		bool findPivot(Rep &A, size_t n) const
		{
			for (size_t i = n; i < A.rowdim(); i++)
			{
				for (size_t j = n; j < A.coldim(); j++)
				{
					Element tmp;
					A.getEntry(tmp, i, j);

					if (!field().isZero(tmp))
					{
						if (i != n)
							swapRows(A, n, n, i);

						if (j != n)
							swapCols(A, n, n, j);

						return true;;
					}
				}
			}

			return false;
		}

		bool eliminateCol(Rep &A, int n) const
		{
			bool modified = false;

			for (size_t i = n+1; i < A.rowdim(); i++)
			{
				Element nn, in;

				A.getEntry(nn, n, n);
				A.getEntry(in, i, n);

				if (!field().isZero(in))
				{
					modified = true;

					Element s, t, u, v;
					dxgcd(s, t, u, v, nn, in);

					for (size_t j = n; j < A.coldim(); j++)
					{
						Element nj, ij;

						A.getEntry(nj, n, j);
						A.getEntry(ij, i, j);

						Element tmp1, tmp2;

						field().mul(tmp1, nj, s);
						field().mul(tmp2, ij, t);
						field().addin(tmp1, tmp2);
						A.setEntry(n, j, tmp1);

						field().mul(tmp1, nj, v);
						field().mul(tmp2, ij, u);
						field().subin(tmp1, tmp2);
						A.setEntry(i, j, tmp1);
					}
				}
			}

			return modified;
		}

		bool eliminateRow(Rep &A, int n) const
		{
			int modified = false;

			for (size_t i = n+1; i < A.coldim(); i++)
			{
				Element nn, ni;

				A.getEntry(nn, n, n);
				A.getEntry(ni, n, i);

				if (!field().isZero(ni))
				{
					modified = true;

					Element s, t, u, v;
					dxgcd(s, t, u, v, nn, ni);

					for (size_t j = n; j < A.rowdim(); j++)
					{
						Element jn, ji;

						A.getEntry(jn, j, n);
						A.getEntry(ji, j, i);

						Element tmp1, tmp2;

						field().mul(tmp1, jn, s);
						field().mul(tmp2, ji, t);
						field().addin(tmp1, tmp2);
						A.setEntry(j, n, tmp1);

						field().mul(tmp1, jn, v);
						field().mul(tmp2, ji, u);
						field().subin(tmp1, tmp2);
						A.setEntry(j, i, tmp1);
					}
				}
			}

			return modified;
		}

		bool fixDiagonal(Rep &A) const
		{
			bool fixed = false;

			size_t dim = A.rowdim() < A.coldim() ? A.rowdim() : A.coldim();

			for (size_t i = 0; i < dim-1; i++)
			{
				Element tmp1, tmp2;

				A.getEntry(tmp1, i, i);
				A.getEntry(tmp2, i+1, i+1);

				if (!field().isZero(tmp2))
				{
					Element g;
					field().gcd(g, tmp1, tmp2);

					if (!field().areAssociates(g, tmp1))
					{
						A.setEntry(i+1, i, tmp2);
						fixed = true;
					}
				}
				else
				{
					return fixed;
				}
			}

			return fixed;
		}

	public:
		template<class Vector>
		Vector &solve(Vector &S, const Rep &A) const
		{
			size_t dim = A.rowdim() < A.coldim() ? A.rowdim() : A.coldim();

			linbox_check(S.size() >= dim);

			Rep B(A);

			do
			{
				for (size_t n = 0; n < dim && findPivot(B, n); n++)
				{
					eliminateCol(B, n);
					while(eliminateRow(B, n) && eliminateCol(B, n));
				}
			} while(fixDiagonal(B));

			for (size_t i = 0; i < dim; i++)
			{
				Element tmp;
				B.getEntry(tmp, i, i);
				S.setEntry(i, tmp);
			}

			return S;
		}
	};
}

#endif // __LINBOX_smith_form_direct_domain_H
