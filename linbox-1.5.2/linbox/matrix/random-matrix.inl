/* Copyright (C) 2011 LinBox
 * Written by Brice Boyer (briceboyer) <boyer.brice@gmail.com>
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

#ifndef __LINBOX_matrix_random_matrix_INL
#define __LINBOX_matrix_random_matrix_INL
/** @file matrix/random-matrix.inl
 * @ingroup matrix
 * @brief Implementation of random matrices.
 *
 */

namespace LinBox
{

	//! This is the namespace all LinBox internal code is in.
	namespace Protected {

		template<class Randiter,class Field>
		BlasMatrix<Field> &
		random_lu_rank(const Field			& F,
			       const Randiter                   & R,
			       BlasMatrix<Field>              & A,
			       int                              & rank,
			       const RingCategories::ModularTag & tag)
		{
			size_t m = A.rowdim() ;
			size_t n = A.coldim() ;

			linbox_check(m != 0);
			linbox_check(n != 0);
			if (rank == -1)
				rank = (int) ( (double)std::min(m,n)*drand48() );
			linbox_check(!(rank<0 || rank>(int)std::min(m,n)));

			// typedef typename Field::Element Element ;

			// be ready for PLUQ
			//size_t  * P = new size_t [m]   ;
			BlasPermutation<size_t> P(m);
			//Element * L = new Element[m*m] ;
			//TriangularBlasMatrix<Field> L(F,m,m,Tag::Lower,Tag::Unit);
			//! @todo !!!
			BlasMatrix<Field> L(F,m,m);
			// Element * U = new Element[m*n] ;
			// TriangularBlasMatrix<Field> U(F,m,n,Tag::Upper,Tag::NonUnit);
			//
			BlasMatrix<Field> U(F,m,n);
			//size_t  * Q = new size_t [n]   ;
			BlasPermutation<size_t> Q(n);

			// be ready for random elements
			Givaro::GeneralRingNonZeroRandIter<Field> Rnz(R);

			/* Create L a random invertible lower unit triangular matrix (m x m format) */
			for (size_t j=0 ; j<m ; ++j)
				for (size_t i=j+1; i<m;++i)
					R.random( L.refEntry( i,j ) );
#if 1
			for (size_t i = 0; i < (size_t) m; ++i)
				Rnz.random( L.refEntry( i,i ) ); // non zero diagonal
			for (size_t j=0 ; j<m ; ++j)
				for (size_t i=0; i<j;++i)
					L.setEntry( i,j,F.zero );


#endif

			/* Create U a random rank-r upper non-unit triangular matrix (m x n format) */
			for (size_t i = 0 ; i < (size_t) rank; ++i)
				for (size_t j = i+1; j < n; ++j)
					R.random( U.refEntry( i,j ) ); // r random 'triangular' lines
			for (size_t i = 0; i < (size_t) rank; ++i)
				Rnz.random( U.refEntry( i,i ) ); // non zero diagonal on rank first lines
#if 1
			for (size_t i = (size_t)rank ; i < m ; ++i)
				for (size_t j = i ; j < n ; ++j)
					U.setEntry( i,j,F.zero ) ; //  zero on remaining 'triangular' lines
#endif

			/**
			 * @todo RandomPermutation avec P de type [Matrix-Blas]Permutation
			 */
			RandomBlasPermutation(P);
			RandomBlasPermutation(Q);

			BlasMatrixDomain< Field > BMD(F) ;
			// LU
			/**
			 * @todo : L = [[L1,0],[A,L2]] ;U = [[U1,B],[0,U2]] ; LU = [[ rec(L1,U1), ftrmm(L1,B)],[ftrmm(A,U1),fgemm(A,B)+rec(L2,U2) ]]
			 * de même UL
			 */

			// BlasMatrix<Field> A_ptr(A) ;

			BMD.mul(A,L,U);

			/*!
			 * @todo create BMD.applyP(A,P,Tag::Left) ;
			 * avec P : BlasPermutation
			 * ou P : MatrixPermutation
			 * @todo BlasPermutation a un ordre \p p et une taille \p r distinctes !!!
			 */

			BMD.mulin_left(A,Q);
			BMD.mulin_right(P,A);

			return A ;

		}


		//!@todo ZZ is A.field() !
		template<class Randiter, class Ring>
		BlasMatrix<Ring> &
		random_lu_rank(const Ring          & ZZ,
			       const Randiter      & R,
			       BlasMatrix<Ring>  &A,
			       int                 & rank,
			       const RingCategories::IntegerTag & tag)
		{

			// typedef typename Ring::Element Int ;
			size_t m = A.rowdim() ;
			size_t n = A.coldim() ;

			linbox_check(m != 0);
			linbox_check(n != 0);
			if (rank == -1)
				rank = (int) ( (double)std::min(m,n)*drand48() );
			linbox_check(!(rank<0 || rank>(int)std::min(m,n)));


			// be ready for PLUQ
			BlasPermutation<size_t> P(m);
			BlasMatrix<Ring> L(ZZ,m,m);
			BlasMatrix<Ring> U(ZZ,m,n);
			BlasPermutation<size_t> Q(n);

			// be ready for random elements
			Randiter S_(R);
			S_.setBits(R.getBits()-1);
			typename Ring::Randiter T_(ZZ,3);
			Givaro::GeneralRingNonZeroRandIter<Ring > U_(T_);


			/* Create L a random invertible lower unit triangular matrix (m x m format) */
			for (size_t j=0 ; j<m ; ++j)
				for (size_t i=j+1; i<m;++i)
					S_.random( L.refEntry( i,j ) );
#if 1
			for (size_t i = 0; i < (size_t) m; ++i)
				U_.random( L.refEntry( i,i ) ); // non zero diagonal
			for (size_t j=0 ; j<m ; ++j)
				for (size_t i=0; i<j;++i)
					L.setEntry( i,j,ZZ.zero );


#endif

			/* Create U a random rank-r upper non-unit triangular matrix (m x n format) */
			for (size_t i = 0 ; i < (size_t) rank; ++i)
				for (size_t j = i+1; j < n; ++j)
					T_.random( U.refEntry( i,j ) ); // r random 'triangular' lines
			for (size_t i = 0; i < (size_t) rank; ++i)
				U_.random( U.refEntry( i,i ) ); // non zero diagonal on rank first lines
#if 1
			for (size_t i = (size_t)rank ; i < m ; ++i)
				for (size_t j = i ; j < n ; ++j)
					U.setEntry( i,j,ZZ.zero ) ; //  zero on remaining 'triangular' lines
#endif

			RandomBlasPermutation(P);
			RandomBlasPermutation(Q);

			BlasMatrixDomain< Ring > BMD(ZZ) ;
			MatrixDomain< Ring > MD(ZZ) ;
			// LU
			// L.write(std::cout << "L:=",true ) << ';' << std::endl;
			// L.write(std::cout << "U:=",true ) << ';' << std::endl;

			MD.mul(A,L,U);
			// A.write(std::cout << "pre A=",true) << std::endl;

			BMD.mulin_left(A,Q);
			BMD.mulin_right(P,A);

			// P.write(std::cout<<"P:=",false) << std::endl;
			// P.write(std::cout<<"Q:=",false) << std::endl;
			// P.write(std::cout<<"Q:=",true) << std::endl;

			return A ;
		}

#if 0
		template<class Randiter,class Field>
		BlasMatrix<Field> &
		random_lu_rank(const Field			   & F,
			       const Randiter                      & R,
			       BlasMatrix<Field> & A,
			       int                                 & rank,
			       const RingCategories::IntegerTag    & tag)
		{
			BlasBlackbox<Field> Alink(F,A);
			random_lu_rank(F,R,Alink,rank,RingCategories::IntegerTag());
			return A ;
		}

#endif
#if 0 /*  BlasMatrix spec. */
		template<class Randiter, class Field>
		BlasBlackbox<Field> &
		random_rankupdate( const Field        & F
				   ,const Randiter    & R
				   ,BlasBlackbox<Field>& A
				   , int              & rank
				 )
		{
			size_t m = A.rowdim();
			size_t n = A.coldim();


			BlasBlackbox<Field> D(F,m,rank) ;
			BlasBlackbox<Field> G(F,rank,n) ;
			RandomBlasBlackbox<Randiter, Field> RandMatGen(F,R);
			RandMatGen.random(D) ;
			RandMatGen.random(G) ;
			MatrixDomain<Field> MD(F);
			MD.mul(A,D,G);
			return A ;
		}

		template<class Randiter>
		BlasBlackbox<Givaro::ZRing<Integer> > &
		random_rankupdate( Givaro::ZRing<Integer>               & F //!@bug const !
				   ,const Randiter           & R
				   ,BlasBlackbox<Givaro::ZRing<Integer> > & A
				   , int                     & rank
				 )
		{
			//! @todo check randomness
			size_t m = A.rowdim();
			size_t n = A.coldim();

			BlasBlackbox<Givaro::ZRing<Integer> > D(F,(size_t)m,(size_t)rank) ;
			BlasBlackbox<Givaro::ZRing<Integer> > G(F,(size_t)rank,(size_t)n) ;
			Randiter S_(R);
			S_.setBits(R.getBits()-1);
			RandomBlasBlackbox<Randiter,Givaro::ZRing<Integer> > RandMatGen(F,S_);
			RandMatGen.random(D) ;
			Givaro::ZRing<Integer> >::RandIter T_(F,3);
			RandomBlasBlackbox<Givaro::ZRing<Integer> >::RandIter,Givaro::ZRing<Integer> > RandSmallMatGen(F,T_);
			RandMatGen.random(G) ;
			MatrixDomain<Givaro::ZRing<Integer> > MD(F);
			MD.mul(A,D,G);
			return A ;
		}
#endif

		template<class Randiter,class Field>
		BlasMatrix<Field> &
		random_rankupdate( Field                & F //!@bug const !
				   ,const Randiter      & R
				   ,BlasMatrix<Field> & A
				   , int                & rank
				   , const RingCategories::IntegerTag          &tag
				 )
		{
			size_t m = A.rowdim();
			size_t n = A.coldim();

			BlasMatrix<Field> D(F,(size_t)m,(size_t)rank) ;
			BlasMatrix<Field> G(F,(size_t)rank,(size_t)n) ;
			Randiter S_(R);
			S_.setBits(R.getBits()-1);
			RandomDenseMatrix<Randiter,Field > RandMatGen(F,S_);
			RandMatGen.random(D) ;
			typename Field::Randiter T_(F,3);
			RandomDenseMatrix<typename Field::Randiter, Field> RandSmallMatGen(F,T_);
			RandMatGen.random(G) ;

			MatrixDomain<Field> MD(F);
			MD.mul(A,D,G);

			/// @bug do perms ?

#if 0 /*  necessary */
			BlasPermutation<size_t>  P(m);
			BlasPermutation<size_t>  Q(n);

			RandomBlasPermutation(P);
			RandomBlasPermutation(Q);

			Givaro::ZRing<Integer> ZZ ;
			BlasMatrixDomain< Givaro::ZRing<Integer> > BMD(ZZ) ;

			BMD.mulin_left (A,Q);
			BMD.mulin_right(P,A);
#endif

			return A ;
		}
	} // Protected

	//////// Random Matrix
	template<class Randiter, class Field>
	template<class Matrix>
	Matrix &
	RandomDenseMatrix<Randiter, Field>::random( Matrix & A)
	{
		typename Field::Element d;
		for (size_t i = 0 ; i < A.rowdim() ; ++i)
			for (size_t j = 0 ; j < A.coldim() ; ++j) {
				R_.random(d);
				A.setEntry(i,j,d);
			}
		return A;
	}

	template<class Randiter, class Field>
	template<class Matrix>
	Matrix&
	RandomDenseMatrix<Randiter, Field>::randomFullRank(Matrix &A)
	{
		long unsigned r,b,m,n;
		m=A.rowdim();
		n=A.coldim();
		b=m<n?m:n;
		do {
			random(A);
		} while(LinBox::rank(r,A,Method::Hybrid()) < b);
		return A;
	}

	//////// Random Matrix with prescribed Rank
#if 0
	template<class Randiter, class Field>
	template<class Matrix,class Method>
	Matrix &
	RandomDenseMatrix<Randiter, Field>::randomRank(Matrix & A,
						       int    rank
						       , const Method & meth )
	{
		throw NotImplementedYet(LB_FILE_LOC);
	}
#endif


	template<class Randiter, class Field>
	template<class Matrix>
	Matrix &
	RandomDenseMatrix<Randiter, Field>::randomRank(
						       Matrix & A
						       , int  rank
						      )
	{
		//! @todo use CatergoryTag
		return randomRank(A,rank,RankBuilder::LU_());
	}

	template<class Randiter, class Field>
	template< class Matrix>
	Matrix &
	RandomDenseMatrix<Randiter, Field>::randomRank(
						       Matrix  & A
						       , int   rank
						       , const RankBuilder::LU_ & meth
						      )
	{
		Protected::random_lu_rank( F_,R_,A,rank,
				typename FieldTraits<Field>::categoryTag());
		return A ;
	}



	template<class Randiter, class Field>
	template<class Matrix>
	Matrix &
	RandomDenseMatrix<Randiter, Field>::randomRank(
						       Matrix    & A
						       , int     rank
						       , const RankBuilder::Rank_update_ & meth
						      )
	{
		return Protected::random_rankupdate(F_,R_,A,rank,
						    typename FieldTraits<Field>::categoryTag());
	}

	/*  dense matrix with random entries */

	void RandomBlasPermutation(BlasPermutation<size_t> & P)
	{
		size_t * Pt = P.getWritePointer();
		// size_t n = P.getSize();
		size_t r = P.getOrder();
		size_t n = r ; // no size given ?
		for (size_t i = 0 ; i < r ; ++i) {
			Pt[i] = i + size_t(double (n-i)*( drand48() ) ) ;
		}
		return ;
	}


} // LinBox

#endif // __LINBOX_matrix_random_matrix_INL



// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

