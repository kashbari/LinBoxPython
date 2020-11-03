/* linbox/matrix/blas-matrix-domain.inl
 * Copyright (C) 2004 Pascal Giorgi, Clément Pernet
 *
 * Written by :
 *               Pascal Giorgi  pascal.giorgi@ens-lyon.fr
 *               Clément Pernet clement.pernet@imag.fr
 *               Brice Boyer (briceboyer) <boyer.brice@gmail.com>
 *
 * originally linbox/algorithms/blas-domain.inl
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



#ifndef __LINBOX_matrix_matrixdomain_blas_matrix_domain_INL
#define __LINBOX_matrix_matrixdomain_blas_matrix_domain_INL

namespace LinBox { namespace Protected {

	/*
	 * **********************************************
	 * *** Specialization for BlasSubmatrix<Matrix> ***
	 * the BlasMatrixDomain have template Matrix parameters
	 * and the specialized one are only for views.
	 * **********************************************
	 */
	// Inversion
	// dpritcha: now returns nullity. (2004-07-19)
	// previously returned Ainv but this is passed back anyway.
	template <class Matrix>
	class BlasMatrixDomainInv<typename Matrix::Field, BlasSubmatrix<Matrix> > {
	public:

		int operator() (const typename Matrix::Field &F,
				BlasSubmatrix<Matrix> &Ainv,
				BlasSubmatrix<Matrix> &A) const
		{

			linbox_check( A.rowdim() == A.coldim());
			linbox_check( A.rowdim() == Ainv.rowdim());
			linbox_check( A.coldim() == Ainv.coldim());
			int nullity;
			FFPACK::Invert2 (F, A.rowdim(), A.getPointer(), A.getStride(),
					Ainv.getPointer(), Ainv.getStride(),nullity);
			return nullity;
		}

	};

	// determinant
	template <class Matrix>
	class BlasMatrixDomainDet<typename Matrix::Field, BlasSubmatrix<Matrix> > {
	public:

		inline typename Matrix::Element operator() (const typename Matrix::Field                             &F,
							    BlasSubmatrix<Matrix>     &A) const
		{

			return FFPACK::Det(F, A.rowdim(), A.coldim(), A.getPointer(), A.getStride());
		}
	};


	// rank
	template <class Matrix>
	class 	BlasMatrixDomainRank<typename Matrix::Field, BlasSubmatrix<Matrix> > {
	public:
		inline unsigned int operator() (const typename Matrix::Field                                &F,
						BlasSubmatrix<Matrix>  &A) const
		{
			return (unsigned int) FFPACK::Rank(F,
							   A.rowdim(), A.coldim(), A.getPointer(), A.getStride());
		}
	};

} // Protected
} // LinBox

namespace LinBox
{ /* Det */

	// Det
	template< class Field, class Matrix>
	typename Field::Element
	BlasMatrixDomainDet<Field, Matrix>::operator() (const Field &F, const Matrix& A) const
	{
		typedef typename Matrix::constSubMatrixType constSubMatrixType ;
		typedef typename Matrix::matrixType       matrixType ;
		matrixType A_c(A); // do copy
		constSubMatrixType A_v(A_c);
		return Protected::BlasMatrixDomainDet<Field, constSubMatrixType>()(F, A_v);
	}

	template< class Field, class Matrix>
	typename Field::Element
	BlasMatrixDomainDet<Field, Matrix>::operator() (const Field &F, Matrix& A) const
	{
		typedef typename Matrix::subMatrixType subMatrixType ;
		subMatrixType A_v(A);
		return Protected::BlasMatrixDomainDet<Field, subMatrixType>()(F, A_v);
	}

	template< class Field, class _Rep>
	// template<class _Rep>
	class BlasMatrixDomainDet<Field, TriangularBlasMatrix<Field,_Rep> > {
		typename Field::Element operator() (const Field &F, const TriangularBlasMatrix<Field,_Rep> & A) const
		{
			typename Field::Element d ;
			F.init(d);
			// is A unit ? return 1
			// while not 0, mulin
			return d;
		}
	};

} // LinBox

namespace LinBox
{ /* Rank */

	// what about transpose ?

	// Rank
	template<class Field, class Matrix>
	inline unsigned int
	BlasMatrixDomainRank<Field, Matrix>::operator() (const Field   &F,
							 const  Matrix  &A) const
	{
		typedef typename Matrix::constSubMatrixType constSubMatrixType ;
		typedef typename Matrix::matrixType       matrixType ;
		matrixType A_c(A); // do copy
		constSubMatrixType A_v(A_c);

		return Protected::BlasMatrixDomainRank<Field, constSubMatrixType>()(F, A_v);
	}

	template<class Field, class Matrix>
	inline unsigned int
	BlasMatrixDomainRank<Field, Matrix>::operator() (const Field   &F,
							 Matrix        &A) const
	{
		typedef typename Matrix::subMatrixType subMatrixType ;
		subMatrixType A_v(A);

		return Protected::BlasMatrixDomainRank<Field, subMatrixType>()(F, A_v);
	}

} // LinBox

namespace LinBox
{ /* Inverse */
	// Inverse
	template<class Field, class Matrix1, class Matrix2>
	int BlasMatrixDomainInv<Field, Matrix1, Matrix2>::operator() (const Field &F, Matrix1 &Ainv, const Matrix2 &A) const
	{
		typedef typename Matrix1::subMatrixType subMatrixType ;
		// typedef typename Matrix1::constSubMatrixType constSubMatrixType ;
		typedef typename Matrix1::matrixType       matrixType ;
		subMatrixType Ai_v(Ainv);
		//! @todo check equal submatrix types
		matrixType A_c(A); // do copy
		subMatrixType A_v(A_c);
		return Protected::BlasMatrixDomainInv<Field, subMatrixType>()(F, Ai_v, A_v);
	}

	template<class Field, class Matrix1, class Matrix2>
	int BlasMatrixDomainInv<Field, Matrix1, Matrix2>::operator() (const Field &F, Matrix1 &Ainv, Matrix2 &A) const
	{
		typedef typename Matrix1::subMatrixType subMatrixType ;
		subMatrixType Ai_v(Ainv);
		subMatrixType A_v(A);
		return Protected::BlasMatrixDomainInv<Field, subMatrixType>()(F, Ai_v, A_v);
	}
} // LinBox

namespace LinBox
{ /* Add Sub */

	// Add
	template<class Field, class Matrix1, class Matrix2, class Matrix3>
	Matrix1&
	BlasMatrixDomainAdd<Field, Matrix1, Matrix2, Matrix3 >::operator()(const Field& F,
									   Matrix1& C,
									   const Matrix2& A,
									   const Matrix3& B) const
	{
		linbox_check( A.rowdim() == B.rowdim());
		linbox_check( C.rowdim() == A.rowdim());
		linbox_check( A.coldim() == B.coldim());
		linbox_check( C.coldim() == A.coldim());
		typedef typename Matrix1::subMatrixType subMatrixType ;
		typedef typename Matrix1::constSubMatrixType constSubMatrixType ;
		constSubMatrixType A_v(A);
		constSubMatrixType B_v(B);
		subMatrixType C_v(C);

		FFLAS::fadd (F, C_v.rowdim(), C_v.coldim(),
			     A_v.getPointer(), A_v.getStride(),
			     B_v.getPointer(), B_v.getStride(),
			     C_v.getPointer(), C_v.getStride());
		return C;
	}

	// AddIn
	template<class Field, class Matrix1, class Matrix3>
	Matrix1&
	BlasMatrixDomainAddin<Field, Matrix1, Matrix3 >::operator()(const Field& F,
								    Matrix1& C,
								    const Matrix3& B) const
	{
		linbox_check( C.rowdim() == B.rowdim());
		linbox_check( C.coldim() == B.coldim());
		typedef typename Matrix1::subMatrixType subMatrixType ;
		typedef typename Matrix1::constSubMatrixType constSubMatrixType ;
		constSubMatrixType B_v(B);
		subMatrixType C_v(C);

		FFLAS::faddin (F, C_v.rowdim(), C_v.coldim(),
			       B_v.getPointer(), B_v.getStride(),
			       C_v.getPointer(), C_v.getStride());
		return C;
	}


	// Sub
	template<class Field, class Matrix1, class Matrix2, class Matrix3>
	Matrix1&
	BlasMatrixDomainSub<Field, Matrix1, Matrix2, Matrix3 >::operator()(const Field& F,
									   Matrix1& C,
									   const Matrix2& A,
									   const Matrix3& B) const
	{
		linbox_check( A.rowdim() == B.rowdim());
		linbox_check( C.rowdim() == A.rowdim());
		linbox_check( A.coldim() == B.coldim());
		linbox_check( C.coldim() == A.coldim());
		typedef typename Matrix1::subMatrixType subMatrixType ;
		typedef typename Matrix1::constSubMatrixType constSubMatrixType ;
		constSubMatrixType A_v(A);
		constSubMatrixType B_v(B);
		subMatrixType C_v(C);

		FFLAS::fsub (F, C_v.rowdim(), C_v.coldim(),
			     A_v.getPointer(), A_v.getStride(),
			     B_v.getPointer(), B_v.getStride(),
			     C_v.getPointer(), C_v.getStride());
		return C;
	}

	// SubIn
	template<class Field, class Matrix1, class Matrix3>
	Matrix1&
	BlasMatrixDomainSubin<Field, Matrix1, Matrix3 >::operator()(const Field& F,
								    Matrix1& C,
								    const Matrix3& B) const
	{
		linbox_check( C.rowdim() == B.rowdim());
		linbox_check( C.coldim() == B.coldim());
		typedef typename Matrix1::subMatrixType subMatrixType ;
		typedef typename Matrix1::constSubMatrixType constSubMatrixType ;
		constSubMatrixType B_v(B);
		subMatrixType C_v(C);

		FFLAS::fsubin (F, C_v.rowdim(), C_v.coldim(),
			       B_v.getPointer(), B_v.getStride(),
			       C_v.getPointer(), C_v.getStride());
		return C;
	}

} // LinBox

namespace LinBox
{ /* Copy */
	//Copy
	template<class Field, class Matrix1, class Matrix2>
	Matrix1&
	BlasMatrixDomainCopy<Field, Matrix1, Matrix2 >::operator()(const Field& F,
								   Matrix1& B,
								   const Matrix2& A) const
	{
		linbox_check( A.rowdim() == B.rowdim());
		linbox_check( A.coldim() == B.coldim());
		typedef typename Matrix1::subMatrixType subMatrixType ;
		typedef typename Matrix1::constSubMatrixType constSubMatrixType ;
		constSubMatrixType A_v(A);
		subMatrixType B_v(B);

		for (size_t i=0; i<A.rowdim(); i++)
			FFLAS::fassign (F, A_v.coldim(),
				      A_v.getPointer() + i*A_v.getStride(), 1,
				      B_v.getPointer() + i*B_v.getStride(), 1);
		return B;
	}
} // LinBox

namespace LinBox
{ /* MulAdd */

	//  general matrix-matrix multiplication and addition with scaling
	// D= b11eta.C + alpha.A*B
#if 0
	template<class Field>
	class 	BlasMatrixDomainMulAdd<BlasMatrix<Field, _Rep>, BlasMatrix<Field, _Rep>, BlasMatrix<Field, _Rep> > {
	public:
		BlasMatrix<Field, _Rep>&
		operator()(//const Field                              & F,
			   BlasMatrix<Field, _Rep>      & D,
			   const typename Field::Element            & beta,
			   const BlasMatrix<Field, _Rep>& C,
			   const typename Field::Element            & alpha,
			   const BlasMatrix<Field, _Rep>& A,
			   const BlasMatrix<Field, _Rep>& B) const
		{
			linbox_check( A.coldim() == B.rowdim());
			linbox_check( C.rowdim() == A.rowdim());
			linbox_check( C.coldim() == B.coldim());
			linbox_check( D.rowdim() == C.rowdim());
			linbox_check( D.coldim() == C.coldim());

			D=C;

			FFLAS::fgemm( C.field(), FFLAS::FflasNoTrans, FFLAS::FflasNoTrans,
				     C.rowdim(), C.coldim(), A.coldim(),
				     alpha,
				     A.getPointer(), A.getStride(),
				     B.getPointer(), B.getStride(),
`				     beta,
				     D.getPointer(), D.getStride());
			return D;
		}


		BlasMatrix<Field, _Rep>&
		operator() (//const Field                              & F,
			    const typename Field::Element            & beta,
			    BlasMatrix<Field, _Rep>      & C,
			    const typename Field::Element            & alpha,
			    const BlasMatrix<Field, _Rep>& A,
			    const BlasMatrix<Field, _Rep>& B) const
		{
			linbox_check( A.coldim() == B.rowdim());
			linbox_check( C.rowdim() == A.rowdim());
			linbox_check( C.coldim() == B.coldim());

			FFLAS::fgemm( C.field(), FFLAS::FflasNoTrans, FFLAS::FflasNoTrans,
				     C.rowdim(), C.coldim(), A.coldim(),
				     alpha,
				     A.getPointer(), A.getStride(),
				     B.getPointer(), B.getStride(),
				     beta,
				     C.getPointer(), C.getStride());
			return C;
		}
	};
#endif


#if 1
	template<class Matrix1, class Matrix2, class Matrix3>
	class 	BlasMatrixDomainMulAdd {
	public:
		typedef typename Matrix1::Field Field;

		template<class _Rep>
		Matrix1&
		operator()( Matrix1                       & D,
			    const typename Field::Element & beta,
			    const BlasMatrix<Field, _Rep>       & C,
			    const typename Field::Element & alpha,
			    const Matrix1                 & A,
			    const Matrix2                 & B) const
		{
			linbox_check( A.coldim() == B.rowdim());
			linbox_check( C.rowdim() == A.rowdim());
			linbox_check( C.coldim() == B.coldim());
			linbox_check( D.rowdim() == C.rowdim());
			linbox_check( D.coldim() == C.coldim());
			typedef typename Matrix1::subMatrixType subMatrixType ;
			typedef typename Matrix1::constSubMatrixType constSubMatrixType ;

			D.copy(C);

			constSubMatrixType A_v(A);
			constSubMatrixType B_v(B);
			constSubMatrixType C_v(C);
			subMatrixType D_v(D);

                        FFLAS::fgemm( C.field(), FFLAS::FflasNoTrans, FFLAS::FflasNoTrans,
				     C_v.rowdim(), C_v.coldim(), A_v.coldim(),
				     alpha,
				     A_v.getPointer(), A_v.getStride(),
				     B_v.getPointer(), B_v.getStride(),
				     beta,
				     D_v.getWritePointer(), D_v.getStride());
			return D;
		}

		Matrix1&
		operator() ( const typename Field::Element              & beta,
			     Matrix1                                    & C,
			     const typename Field::Element              & alpha,
			     const Matrix2                              & A,
			     const Matrix3                              & B) const
		{
			linbox_check( A.coldim() == B.rowdim());
			linbox_check( C.rowdim() == A.rowdim());
			linbox_check( C.coldim() == B.coldim());
			typedef typename Matrix1::subMatrixType subMatrixType ;
			typedef typename Matrix1::constSubMatrixType constSubMatrixType ;
			constSubMatrixType A_v(A);
			constSubMatrixType B_v(B);
			subMatrixType C_v(C);

			FFLAS::fgemm( C.field(), FFLAS::FflasNoTrans, FFLAS::FflasNoTrans,
				     C_v.rowdim(), C_v.coldim(), A_v.coldim(),
				     alpha,
				     A_v.getPointer(), A_v.getStride(),
				     B_v.getPointer(), B_v.getStride(),
				     beta,
				     C_v.getWritePointer(), C_v.getStride());
			return C;
		}
	};

#endif

	// One specialization with BlasSubmatrix (needed by BlockMasseyDomain
	// other specialisation need to be done ...
	template<class Matrix, class _Rep>
	class 	BlasMatrixDomainMulAdd<BlasSubmatrix<Matrix>, BlasSubmatrix<Matrix>, BlasMatrix<typename Matrix::Field, _Rep> > {
	public:
		typedef typename Matrix::Field Field;
		BlasSubmatrix<Matrix>&
		operator()( BlasSubmatrix<Matrix>         & D,
			    const typename Field::Element & beta,
			    const BlasSubmatrix<Matrix>   & C,
			    const typename Field::Element & alpha,
			    const BlasSubmatrix<Matrix>   & A,
			    const BlasMatrix<Field, _Rep>       & B) const
		{
			linbox_check( A.coldim() == B.rowdim());
			linbox_check( C.rowdim() == A.rowdim());
			linbox_check( C.coldim() == B.coldim());
			linbox_check( D.rowdim() == C.rowdim());
			linbox_check( D.coldim() == C.coldim());

			D.copy(C);

			FFLAS::fgemm( C.field(), FFLAS::FflasNoTrans, FFLAS::FflasNoTrans,
				     C.rowdim(), C.coldim(), A.coldim(),
				     alpha,
				     A.getPointer(), A.getStride(),
				     B.getPointer(), B.getStride(),
				     beta,
				     D.getPointer(), D.getStride());
			return D;
		}


		BlasSubmatrix<Matrix>&
		operator() ( const typename Field::Element & beta,
			     BlasSubmatrix<Matrix>         & C,
			     const typename Field::Element & alpha,
			     const BlasSubmatrix<Matrix>   & A,
			     const BlasMatrix<Field, _Rep>       & B) const
		{
			linbox_check( A.coldim() == B.rowdim());
			linbox_check( C.rowdim() == A.rowdim());
			linbox_check( C.coldim() == B.coldim());

			FFLAS::fgemm( C.field(), FFLAS::FflasNoTrans, FFLAS::FflasNoTrans,
				     C.rowdim(), C.coldim(), A.coldim(),
				     alpha,
				     A.getPointer(), A.getStride(),
				     B.getPointer(), B.getStride(),
				     beta,
				     C.getPointer(), C.getStride());
			return C;
		}
	};

	template<class Matrix>
	class 	BlasMatrixDomainMulAdd<BlasSubmatrix<Matrix>, BlasSubmatrix<Matrix>, BlasSubmatrix<Matrix> > {
	public:
		typedef typename Matrix::Field Field;
		BlasSubmatrix<Matrix>&
		operator()( BlasSubmatrix<Matrix>         & D,
			    const typename Field::Element & beta,
			    const BlasSubmatrix<Matrix>   & C,
			    const typename Field::Element & alpha,
			    const BlasSubmatrix<Matrix>   & A,
			    const BlasSubmatrix<Matrix>   & B) const
		{
			linbox_check( A.coldim() == B.rowdim());
			linbox_check( C.rowdim() == A.rowdim());
			linbox_check( C.coldim() == B.coldim());
			linbox_check( D.rowdim() == C.rowdim());
			linbox_check( D.coldim() == C.coldim());

			D.copy(C);

			FFLAS::fgemm( C.field(), FFLAS::FflasNoTrans, FFLAS::FflasNoTrans,
				     C.rowdim(), C.coldim(), A.coldim(),
				     alpha,
				     A.getPointer(), A.getStride(),
				     B.getPointer(), B.getStride(),
				     beta,
				     D.getPointer(), D.getStride());
			return D;
		}


		BlasSubmatrix<Matrix>&
		operator() ( const typename Field::Element & beta,
			     BlasSubmatrix<Matrix>         & C,
			     const typename Field::Element & alpha,
			     const BlasSubmatrix<Matrix>   & A,
			     const BlasSubmatrix<Matrix>   & B) const
		{
			linbox_check( A.coldim() == B.rowdim());
			linbox_check( C.rowdim() == A.rowdim());
			linbox_check( C.coldim() == B.coldim());

			FFLAS::fgemm( C.field(), FFLAS::FflasNoTrans, FFLAS::FflasNoTrans,
				     C.rowdim(), C.coldim(), A.coldim(),
				     alpha,
				     A.getPointer(), A.getStride(),
				     B.getPointer(), B.getStride(),
				     beta,
				     C.getWritePointer(), C.getStride());
			return C;
		}
	};


	template<class Field, class _Rep>
	class 	BlasMatrixDomainMulAdd< BlasMatrix<Field, _Rep>, TransposedBlasMatrix<BlasMatrix<Field, _Rep> >, BlasMatrix<Field, _Rep> > {
	public:
		BlasMatrix<Field, _Rep>&
		operator()( BlasMatrix<Field, _Rep>                              & D,
			    const typename Field::Element                  & beta,
			    const BlasMatrix<Field, _Rep>                        & C,
			    const typename Field::Element                  & alpha,
			    const TransposedBlasMatrix<BlasMatrix<Field, _Rep> > & A,
			    const BlasMatrix<Field, _Rep>                        & B) const
		{
			linbox_check( A.getMatrix().rowdim() == B.rowdim());
			linbox_check( C.rowdim() == A.getMatrix().coldim());
			linbox_check( C.coldim() == B.coldim());
			linbox_check( D.rowdim() == C.rowdim());
			linbox_check( D.coldim() == C.coldim());

			D=C;

			FFLAS::fgemm( B.field(), FFLAS::FflasTrans, FFLAS::FflasNoTrans,
				     C.rowdim(), C.coldim(), B.rowdim(),
				     alpha,
				     A.getMatrix().getPointer(), A.getMatrix().getStride(),
				     B.getPointer(), B.getStride(),
				     beta,
				     D.getPointer(), D.getStride());

			return D;
		}


		BlasMatrix<Field, _Rep>&
		operator() ( const typename Field::Element                  & beta,
			     BlasMatrix<Field, _Rep>                              & C,
			     const typename Field::Element                  & alpha,
			     const TransposedBlasMatrix<BlasMatrix<Field, _Rep> > & A,
			     const BlasMatrix<Field, _Rep>                        & B) const
		{
			linbox_check( A.getMatrix().rowdim() == B.rowdim());
			linbox_check( C.rowdim() == A.getMatrix().coldim());
			linbox_check( C.coldim() == B.coldim());

			FFLAS::fgemm( B.field(), FFLAS::FflasTrans, FFLAS::FflasNoTrans,
				     C.rowdim(), C.coldim(), B.rowdim(),
				     alpha,
				     A.getMatrix().getPointer(), A.getMatrix().getStride(),
				     B.getPointer(), B.getStride(),
				     beta,
				     C.getPointer(), C.getStride());
			return C;
		}
	};

	template<class Field, class _Rep>
	class 	BlasMatrixDomainMulAdd< BlasMatrix<Field, _Rep>, TransposedBlasMatrix<BlasMatrix<Field, _Rep> >, TransposedBlasMatrix<BlasMatrix<Field, _Rep> > > {
	public:
		BlasMatrix<Field, _Rep>&
		operator()( BlasMatrix<Field, _Rep>                              & D,
			    const typename Field::Element                  & beta,
			    const BlasMatrix<Field, _Rep>                        & C,
			    const typename Field::Element                  & alpha,
			    const TransposedBlasMatrix<BlasMatrix<Field, _Rep> > & A,
			    const TransposedBlasMatrix<BlasMatrix<Field, _Rep> > & B) const
		{
			linbox_check( A.getMatrix().rowdim() == B.getMatrix().coldim());
			linbox_check( C.rowdim() == A.getMatrix().coldim());
			linbox_check( C.coldim() == B.getMatrix().rowdim());
			linbox_check( D.rowdim() == C.rowdim());
			linbox_check( D.coldim() == C.coldim());

			D=C;

			FFLAS::fgemm( C.field(), FFLAS::FflasTrans, FFLAS::FflasTrans,
				     C.rowdim(), C.coldim(), A.getMatrix().rowdim(),
				     alpha,
				     A.getMatrix().getPointer(), A.getMatrix().getStride(),
				     B.getMatrix().getPointer(), B.getMatrix().getStride(),
				     beta,
				     D.getPointer(), D.getStride());
			return D;
		}


		BlasMatrix<Field, _Rep>&
		operator() ( const typename Field::Element                  & beta,
			     BlasMatrix<Field, _Rep>                              & C,
			     const typename Field::Element                  & alpha,
			     const TransposedBlasMatrix<BlasMatrix<Field, _Rep> > & A,
			     const TransposedBlasMatrix<BlasMatrix<Field, _Rep> > & B) const
		{
			linbox_check( A.getMatrix().rowdim() == B.getMatrix().coldim());
			linbox_check( C.rowdim() == A.getMatrix().coldim());
			linbox_check( C.coldim() == B.getMatrix().rowdim());

			FFLAS::fgemm( C.field(), FFLAS::FflasTrans, FFLAS::FflasTrans,
				     C.rowdim(), C.coldim(), A.getMatrix().rowdim(),
				     alpha,
				     A.getMatrix().getPointer(), A.getMatrix().getStride(),
				     B.getMatrix().getPointer(), B.getMatrix().getStride(),
				     beta,
				     C.getPointer(), C.getStride());
			return C;
		}
	};

	template<class Field, class _Rep>
	class 	BlasMatrixDomainMulAdd< BlasMatrix<Field, _Rep>, BlasMatrix<Field, _Rep>, TransposedBlasMatrix<BlasMatrix<Field, _Rep> > > {
	public:
		BlasMatrix<Field, _Rep>&
		operator()( BlasMatrix<Field, _Rep>                              & D,
			    const typename Field::Element                  & beta,
			    const BlasMatrix<Field, _Rep>                        & C,
			    const typename Field::Element                  & alpha,
			    const BlasMatrix<Field, _Rep>                        & A,
			    const TransposedBlasMatrix<BlasMatrix<Field, _Rep> > & B) const
		{
			linbox_check( A.coldim() == B.getMatrix().coldim());
			linbox_check( C.rowdim() == A.rowdim());
			linbox_check( C.coldim() == B.getMatrix().rowdim());
			linbox_check( D.rowdim() == C.rowdim());
			linbox_check( D.coldim() == C.coldim());

			D=C;
			FFLAS::fgemm( C.field(), FFLAS::FflasNoTrans, FFLAS::FflasTrans,
				     C.rowdim(), C.coldim(), A.coldim(),
				     alpha,
				     A.getPointer(), A.getStride(),
				     B.getMatrix().getPointer(), B.getMatrix().getStride(),
				     beta,
				     D.getPointer(), D.getStride());
			return D;
		}


		BlasMatrix<Field, _Rep>&
		operator() ( const typename Field::Element                  & beta,
			     BlasMatrix<Field, _Rep>                              & C,
			     const typename Field::Element                  & alpha,
			     const BlasMatrix<Field, _Rep>                        & A,
			     const TransposedBlasMatrix<BlasMatrix<Field, _Rep> > & B) const
		{
			linbox_check( A.coldim() == B.getMatrix().coldim());
			linbox_check( C.rowdim() == A.rowdim());
			linbox_check( C.coldim() == B.getMatrix().rowdim());

			FFLAS::fgemm( C.field(), FFLAS::FflasNoTrans, FFLAS::FflasTrans,
				     C.rowdim(), C.coldim(), A.coldim(),
				     alpha,
				     A.getPointer(), A.getStride(),
				     B.getMatrix().getPointer(), B.getMatrix().getStride(),
				     beta,
				     C.getPointer(), C.getStride());
			return C;
		}
	};

#if 1
	/*
	 * specialization for Operand1 and Operand3 of type std::vector<Element>
	 * and Operand2 of type BlasMatrix<Field, _Rep>
	 */

	//  general matrix-vector multiplication and addition with scaling
	// d = beta.c + alpha.A*b
	template<class Field, class _Rep>
	class BlasMatrixDomainMulAdd<std::vector<typename Field::Element>, BlasMatrix<Field, _Rep>, std::vector<typename Field::Element> > {
	public:
		std::vector<typename Field::Element>&
		operator() ( std::vector<typename Field::Element>       & d,
			     const typename Field::Element              & beta,
			     const std::vector<typename Field::Element> & c,
			     const typename Field::Element              & alpha,
			     const BlasMatrix<Field, _Rep>                    & A,
			     const std::vector<typename Field::Element> & b) const
		{
			linbox_check( A.coldim() == b.size());
			linbox_check( c.size()   == b.size());
			linbox_check( d.size()   == c.size());
			d=c;

			FFLAS::fgemv( A.field(), FFLAS::FflasNoTrans,
				     A.rowdim(), A.coldim(),
				     alpha,
				     A.getPointer(), A.getStride(),
				     &b[0],1,
				     beta,
				     &d[0],1);
			return d;
		}


		std::vector<typename Field::Element>&
		operator() ( const typename Field::Element              & beta,
			     std::vector<typename Field::Element>       & c,
			     const typename Field::Element              & alpha,
			     const BlasMatrix<Field, _Rep>                    & A,
			     const std::vector<typename Field::Element> & b) const
		{
			linbox_check( A.coldim() == b.size());
			linbox_check( A.rowdim() == c.size()); //fixed: dpritcha

			FFLAS::fgemv( A.field(), FFLAS::FflasNoTrans,
				     A.rowdim(), A.coldim(),
				     alpha,
				     A.getPointer(), A.getStride(),
				     &b[0],1,
				     beta,
				     &c[0],1);
			return c;
		}
	};


	//  general matrix-vector multiplication and addition with scaling
	// d = beta.c + alpha.A*b
	//! what about subvector/submatrices ?
	template<class Field, class _Rep>
	class BlasMatrixDomainMulAdd<BlasVector<Field>, BlasMatrix<Field, _Rep>, BlasVector<Field> > {
	public:
		BlasVector<Field>&
		operator() ( BlasVector<Field>             & d,
			     const typename Field::Element & beta,
			     const BlasVector<Field>       & c,
			     const typename Field::Element & alpha,
			     const BlasMatrix<Field, _Rep>       & A,
			     const BlasVector<Field>       & b) const
		{
			linbox_check( A.coldim() == b.size());
			linbox_check( c.size()   == b.size());
			linbox_check( d.size()   == c.size());
			d=c;

			FFLAS::fgemv( A.field(), FFLAS::FflasNoTrans,
				     A.rowdim(), A.coldim(),
				     alpha,
				     A.getPointer(), A.getStride(),
				     b.getPointer(),b.getStride(),
				     beta,
				     d.getWritePointer(),d.getStride());
			return d;
		}


		BlasVector<Field>&
		operator() ( const typename Field::Element & beta,
			     BlasVector<Field>             & c,
			     const typename Field::Element & alpha,
			     const BlasMatrix<Field, _Rep>       & A,
			     const BlasVector<Field>       & b) const
		{
			linbox_check( A.coldim() == b.size());
			linbox_check( A.rowdim() == c.size()); //fixed: dpritcha

			FFLAS::fgemv( A.field(), FFLAS::FflasNoTrans,
				     A.rowdim(), A.coldim(),
				     alpha,
				     A.getPointer(), A.getStride(),
				     b.getPointer(),b.getStride(),
				     beta,
				     c.getWritePointer(),c.getStride());
			return c;
		}
	};

	//  general vector-matrix multiplication and addition with scaling
	// d = beta.c + alpha.a*B -- note order of coldim, rowdim passed to fgemv is switched
	template<class Field, class _Rep>
	class BlasMatrixDomainMulAdd<std::vector<typename Field::Element>,std::vector<typename Field::Element>, BlasMatrix<Field, _Rep> > {
	public:
		std::vector<typename Field::Element>&
		operator() ( std::vector<typename Field::Element>       & d,
			     const typename Field::Element              & beta,
			     const std::vector<typename Field::Element> & c,
			     const typename Field::Element              & alpha,
			     const std::vector<typename Field::Element> & a,
			     const BlasMatrix<Field, _Rep>                    & B) const
		{
			linbox_check( B.rowdim() == a.size());
			linbox_check( B.coldim() == c.size());
			linbox_check( d.size()   == c.size());
			d=c;

			FFLAS::fgemv( B.field(), FFLAS::FflasTrans,
				     B.rowdim(), B.coldim(),
				     alpha,
				     B.getPointer(), B.getStride(),
				     &a[0],1,
				     beta,
				     &d[0],1);
			return d;
		}


		std::vector<typename Field::Element>&
		operator() ( const typename Field::Element              & beta,
			     std::vector<typename Field::Element>       & c,
			     const typename Field::Element              & alpha,
			     const std::vector<typename Field::Element> & a,
			     const BlasMatrix<Field, _Rep>                    & B) const
		{
			linbox_check( B.rowdim() == a.size());
			linbox_check( B.coldim() == c.size());

			FFLAS::fgemv( B.field(), FFLAS::FflasTrans,
				     B.rowdim(), B.coldim(),
				     alpha,
				     B.getPointer(), B.getStride(),
				     &a[0],1,
				     beta,

				     &c[0],1);
			return c;
		}
	};

	template<class Field, class _Rep>
	class BlasMatrixDomainMulAdd<BlasVector<Field>, BlasVector<Field>, BlasMatrix<Field, _Rep> > {
	public:
		BlasVector<Field>&
		operator() ( BlasVector<Field>              & d,
			     const typename Field::Element  & beta,
			     const BlasVector<Field>        & c,
			     const typename Field::Element  & alpha,
			     const BlasVector<Field>        & a,
			     const BlasMatrix<Field, _Rep>        & B) const
		{
			linbox_check( B.rowdim() == a.size());
			linbox_check( B.coldim() == c.size());
			linbox_check( d.size()   == c.size());
			d=c;

			FFLAS::fgemv( B.field(), FFLAS::FflasTrans,
				     B.rowdim(), B.coldim(),
				     alpha,
				     B.getPointer(), B.getStride(),
				     a.getPonter(),a.getStride(),
				     beta,
				     a.getWritePointer(),a.getStride());
			return d;
		}


		BlasVector<Field>&
		operator() ( const typename Field::Element              & beta,
			     BlasVector<Field>                          & c,
			     const typename Field::Element              & alpha,
			     const BlasVector<Field>                    & a,
			     const BlasMatrix<Field, _Rep>                    & B) const
		{
			linbox_check( B.rowdim() == a.size());
			linbox_check( B.coldim() == c.size());

			FFLAS::fgemv( B.field(), FFLAS::FflasTrans,
				     B.rowdim(), B.coldim(),
				     alpha,
				     B.getPointer(), B.getStride(),
				     a.getPointer(),a.getStride(),
				     beta,
				     c.getWritePointer(),c.getStride());
			return c;
		}
	};

#endif


	/*
	 * Specialization for Operand1, Operand2  of type BlasMatrix<Field, _Rep>
	 * and Operand3 of type BlasPermutation
	 */

	// Matrix permutation product C = A*B
	template<class Field, class _Rep>
	class BlasMatrixDomainMul<Field, BlasMatrix<Field, _Rep>, BlasMatrix<Field, _Rep>, BlasPermutation<size_t> > {
	public:
		BlasMatrix<Field, _Rep>& operator()(const Field& F,
						    BlasMatrix<Field, _Rep>& C,
						    const BlasMatrix<Field, _Rep>& A,
						    const BlasPermutation<size_t>& B) const
		{
			C = A;
			return BlasMatrixDomainMulin<Field, BlasMatrix<Field, _Rep>, BlasPermutation<size_t> >()( F, C, B);
		}
	};

	template<class Field, class _Rep>
	class BlasMatrixDomainMul<Field, BlasMatrix<Field, _Rep>, BlasPermutation<size_t>, BlasMatrix<Field, _Rep> > {
	public:
		BlasMatrix<Field, _Rep>& operator()(const Field& F,
						    BlasMatrix<Field, _Rep>& C,
						    const BlasPermutation<size_t>& B,
						    const BlasMatrix<Field, _Rep>& A) const
		{
			C = A;
			return BlasMatrixDomainMulin<Field, BlasMatrix<Field,_Rep>, BlasPermutation<size_t> >()( F, B, C);
		}
	};

	/*
	 * specialization for Operand1, Operand2  of type BlasMatrix<Field, _Rep> and Operand3 of type TransposedBlasMatrix<BlasPermutation<size_t> >
	 */

	// Matrix permutation product C = A*B
	template<class Field, class _Rep>
	class BlasMatrixDomainMul<Field, BlasMatrix<Field,_Rep>, BlasMatrix<Field,_Rep>, TransposedBlasMatrix<BlasPermutation<size_t> > > {
	public:
		BlasMatrix<Field, _Rep>& operator()(const Field& F,
						    BlasMatrix<Field, _Rep>& C,
						    const BlasMatrix<Field, _Rep>& A,
						    const TransposedBlasMatrix<BlasPermutation<size_t> >& B) const
		{
			C = A;
			return BlasMatrixDomainMulin<Field, BlasMatrix<Field,_Rep>, TransposedBlasMatrix<BlasPermutation<size_t> > >()( F, C, B);
		}
	};

	template<class Field, class _Rep>
	class BlasMatrixDomainMul<Field, BlasMatrix<Field,_Rep>, TransposedBlasMatrix<BlasPermutation<size_t> >, BlasMatrix<Field,_Rep> > {
	public:
		BlasMatrix<Field, _Rep>& operator()(const Field& F,
						    BlasMatrix<Field, _Rep>& C,
						    const TransposedBlasMatrix<BlasPermutation<size_t> >& B,
						    const BlasMatrix<Field, _Rep>& A) const
		{
			C = A;
			return BlasMatrixDomainMulin<Field, BlasMatrix<Field,_Rep>, TransposedBlasMatrix<BlasPermutation<size_t> > >()( F, B, C);
		}
	};

	/*
	 * specialization for Operand1 of type BlasMatrix<Field, _Rep> and Operand2 of type BlasPermutation
	 */

} // LinBox

namespace LinBox
{ /* Mulin */
	// In-place matrix permutation product
	template<class Field, class _Rep>
	class BlasMatrixDomainMulin<Field, BlasMatrix<Field,_Rep>, BlasPermutation<size_t> > {
	public:
		BlasMatrix<Field, _Rep>& operator()( const Field& F,
						     BlasMatrix<Field, _Rep>& A,
						     const BlasPermutation<size_t>& B) const
		{
			if (B.isIdentity()) return A ;
			linbox_check( A.coldim() >= B.getSize() );
			FFPACK::applyP( F, FFLAS::FflasRight, FFLAS::FflasNoTrans,
				       A.rowdim(), 0,(int) B.getOrder(),
				       A.getPointer(), A.getStride(), B.getPointer() );
			return A;
		}

		BlasMatrix<Field, _Rep>& operator()( const Field& F,
						     const BlasPermutation<size_t>& B,
						     BlasMatrix<Field, _Rep>& A) const
		{                        
			if (B.isIdentity()) return A ;
			linbox_check( A.rowdim() >= B.getSize() );
			FFPACK::applyP( F, FFLAS::FflasLeft, FFLAS::FflasNoTrans,
				       A.coldim(), 0,(int) B.getOrder(), A.getPointer(), A.getStride(), B.getPointer() );
			return A;
		}

	};

	template<class Field, class _Rep>
	class BlasMatrixDomainMulin<Field, BlasMatrix<Field,_Rep>, TransposedBlasMatrix<BlasPermutation<size_t> > > {
	public:
		BlasMatrix<Field, _Rep>& operator()( const Field& F,
						     BlasMatrix<Field, _Rep>& A,
						     const TransposedBlasMatrix<BlasPermutation<size_t> >& B) const
		{
			if (B.getMatrix().isIdentity()) return A ;
			linbox_check( A.coldim() >= B.getMatrix().getSize() );
			FFPACK::applyP( F, FFLAS::FflasRight, FFLAS::FflasTrans,
				       A.rowdim(), 0,(int) B.getMatrix().getOrder(),
				       A.getPointer(), A.getStride(), B.getMatrix().getPointer() );
			return A;
		}
		BlasMatrix<Field, _Rep>& operator()(  const Field& F,
						      const TransposedBlasMatrix<BlasPermutation<size_t> >& B,
						      BlasMatrix<Field, _Rep>& A) const
		{
			if (B.getMatrix().isIdentity()) return A ;
			linbox_check( A.rowdim() >= B.getMatrix().getSize() );
			FFPACK::applyP( F, FFLAS::FflasLeft, FFLAS::FflasTrans,
				       A.coldim(), 0,(int) B.getMatrix().getOrder(), A.getPointer(), A.getStride(), B.getMatrix().getPointer() );
			return A;
		}
	};



#if 1
	/*
	 * specialization for Operand1, Operand2  of type std::vector<Element> and Operand3 of type BlasPermutation
	 */

	// Matrix permutation product C = A*B
	template<class Field>
	class BlasMatrixDomainMul<Field,std::vector<typename Field::Element>,std::vector<typename Field::Element>, BlasPermutation<size_t> > {
	public:
		std::vector<typename Field::Element>& operator()(const Field& F,
								 std::vector<typename Field::Element>& C,
								 const std::vector<typename Field::Element>& A,
								 const BlasPermutation<size_t>& B) const
		{
			C = A;
			return BlasMatrixDomainMulin<Field,std::vector<typename Field::Element>, BlasPermutation<size_t> >()( F, C, B);
		}
	};

	template<class Field>
	class BlasMatrixDomainMul<Field,std::vector<typename Field::Element>, BlasPermutation<size_t>,std::vector<typename Field::Element> > {
	public:
		std::vector<typename Field::Element>& operator()(const Field& F,
								 std::vector<typename Field::Element>& C,
								 const BlasPermutation<size_t>& B,
								 const std::vector<typename Field::Element>& A) const
		{
			C = A;
			return BlasMatrixDomainMulin<Field,std::vector<typename Field::Element>, BlasPermutation<size_t> >()( F, B, C);
		}
	};


	template<class Field>
	class BlasMatrixDomainMul<Field, BlasVector<Field >, BlasVector<Field >, BlasPermutation<size_t> > {
	public:
		BlasVector<Field >& operator()(const Field& F,
					       BlasVector<Field >& C,
					       const BlasVector<Field >& A,
					       const BlasPermutation<size_t>& B) const
		{
			C = A;
			return BlasMatrixDomainMulin<Field, BlasVector<Field >, BlasPermutation<size_t> >()( F, C, B);
		}
	};

	template<class Field>
	class BlasMatrixDomainMul<Field, BlasVector<Field >, BlasPermutation<size_t>, BlasVector<Field > > {
	public:
		BlasVector<Field >& operator()(const Field& F,
					       BlasVector<Field >& C,
					       const BlasPermutation<size_t>& B,
					       const BlasVector<Field >& A) const
		{
			C = A;
			return BlasMatrixDomainMulin<Field, BlasVector<Field >, BlasPermutation<size_t> >()( F, B, C);
		}
	};


	/*
	 * specialization for Operand1, Operand2  of type std::vector<Element> and Operand3 of type TransposedBlasMatrix<BlasPermutation<size_t> >
	 */

	// Matrix permutation product C = A*B
	template<class Field>
	class BlasMatrixDomainMul<Field,std::vector<typename Field::Element>,std::vector<typename Field::Element>, TransposedBlasMatrix<BlasPermutation<size_t> > > {
	public:
		std::vector<typename Field::Element>& operator()(const Field& F,
								 std::vector<typename Field::Element>& C,
								 const std::vector<typename Field::Element>& A,
								 const TransposedBlasMatrix<BlasPermutation<size_t> >& B) const
		{
			C = A;
			return BlasMatrixDomainMulin<Field,std::vector<typename Field::Element>, TransposedBlasMatrix<BlasPermutation<size_t> > >()( F, C, B);
		}
	};

	template<class Field>
	class BlasMatrixDomainMul<Field,std::vector<typename Field::Element>, TransposedBlasMatrix<BlasPermutation<size_t> >,std::vector<typename Field::Element> > {
	public:
		std::vector<typename Field::Element>& operator()(const Field& F,
								 std::vector<typename Field::Element>& C,
								 const TransposedBlasMatrix<BlasPermutation<size_t> >& B,
								 const std::vector<typename Field::Element>& A) const
		{
			C = A;
			return BlasMatrixDomainMulin<Field,std::vector<typename Field::Element>, TransposedBlasMatrix<BlasPermutation<size_t> > >()( F, B, C);
		}
	};

	template<class Field>
	class BlasMatrixDomainMul<Field, BlasVector<Field>, BlasVector<Field>, TransposedBlasMatrix<BlasPermutation<size_t> > > {
	public:
		BlasVector<Field>& operator()(const Field& F,
					      BlasVector<Field>& C,
					      const BlasVector<Field>& A,
					      const TransposedBlasMatrix<BlasPermutation<size_t> >& B) const
		{
			C = A;
			return BlasMatrixDomainMulin<Field, BlasVector<Field>, TransposedBlasMatrix<BlasPermutation<size_t> > >()( F, C, B);
		}
	};

	template<class Field>
	class BlasMatrixDomainMul<Field, BlasVector<Field>, TransposedBlasMatrix<BlasPermutation<size_t> >, BlasVector<Field> > {
	public:
		BlasVector<Field>& operator()(const Field& F,
					      BlasVector<Field>& C,
					      const TransposedBlasMatrix<BlasPermutation<size_t> >& B,
					      const BlasVector<Field>& A) const
		{
			C = A;
			return BlasMatrixDomainMulin<Field, BlasVector<Field>, TransposedBlasMatrix<BlasPermutation<size_t> > >()( F, B, C);
		}
	};

	/*
	 * specialization for Operand1 of type std::vector<Element> and Operand2 of type BlasPermutation
	 */

	// In-place matrix permutation product
	template<class Field>
	class BlasMatrixDomainMulin<Field,std::vector<typename Field::Element>, BlasPermutation<size_t> > {
	public:
		std::vector<typename Field::Element>& operator()( const Field& F,
								  std::vector<typename Field::Element>& A,
								  const BlasPermutation<size_t>& B) const
		{
			if (B.isIdentity()) return A ;
                        //linbox_check( A.size() == B.getSize() );
                        linbox_check( A.size() == B.getOrder() );
			FFPACK::applyP( F, FFLAS::FflasRight, FFLAS::FflasNoTrans,
				       1, 0,(int) B.getOrder(), &A[0], 1, B.getPointer() );
			return A;
		}

		std::vector<typename Field::Element>& operator()( const Field& F,
								  const BlasPermutation<size_t>& B,
								  std::vector<typename Field::Element>& A) const
		{
			if (B.isIdentity()) return A ;
			//linbox_check( A.size() >= B.getSize() );
                        linbox_check( A.size() == B.getOrder() );
			FFPACK::applyP( F, FFLAS::FflasLeft, FFLAS::FflasNoTrans,
				       1, 0,(int) B.getOrder(), &A[0], 1, B.getPointer() );
			return A;
		}

	};

	template<class Field>
	class BlasMatrixDomainMulin<Field, BlasVector<Field>, BlasPermutation<size_t> > {
	public:
		BlasVector< Field >& operator()( const Field& F,
						 BlasVector< Field>& A,
						 const BlasPermutation<size_t>& B) const
		{
			if (B.isIdentity()) return A ;
			linbox_check( A.size() == B.getSize() );
			FFPACK::applyP( F, FFLAS::FflasRight, FFLAS::FflasNoTrans,
				       1, 0,(int) B.getOrder(), A.getPointer(), 1, B.getPointer() );
			return A;
		}

		BlasVector< Field >& operator()( const Field& F,
						 const BlasPermutation<size_t>& B,
						 BlasVector< Field>& A) const
		{
			if (B.isIdentity()) return A ;
			linbox_check( A.size() >= B.getSize() );
			FFPACK::applyP( F, FFLAS::FflasLeft, FFLAS::FflasNoTrans,
				       1, 0,(int) B.getOrder(), A.getPointer(), 1, B.getPointer() );
			return A;
		}

	};

	template<class Field>
	class BlasMatrixDomainMulin<Field,std::vector<typename Field::Element>, TransposedBlasMatrix<BlasPermutation<size_t> > > {
	public:
		std::vector<typename Field::Element>& operator()( const Field& F,
								  std::vector<typename Field::Element>& A,
								  const TransposedBlasMatrix<BlasPermutation<size_t> >& B) const
		{
			if (B.getMatrix().isIdentity()) return A ;
			linbox_check( A.size() >= B.getMatrix().getSize() );
			FFPACK::applyP( F, FFLAS::FflasRight, FFLAS::FflasTrans,
				       1, 0,(int) B.getMatrix().getOrder(),
				       &A[0], 1, B.getMatrix().getPointer() );
			return A;
		}
		std::vector<typename Field::Element>& operator()(  const Field& F,
								   const TransposedBlasMatrix<BlasPermutation<size_t> >& B,
								   std::vector<typename Field::Element>& A) const
		{
			if (B.getMatrix().isIdentity()) return A ;
			linbox_check( A.size() >= B.getMatrix().getSize() );
			FFPACK::applyP( F, FFLAS::FflasLeft, FFLAS::FflasTrans,
				       1, 0,(int) B.getMatrix().getOrder(), &A[0], 1, B.getMatrix().getPointer() );
			return A;
		}
	};

	template<class Field>
	class BlasMatrixDomainMulin<Field, BlasVector<Field>, TransposedBlasMatrix<BlasPermutation<size_t> > > {
	public:
		BlasVector<Field>& operator()( const Field& F,
					       BlasVector<Field>& A,
					       const TransposedBlasMatrix<BlasPermutation<size_t> >& B) const
		{
			if (B.getMatrix().isIdentity()) return A ;
			linbox_check( A.size() >= B.getMatrix().getSize() );
			FFPACK::applyP( F, FFLAS::FflasRight, FFLAS::FflasTrans,
				       1, 0,(int) B.getMatrix().getOrder(),
				       &A[0], 1, B.getMatrix().getPointer() );
			return A;
		}
		BlasVector<Field>& operator()(  const Field& F,
						const TransposedBlasMatrix<BlasPermutation<size_t> >& B,
						BlasVector<Field>& A) const
		{
			if (B.getMatrix().isIdentity()) return A ;
			linbox_check( A.size() >= B.getMatrix().getSize() );
			FFPACK::applyP( F, FFLAS::FflasLeft, FFLAS::FflasTrans,
				       1, 0,(int) B.getMatrix().getOrder(), &A[0], 1, B.getMatrix().getPointer() );
			return A;
		}
	};

#endif

	/*
	 * specialization for Operand1 of type BlasMatrix<Field, _Rep> and Operand2
	 * of type TriangularBlasMatrix<Field,_Rep>
	 */

	// Matrix/Triangular product C = A*B
	template<class Field, class _Rep>
	class BlasMatrixDomainMul<Field, BlasMatrix<Field,_Rep>, BlasMatrix<Field,_Rep>, TriangularBlasMatrix<Field,_Rep> > {
	public:
		BlasMatrix<Field, _Rep>& operator()(const Field& F,
						    BlasMatrix<Field, _Rep>& C,
						    const BlasMatrix<Field, _Rep>& A,
						    const TriangularBlasMatrix<Field,_Rep>& B) const
		{
			C = A;
			return BlasMatrixDomainMulin<Field, BlasMatrix<Field,_Rep>, TriangularBlasMatrix<Field,_Rep> >()( F, C, B);
		}
	};

	template<class Field, class _Rep>
	class BlasMatrixDomainMul<Field, BlasMatrix<Field,_Rep>, TriangularBlasMatrix<Field,_Rep>, BlasMatrix<Field,_Rep> > {
	public:
		BlasMatrix<Field, _Rep>& operator()(const Field& F,
						    BlasMatrix<Field, _Rep>& C,
						    const TriangularBlasMatrix<Field,_Rep>& B,
						    const BlasMatrix<Field, _Rep>& A) const
		{
			C = A;
			return BlasMatrixDomainMulin<Field, BlasMatrix<Field,_Rep>, TriangularBlasMatrix<Field,_Rep> >()( F, B, C);
		}
	};

	/*
	 * specialization for Operand1 of type BlasMatrix<Field, _Rep> and Operand2 of type TriangularBlasMatrix<Field,_Rep>
	 */

	// In-place matrix*triangular matrix product
	template<class Field, class _Rep>
	class BlasMatrixDomainMulin<Field, BlasMatrix<Field,_Rep>,
	      TriangularBlasMatrix<Field,_Rep> >{
	      public:
		      BlasMatrix<Field, _Rep>& operator()( const Field& F,
							   BlasMatrix<Field, _Rep>& A,
							   const TriangularBlasMatrix<Field,_Rep>& B) const
		      {
			      linbox_check( A.coldim() == B.rowdim() );

			      FFLAS::ftrmm( F, FFLAS::FflasRight, (FFLAS::FFLAS_UPLO) (B.getUpLo()),
					   FFLAS::FflasNoTrans,(FFLAS::FFLAS_DIAG) (B.getDiag()),
					   A.rowdim(), A.coldim(), F.one,
					   B.getPointer(), B.getStride(), A.getPointer(), A.getStride() );
			      return A;
		      }

		      BlasMatrix<Field, _Rep>& operator()( const Field& F,
							   const TriangularBlasMatrix<Field,_Rep>& B,
							   BlasMatrix<Field, _Rep>& A) const
		      {
			      linbox_check( B.coldim() == A.rowdim() );
			      FFLAS::ftrmm( F, FFLAS::FflasLeft, (FFLAS::FFLAS_UPLO)(B.getUpLo()),
					   FFLAS::FflasNoTrans, (FFLAS::FFLAS_DIAG) (B.getDiag()),
					   A.rowdim(), A.coldim(), F.one,
					   B.getPointer(), B.getStride(),
					   A.getPointer(), A.getStride() );
			      return A;
		      }
	      };


	/*! @internal In-place matrix*triangular matrix product with transpose.
	*/
	template<class Field, class _Rep>
	class BlasMatrixDomainMulin<Field, BlasMatrix<Field,_Rep>,
	      TransposedBlasMatrix<TriangularBlasMatrix<Field,_Rep> > >{
	      public:
		      BlasMatrix<Field, _Rep>& operator()( const Field& F,
							   BlasMatrix<Field, _Rep>& A,
							   const TransposedBlasMatrix< TriangularBlasMatrix<Field,_Rep> >& B) const
		      {
			      linbox_check( B.getMatrix().coldim() == A.coldim() );

			      FFLAS::ftrmm( F, FFLAS::FflasRight,
					   (FFLAS::FFLAS_UPLO)(B.getMatrix().getUpLo()),
					   FFLAS::FflasTrans,
					   (FFLAS::FFLAS_DIAG) (B.getMatrix().getDiag()),
					   A.rowdim(), A.coldim(),
					   F.one,
					   B.getMatrix().getPointer(), B.getMatrix().getStride(),
					   A.getPointer(), A.getStride() );
			      return A;
		      }

		      BlasMatrix<Field, _Rep>& operator()( const Field& F,
							   const TransposedBlasMatrix< TriangularBlasMatrix<Field,_Rep> >& B,
							   BlasMatrix<Field, _Rep>& A) const
		      {
			      linbox_check( B.getMatrix().coldim() == A.rowdim() );
			      FFLAS::ftrmm( F, FFLAS::FflasLeft,
					   (FFLAS::FFLAS_UPLO) (B.getMatrix().getUpLo()),
					   FFLAS::FflasTrans,
					   (FFLAS::FFLAS_DIAG) (B.getMatrix().getDiag()),
					   A.rowdim(), A.coldim(), F.one,
					   B.getMatrix().getPointer(), B.getMatrix().getStride(),
					   A.getPointer(), A.getStride() );
			      return A;
		      }
	      };



	/*
	 * specialization for Operand1 of type TriangularBlasMatrix<Field,_Rep> and Operand2 of type BlasPermutation
	 */

	// Matrix permutation product C = A*B
	template<class Field, class _Rep>
	class BlasMatrixDomainMul<Field, BlasMatrix<Field,_Rep>, TriangularBlasMatrix<Field,_Rep>, BlasPermutation<size_t> > {
	public:
		BlasMatrix<Field, _Rep>& operator()(const Field& F,
						    BlasMatrix<Field, _Rep>& C,
						    const TriangularBlasMatrix<Field,_Rep>& A,
						    const BlasPermutation<size_t>& B) const
		{
			C = A;
			return BlasMatrixDomainMulin<Field, BlasMatrix<Field,_Rep>, BlasPermutation<size_t> >()( F, C, B);
		}
	};

	template<class Field, class _Rep>
	class BlasMatrixDomainMul<Field, BlasMatrix<Field,_Rep>, BlasPermutation<size_t>, TriangularBlasMatrix<Field,_Rep> > {
	public:
		BlasMatrix<Field, _Rep>& operator()(const Field& F,
						    BlasMatrix<Field, _Rep>& C,
						    const BlasPermutation<size_t>& B,
						    const TriangularBlasMatrix<Field,_Rep>& A) const
		{
			C = A;
			return BlasMatrixDomainMulin<Field, BlasMatrix<Field,_Rep>, BlasPermutation<size_t> >()( F, B, C);
		}
	};

} // LinBox

namespace LinBox
{ /* Solve */

	/*
	 * Specialization for Operand of type BlasMatrix<Field, _Rep>
	 */

	template <class Field, class Operand1, class Matrix, class Operand2>
	Operand1&
	BlasMatrixDomainLeftSolve<Field, Operand1, Matrix, Operand2 >::operator() (const Field& F,
										   Operand1& X,
										   const Matrix& A,
										   const Operand2& B) const
	{
		LQUPMatrix<Field> LQUP(A);
		LQUP.left_solve(X, B);
		return X;
	}

	template <class Field, class Operand1, class Matrix, class Operand2>
	Operand1&
	BlasMatrixDomainLeftSolve<Field, Operand1, Matrix, Operand2 >::operator() (const Field& F,
										   const Matrix& A,
										   Operand1& B) const
	{
		LQUPMatrix<Field> LQUP(A);
		LQUP.left_solve(B);
		return B;
	}

	template <class Field, class Operand1, class Matrix, class Operand2>
	Operand1&
	BlasMatrixDomainRightSolve<Field, Operand1, Matrix, Operand2 >::operator() (const Field& F,
										    Operand1& X,
										    const Matrix& A,
										    const Operand2& B) const
	{
		// typename Matrix::subMatrixType subMatrixType ;
		LQUPMatrix<Field> LQUP(A);
		LQUP.right_solve(X, B);
		return X;
	}

	template <class Field, class Operand1, class Matrix, class Operand2>
	Operand1&
	BlasMatrixDomainRightSolve<Field, Operand1, Matrix, Operand2 >::operator() (const Field& F,
										    const Matrix& A,
										    Operand1& B) const
	{
		LQUPMatrix<Field> LQUP(A);
		LQUP.right_solve(B);
		return B;
	}


	template <class Matrix>
	class BlasMatrixDomainRightSolve<typename Matrix::Field, BlasSubmatrix<Matrix>, BlasSubmatrix<Matrix> > {
	public:
		typedef typename Matrix::Field Field;
		BlasSubmatrix<Matrix>& operator() (const Field& F,
						   BlasSubmatrix<Matrix>& X,
						   const BlasSubmatrix<Matrix>& A,
						   const BlasSubmatrix<Matrix>& B) const
		{
			LQUPMatrix<Field> LQUP(A);
			LQUP.right_solve(X, B);
			return X;
		}


		BlasSubmatrix<Matrix>& operator() (const Field& F,
						   const BlasSubmatrix<Matrix>& A,
						   BlasSubmatrix<Matrix>& B) const
		{
			LQUPMatrix<Field> LQUP(A);
			LQUP.right_solve(B);
			return B;
		}

	};

#if 1
	/*
	 * Specialization for Operand of type std::vector<Element>
	 */

	template <class Field, class _Rep>
	class BlasMatrixDomainLeftSolve<Field, std::vector<typename Field::Element>, BlasMatrix<Field, _Rep> > {
	public:
		std::vector<typename Field::Element>& operator() (const Field& F,
								  std::vector<typename Field::Element>& X,
								  const BlasMatrix<Field, _Rep>& A,
								  const std::vector<typename Field::Element>& B) const
		{
			LQUPMatrix<Field> LQUP(A);
			LQUP.left_solve(X, B);
			return X;
		}

		std::vector<typename Field::Element>& operator()(const Field& F,
								 const BlasMatrix<Field, _Rep>& A,
								 std::vector<typename Field::Element>& B) const
		{
			LQUPMatrix<Field> LQUP(A);
			LQUP.left_solve(B);
			return B;
		}

	};

	template <class Field, class _Rep>
	class BlasMatrixDomainRightSolve<Field, std::vector<typename Field::Element>, BlasMatrix<Field, _Rep> > {
	public:
		std::vector<typename Field::Element>& operator() (const Field& F,
								  std::vector<typename Field::Element>& X,
								  const BlasMatrix<Field, _Rep>& A,
								  const std::vector<typename Field::Element>& B) const
		{
			LQUPMatrix<Field> LQUP(A);
			LQUP.right_solve(X, B);
			return X;
		}

		std::vector<typename Field::Element>& operator() (const Field& F,
								  const BlasMatrix<Field, _Rep>& A,
								  std::vector<typename Field::Element>& B) const
		{
			LQUPMatrix<Field> LQUP(A);
			LQUP.right_solve(B);
			return B;
		}

	};

	template <class Field, class _Rep>
	class BlasMatrixDomainLeftSolve<Field, BlasVector<Field>, BlasMatrix<Field, _Rep> > {
	public:
		BlasVector<Field>& operator() (const Field& F,
					       BlasVector<Field>& X,
					       const BlasMatrix<Field, _Rep>& A,
					       const BlasVector<Field>& B) const
		{
			LQUPMatrix<Field> LQUP(A);
			LQUP.left_solve(X, B);
			return X;
		}

		BlasVector<Field>& operator()(const Field& F,
					      const BlasMatrix<Field, _Rep>& A,
					      BlasVector<Field>& B) const
		{
			LQUPMatrix<Field> LQUP(A);
			LQUP.left_solve(B);
			return B;
		}

	};

	template <class Field, class _Rep>
	class BlasMatrixDomainRightSolve<Field, BlasVector<Field>, BlasMatrix<Field, _Rep> > {
	public:
		BlasVector<Field>& operator() (const Field& F,
					       BlasVector<Field>& X,
					       const BlasMatrix<Field, _Rep>& A,
					       const BlasVector<Field>& B) const
		{
			LQUPMatrix<Field> LQUP(A);
			LQUP.right_solve(X, B);
			return X;
		}

		BlasVector<Field>& operator() (const Field& F,
					       const BlasMatrix<Field, _Rep>& A,
					       BlasVector<Field>& B) const
		{
			LQUPMatrix<Field> LQUP(A);
			LQUP.right_solve(B);
			return B;
		}

	};

#endif


	/*
	 * ********************************************************
	 * *** Specialization for TriangularBlasMatrix<Field,_Rep> ***
	 * ********************************************************
	 */


	/*
	 * specialization for Operand of type BlasMatrix<Field, _Rep>
	 */

	template <class Field, class _Rep>
	class BlasMatrixDomainLeftSolve<Field, BlasMatrix<Field, _Rep>, TriangularBlasMatrix<Field,_Rep> > {
	public:
		BlasMatrix<Field, _Rep>& operator() (const Field& F,
						     BlasMatrix<Field, _Rep>& X,
						     const TriangularBlasMatrix<Field,_Rep>& A,
						     const BlasMatrix<Field, _Rep>& B) const
		{

			linbox_check( X.rowdim() == B.rowdim());
			linbox_check( X.coldim() == B.coldim());

			//! @bug use copy
			typename BlasMatrix<Field, _Rep>::ConstIterator  Biter =   B.Begin();
			typename BlasMatrix<Field, _Rep>::Iterator       Xiter =   X.Begin();

			for (; Biter != B.End(); ++Biter,++Xiter)
				F.assign(*Xiter,*Biter);

			return (*this)(F, A, X);

		}

		BlasMatrix<Field, _Rep>& operator() (const Field& F,
						     const TriangularBlasMatrix<Field,_Rep>& A,
						     BlasMatrix<Field, _Rep>& B) const
		{
			linbox_check( A.rowdim() == A.coldim());
			linbox_check( A.coldim() == B.rowdim());

			FFLAS::ftrsm( F,
				     FFLAS::FflasLeft, (FFLAS::FFLAS_UPLO) A.getUpLo(),
				     FFLAS::FflasNoTrans,(FFLAS::FFLAS_DIAG) A.getDiag(),
				     A.rowdim(), B.coldim(),
				     F.one, A.getPointer(), A.getStride(),
				     B.getPointer(), B.getStride());

			return B;
		}
	};

	template <class Field, class _Rep>
	class BlasMatrixDomainRightSolve<Field, BlasMatrix<Field,_Rep>, TriangularBlasMatrix<Field,_Rep> > {
	public:
		BlasMatrix<Field, _Rep>& operator() (const Field& F,
						     BlasMatrix<Field, _Rep>& X,
						     const TriangularBlasMatrix<Field,_Rep>& A,
						     const BlasMatrix<Field, _Rep>& B) const
		{

			linbox_check( X.rowdim() == B.rowdim());
			linbox_check( X.coldim() == B.coldim());

			typename BlasMatrix<Field, _Rep>::ConstIterator  Biter =   B.Begin();
			typename BlasMatrix<Field, _Rep>::Iterator       Xiter =   X.Begin();

			for (; Biter != B.End(); ++Biter,++Xiter)
				F.assign(*Xiter,*Biter);

			return (*this)(F, A, X);
		}

		BlasMatrix<Field, _Rep>& operator() (const Field& F,
						     const TriangularBlasMatrix<Field,_Rep>& A,
						     BlasMatrix<Field, _Rep>& B) const
		{

			linbox_check( A.rowdim() == A.coldim());
			linbox_check( B.coldim() == A.rowdim());

			FFLAS::ftrsm( F,
				     FFLAS::FflasRight,(FFLAS::FFLAS_UPLO) A.getUpLo(),
				     FFLAS::FflasNoTrans,(FFLAS::FFLAS_DIAG) A.getDiag() ,
				     B.rowdim(), A.coldim(),
				     F.one, A.getPointer(), A.getStride(),
				     B.getPointer(), B.getStride());


			return B;
		}
	};

	template <class Matrix, class _Rep>
	class BlasMatrixDomainRightSolve<typename Matrix::Field, BlasSubmatrix<Matrix>, TriangularBlasMatrix<typename Matrix::Field, _Rep> > {
	public:
		typedef typename Matrix::Field Field;
		BlasSubmatrix<Matrix>& operator() (const typename Matrix::Field& F,
						   BlasSubmatrix<Matrix>& X,
						   const TriangularBlasMatrix<Field,_Rep>& A,
						   const BlasSubmatrix<Matrix>& B) const
		{

			linbox_check( X.rowdim() == B.rowdim());
			linbox_check( X.coldim() == B.coldim());

			typename BlasSubmatrix<Matrix>::ConstIterator  Biter =   B.Begin();
			typename BlasSubmatrix<Matrix>::Iterator       Xiter =   X.Begin();

			for (; Biter != B.End(); ++Biter,++Xiter)
				F.assign(*Xiter,*Biter);

			return (*this)(F, A, X);
		}

		BlasSubmatrix<Matrix>& operator() (const typename Matrix::Field& F,
						   const TriangularBlasMatrix<Field,_Rep>& A,
						   BlasSubmatrix<Matrix>& B) const
		{

			linbox_check( A.rowdim() == A.coldim());
			linbox_check( B.coldim() == A.rowdim());

			FFLAS::ftrsm( F,
				     FFLAS::FflasRight,(FFLAS::FFLAS_UPLO) A.getUpLo(),
				     FFLAS::FflasNoTrans,(FFLAS::FFLAS_DIAG) A.getDiag() ,
				     B.rowdim(), A.coldim(),
				     F.one, A.getPointer(), A.getStride(),
				     B.getPointer(), B.getStride());


			return B;
		}
	};


#if 1

	/*
	 * specialization for Operand of type std::vector<Element>
	 */

	template <class Field, class _Rep>
	class BlasMatrixDomainLeftSolve<Field, std::vector<typename Field::Element>, TriangularBlasMatrix<Field,_Rep> > {
	public:
		std::vector<typename Field::Element>& operator() (const Field& F,
								  std::vector<typename Field::Element>& x,
								  const TriangularBlasMatrix<Field,_Rep>& A,
								  const std::vector<typename Field::Element>& b) const
		{

			linbox_check (x.size() == b.size());
			typename std::vector<typename Field::Element>::const_iterator biter = b.begin();
			typename std::vector<typename Field::Element>::iterator       xiter = x.begin();
			for (;biter!=b.end();++biter,++xiter)
				F.assign(*xiter,*biter);

			return (*this)(F, A,x);
		}

		std::vector<typename Field::Element>& operator() (const Field& F,
								  const TriangularBlasMatrix<Field,_Rep>& A,
								  std::vector<typename Field::Element>& b) const
		{

			linbox_check( A.rowdim() == A.coldim());
			linbox_check( A.rowdim() == b.size());

			switch (A.getUpLo()) {
			case Tag::Shape::Upper:
			switch(A.getDiag()) {
			case Tag::Diag::Unit:
			FFLAS::ftrsv( F,
				     FFLAS::FflasUpper, FFLAS::FflasNoTrans, FFLAS::FflasUnit,
				     b.size(), A.getPointer(), A.getStride(),&b[0],1);
			break;
			case Tag::Diag::NonUnit:
			FFLAS::ftrsv( F,
				     FFLAS::FflasUpper, FFLAS::FflasNoTrans, FFLAS::FflasNonUnit,
				     b.size(), A.getPointer(), A.getStride(),&b[0],1);
			break;
			default:
			throw LinboxError ("Error in BlasMatrixDomain (triangular matrix not well defined)");
			}
			break;
			case Tag::Shape::Lower:
			switch(A.getDiag()) {
			case Tag::Diag::Unit:
			FFLAS::ftrsv( F,
				     FFLAS::FflasLower, FFLAS::FflasNoTrans, FFLAS::FflasUnit,
				     b.size(), A.getPointer(), A.getStride(),&b[0],1);
			break;
			case Tag::Diag::NonUnit:
			FFLAS::ftrsv( F,
				     FFLAS::FflasLower, FFLAS::FflasNoTrans, FFLAS::FflasNonUnit,
				     b.size(), A.getPointer(), A.getStride(),&b[0],1);
			break;
			default:
			throw LinboxError ("Error in BlasMatrixDomain (triangular matrix not well defined)");
			}
			break;
			default:
			throw LinboxError ("Error in BlasMatrixDomain (triangular matrix not well defined)");

			}
			return b;
		}
	};

	template <class Field, class _Rep>
	class BlasMatrixDomainRightSolve<Field, std::vector<typename Field::Element>, TriangularBlasMatrix<Field,_Rep> > {
	public:
		std::vector<typename Field::Element>& operator() (const Field& F,
								  std::vector<typename Field::Element>& x,
								  const TriangularBlasMatrix<Field,_Rep>& A,
								  const std::vector<typename Field::Element>& b) const
		{

			linbox_check (x.size() == b.size());
			typename std::vector<typename Field::Element>::const_iterator biter = b.begin();
			typename std::vector<typename Field::Element>::iterator       xiter = x.begin();
			for (;biter!=b.end();++biter,++xiter)
				F.assign(*xiter,*biter);

			return (*this)(F, A,x);
		}

		std::vector<typename Field::Element>& operator() (const Field& F,
								  const TriangularBlasMatrix<Field,_Rep>& A,
								  std::vector<typename Field::Element>& b) const
		{

			linbox_check( A.rowdim() == A.coldim());
			linbox_check( A.coldim() == b.size());


			switch (A.getUpLo()) {
			case Tag::Shape::Upper:
			switch(A.getDiag()) {
			case Tag::Diag::Unit:
			FFLAS::ftrsv( F,
				      FFLAS::FflasUpper, FFLAS::FflasTrans, FFLAS::FflasUnit,
				      b.size(), A.getPointer(), A.getStride(),&b[0],1);
			break;
			case Tag::Diag::NonUnit:
			FFLAS::ftrsv( F,
				     FFLAS::FflasUpper, FFLAS::FflasTrans, FFLAS::FflasNonUnit,
				     b.size(), A.getPointer(), A.getStride(),&b[0],1);
			break;
			default:
			throw LinboxError ("Error in BlasMatrixDomain (triangular matrix not well defined)");
			}
			break;
			case Tag::Shape::Lower:
			switch(A.getDiag()) {
			case Tag::Diag::Unit:
			FFLAS::ftrsv( F,
				     FFLAS::FflasLower, FFLAS::FflasTrans, FFLAS::FflasUnit,
				     b.size(), A.getPointer(), A.getStride(),&b[0],1);
			break;
			case Tag::Diag::NonUnit:
			FFLAS::ftrsv( F,
				      FFLAS::FflasLower, FFLAS::FflasTrans, FFLAS::FflasNonUnit,
				      b.size(), A.getPointer(), A.getStride(),&b[0],1);
			break;
			default:
			throw LinboxError ("Error in BlasMatrixDomain (triangular matrix not well defined)");
			}
			break;
			default:
			throw LinboxError ("Error in BlasMatrixDomain (triangular matrix not well defined)");

			}
			return b;
		}
	};

	template <class Field, class _Rep>
	class BlasMatrixDomainLeftSolve<Field, BlasVector<Field>, TriangularBlasMatrix<Field,_Rep> > {
	public:
		BlasVector<Field>& operator() (const Field& F,
					       BlasVector<Field>& x,
					       const TriangularBlasMatrix<Field,_Rep>& A,
					       const BlasVector<Field>& b) const
		{

			linbox_check (x.size() == b.size());
			typename BlasVector<Field>::const_iterator biter = b.begin();
			typename BlasVector<Field>::iterator       xiter = x.begin();
			for (;biter!=b.end();++biter,++xiter)
				F.assign(*xiter,*biter);

			return (*this)(F, A,x);
		}

		BlasVector<Field>& operator() (const Field& F,
					       const TriangularBlasMatrix<Field,_Rep>& A,
					       BlasVector<Field>& b) const
		{

			linbox_check( A.rowdim() == A.coldim());
			linbox_check( A.rowdim() == b.size());

			switch (A.getUpLo()) {
			case Tag::Shape::Upper:
			switch(A.getDiag()) {
			case Tag::Diag::Unit:
			FFLAS::ftrsv( F,
				     FFLAS::FflasUpper, FFLAS::FflasNoTrans, FFLAS::FflasUnit,
				     b.size(), A.getPointer(), A.getStride(),&b[0],1);
			break;
			case Tag::Diag::NonUnit:
			FFLAS::ftrsv( F,
				     FFLAS::FflasUpper, FFLAS::FflasNoTrans, FFLAS::FflasNonUnit,
				     b.size(), A.getPointer(), A.getStride(),&b[0],1);
			break;
			default:
			throw LinboxError ("Error in BlasMatrixDomain (triangular matrix not well defined)");
			}
			break;
			case Tag::Shape::Lower:
			switch(A.getDiag()) {
			case Tag::Diag::Unit:
			FFLAS::ftrsv( F,
				     FFLAS::FflasLower, FFLAS::FflasNoTrans, FFLAS::FflasUnit,
				     b.size(), A.getPointer(), A.getStride(),&b[0],1);
			break;
			case Tag::Diag::NonUnit:
			FFLAS::ftrsv( F,
				     FFLAS::FflasLower, FFLAS::FflasNoTrans, FFLAS::FflasNonUnit,
				     b.size(), A.getPointer(), A.getStride(),&b[0],1);
			break;
			default:
			throw LinboxError ("Error in BlasMatrixDomain (triangular matrix not well defined)");
			}
			break;
			default:
			throw LinboxError ("Error in BlasMatrixDomain (triangular matrix not well defined)");

			}
			return b;
		}
	};

	template <class Field, class _Rep>
	class BlasMatrixDomainRightSolve<Field, BlasVector<Field>, TriangularBlasMatrix<Field,_Rep> > {
	public:
		BlasVector<Field>& operator() (const Field& F,
					       BlasVector<Field>& x,
					       const TriangularBlasMatrix<Field,_Rep>& A,
					       const BlasVector<Field>& b) const
		{

			linbox_check (x.size() == b.size());
			typename BlasVector<Field>::const_iterator biter = b.begin();
			typename BlasVector<Field>::iterator       xiter = x.begin();
			for (;biter!=b.end();++biter,++xiter)
				F.assign(*xiter,*biter);

			return (*this)(F, A,x);
		}

		BlasVector<Field>& operator() (const Field& F,
					       const TriangularBlasMatrix<Field,_Rep>& A,
					       BlasVector<Field>& b) const
		{

			linbox_check( A.rowdim() == A.coldim());
			linbox_check( A.coldim() == b.size());


			switch (A.getUpLo()) {
			case Tag::Shape::Upper:
			switch(A.getDiag()) {
			case Tag::Diag::Unit:
			FFLAS::ftrsv( F,
				      FFLAS::FflasUpper, FFLAS::FflasTrans, FFLAS::FflasUnit,
				      b.size(), A.getPointer(), A.getStride(),&b[0],1);
			break;
			case Tag::Diag::NonUnit:
			FFLAS::ftrsv( F,
				     FFLAS::FflasUpper, FFLAS::FflasTrans, FFLAS::FflasNonUnit,
				     b.size(), A.getPointer(), A.getStride(),&b[0],1);
			break;
			default:
			throw LinboxError ("Error in BlasMatrixDomain (triangular matrix not well defined)");
			}
			break;
			case Tag::Shape::Lower:
			switch(A.getDiag()) {
			case Tag::Diag::Unit:
			FFLAS::ftrsv( F,
				     FFLAS::FflasLower, FFLAS::FflasTrans, FFLAS::FflasUnit,
				     b.size(), A.getPointer(), A.getStride(),&b[0],1);
			break;
			case Tag::Diag::NonUnit:
			FFLAS::ftrsv( F,
				      FFLAS::FflasLower, FFLAS::FflasTrans, FFLAS::FflasNonUnit,
				      b.size(), A.getPointer(), A.getStride(),&b[0],1);
			break;
			default:
			throw LinboxError ("Error in BlasMatrixDomain (triangular matrix not well defined)");
			}
			break;
			default:
			throw LinboxError ("Error in BlasMatrixDomain (triangular matrix not well defined)");

			}
			return b;
		}
	};

#endif

} // LinBox

namespace LinBox
{ /* Minpoly Charpoly */

	template< class Field, class Polynomial, class Matrix>
	Polynomial&
	BlasMatrixDomainMinpoly<Field,Polynomial,Matrix>::operator() (const Field &F, Polynomial& P, const Matrix& A) const
	{
		commentator().start ("Givaro::Modular Dense Minpoly ", "MDMinpoly");

		size_t n = A.coldim();
		linbox_check( n == A.rowdim());
		typedef typename Matrix::constSubMatrixType constSubMatrixType ;
		constSubMatrixType A_v(A);
		FFPACK::MinPoly<Field, Polynomial>(  F, P, n, A_v.getPointer(), A_v.getStride());
		commentator().report(Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION) << "minpoly with " << P.size() << " coefficients" << std::endl;

		commentator().stop ("done", NULL, "MDMinpoly");
		return P;
	}

	template<class Field, class Polynomial, class Matrix>
	Polynomial &
	BlasMatrixDomainCharpoly<Field,Polynomial,Matrix>::operator() ( 
        const Field    &F, Polynomial    &P, const Matrix   &A) const
	{
		size_t n = A.coldim();
		P.clear();
		linbox_check( n == A.rowdim());
		typedef typename Matrix::constSubMatrixType constSubMatrixType ;
		constSubMatrixType A_v(A);

        typename Field::RandIter G(F);
        typename Givaro::Poly1Dom<Field> PolDom(F);
        FFPACK::CharPoly (PolDom, P, n, A_v.getPointer(), A_v.getStride(), G);
		return P;
	}

} //end of namespace LinBox

#endif // __LINBOX_matrix_matrixdomain_blas_matrix_domain_INL


// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=
