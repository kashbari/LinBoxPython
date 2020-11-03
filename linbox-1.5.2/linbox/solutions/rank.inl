/* linbox/solutions/rank.inl
 * Copyright(C) LinBox
 * ------------------------------------
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

#ifndef __LINBOX_rank_INL
#define __LINBOX_rank_INL

#define __LINBOX_rank_sparse_elimination_format SparseMatrixFormat::SparseSeq
// #define __LINBOX_rank_sparse_elimination_format SparseMatrixFormat::SparseMar
// #define __LINBOX_rank_sparse_elimination_format SparseMatrixFormat::SparsePar
// #define __LINBOX_rank_sparse_elimination_format SparseMatrixFormat::COO
// #define __LINBOX_rank_sparse_elimination_format SparseMatrixFormat::CSR

#include "linbox/field/field-traits.h"

#include <givaro/extension.h>

// Namespace in which all LinBox library code resides
namespace LinBox
{


	template <class Blackbox>
	inline unsigned long &rank (unsigned long                    &r,
				    const Blackbox                   &A,
				    const RingCategories::ModularTag &tag,
				    const Method::Hybrid             &m)
	{ // this should become a BB/Blas hybrid in the style of Duran/Saunders/Wan.
		//! @bug choose (benchmark) better cuttoff (size, nbnz, sparse rep)
		if (useBB(A)) {
			return rank(r, A, tag, Method::Blackbox(m ));
		}
		else {
			return rank(r, A, tag, Method::Elimination( m ));
		}
	}

	template <class Blackbox>
	inline unsigned long &rank (unsigned long                     &r,
				    const Blackbox                    &A,
				    const RingCategories::ModularTag  &tag,
				    const Method::Elimination         &m)
	{
		typedef typename Blackbox::Field Field;
		const Field& F = A.field();
		integer a, b; F.characteristic(a); F.cardinality(b);
		if (a == b && a < LinBox::BlasBound)
			return rank(r, A, tag, Method::BlasElimination(m));
		else
			return rank(r, A, tag, Method::NonBlasElimination( m ));
	}

	template <class Field, class Vector>
	inline unsigned long &rank (unsigned long                      &r,
				    const SparseMatrix<Field, Vector>  &A,
				    const RingCategories::ModularTag   &tag,
				    const Method::Elimination          &m)
	{
		return rank(r, A, tag, Method::SparseElimination(m));
	}


	// specialization of NonBlas for SparseMatrix
	template <class Blackbox>
	inline unsigned long &rank (unsigned long                       &r,
				    const Blackbox                      &A,
				    const   RingCategories::ModularTag  &tag,
				    const Method::NonBlasElimination    & m)
	{
		return rank(r, A, tag, Method::SparseElimination(m));
	}

	// specialization of NonBlas for SparseMatrix
	template <class Blackbox>
	inline unsigned long &rankin (unsigned long                       &r,
				    Blackbox                      &A,
				    const   RingCategories::ModularTag  &tag,
				    const Method::NonBlasElimination    & m)
	{
		return rankin(r, A, tag, Method::SparseElimination(m));
	}


	template <class Blackbox>
	inline unsigned long &rank (unsigned long                     &r,
				    const Blackbox                    &A,
				    const  RingCategories::ModularTag &tag,
				    const Method::Blackbox            &m);



	/// M may be <code>Method::Wiedemann()</code>.
	template <class Blackbox>
	inline unsigned long &rank (unsigned long                     &res,
				    const Blackbox                    &A,
				    const RingCategories::ModularTag  &tag,
				    const Method::Wiedemann           &M)
	//! @bug This is too much for solutions.  It belongs in algorithms
	{

		typedef typename Blackbox::Field Field;
		const Field F = A.field();
		typename Field::RandIter iter (F);

		if (M.symmetric()) {
			commentator().start ("Symmetric Rank", "srank");


			BlasVector<Field> d1(F);
			size_t i;

			VectorWrapper::ensureDim (d1, A.coldim ());

			for (i = 0; i < A.coldim (); i++)
				do iter.random (d1[i]); while (F.isZero (d1[i]));


			typedef Compose<Compose<Diagonal<Field>,Blackbox >, Diagonal<Field> > BlackBox1;
			Diagonal<Field> D_0 (d1);
			Compose<Diagonal<Field>,Blackbox > B_0 (&D_0, &A);
			BlackBox1 B (&B_0, &D_0);

			BlackboxContainerSymmetric<Field, BlackBox1> TF (&B, F, iter);
			MasseyDomain<Field, BlackboxContainerSymmetric<Field, BlackBox1> > WD (&TF, M.earlyTermThreshold ());
			BlasVector<Field> phi(F);
			WD.pseudo_minpoly (phi, res);
			commentator().report(Commentator::LEVEL_ALWAYS,INTERNAL_DESCRIPTION) << "Pseudo Minpoly degree: " << res << std::endl;

			commentator().start ("Monte Carlo certification (1)", "trace");
			typename Field::Element t, p2; F.assign(p2, F.zero);
			trace(t, B);
			if (phi.size() >= 2) F.neg(p2, phi[ phi.size()-2]);

			int nbperm = 0; unsigned long rk;
			int logn = (int)(2*(unsigned long)floor( log( (double)A.rowdim() ) ));
			bool tryagain = (! F.areEqual( t, p2 ));
			while( tryagain ) {
				commentator().stop ("fail", NULL, "trace");
#if 0

				Permutation<Field> P(F,A.rowdim());
				for (i = 0; i < A.rowdim (); ++i)
					P.permute( rand() % A.rowdim() , rand() % A.rowdim() );
				for (i = 0; i < A.rowdim (); ++i)
					P.permute( rand() % A.rowdim() , rand() % A.rowdim() );

				Transpose< Permutation<Field> > TP(&P);
				typedef Compose< Permutation<Field>, Blackbox > BlackboxP;
				typedef Compose< Compose< Permutation<Field>, Blackbox >, Transpose< Permutation<Field> > > BlackboxPAP;
				BlackboxP PA(&P, &A);
				BlackboxPAP BP( &PA , &TP );

				for (i = 0; i < A.coldim (); i++)
					do iter.random (d1[i]); while (F.isZero (d1[i]));
				Diagonal<Field> D1 (F, d1);
				Compose<Diagonal<Field>,BlackboxPAP > B1 (&D1, &BP);
				typedef Compose<Compose<Diagonal<Field>,BlackboxPAP >, Diagonal<Field> > BlackBox2;
				BlackBox2 B (&B1, &D1);
#endif

				for (i = 0; i < A.coldim (); i++)
					do iter.random (d1[i]); while (F.isZero (d1[i]));
				Diagonal<Field> D1 (d1);
				Compose<Diagonal<Field>,Blackbox > B1 (&D1, &A);
				BlackBox1 B2 (&B1, &D1);

				BlackboxContainerSymmetric<Field, BlackBox1> TF1 (&B2, F, iter);
				MasseyDomain<Field, BlackboxContainerSymmetric<Field, BlackBox1> > WD1 (&TF1, M.earlyTermThreshold ());

				WD1.pseudo_minpoly (phi, rk);
				commentator().report(Commentator::LEVEL_ALWAYS,INTERNAL_DESCRIPTION) << "Permuted pseudo Minpoly degree: " << res << std::endl;
				commentator().start ("Monte Carlo certification (2)", "trace");
				if (phi.size() >= 2) F.neg(p2, phi[ phi.size()-2]);

				trace(t, B2);

				tryagain = (! F.areEqual( t, p2 ));
				if (res > rk)
					tryagain = true;
				else
					res = rk;
				if( ++nbperm > logn) break;
			}
			commentator().report(Commentator::LEVEL_ALWAYS,INTERNAL_DESCRIPTION) << "symm permutations : " << nbperm << std::endl;
			nbperm = 0;
			while(tryagain) {
				commentator().stop ("fail", NULL, "trace");
				//             F.write( F.write( commentator().report(Commentator::LEVEL_ALWAYS,INTERNAL_DESCRIPTION)
				//                               << "end trace: ", t) << ", p2: ", p2) << std::endl;
				typename Field::RandIter r (F);
				typename CekstvSwitch<Field>::Factory factory (r);
				typedef Butterfly<Field, CekstvSwitch<Field> > ButterflyP;
				ButterflyP P (F, A.rowdim(), factory);
				for (i = 0; i < A.coldim (); i++)
					do iter.random (d1[i]); while (F.isZero (d1[i]));
				Diagonal<Field> D1 (d1);
				typedef Compose< ButterflyP, Diagonal<Field> > ButD;
				ButD PD(&P, &D1);

				Transpose< ButD > TP (&PD);

				Compose< ButD, Blackbox > B1( &PD, &A);

				typedef Compose< Compose< ButD, Blackbox > , Transpose< ButD > > BlackBoxBAB;
				BlackBoxBAB PAP(&B1, &TP);

				BlackboxContainerSymmetric<Field, BlackBoxBAB> TF1 (&PAP, F, iter);
				MasseyDomain<Field, BlackboxContainerSymmetric<Field, BlackBoxBAB> > WD1 (&TF1, M.earlyTermThreshold ());

				WD1.pseudo_minpoly (phi, rk);
				commentator().report(Commentator::LEVEL_ALWAYS,INTERNAL_DESCRIPTION) << "Butterfly pseudo Minpoly degree: " << res << std::endl;
				commentator().start ("Monte Carlo certification (3)", "trace");
				if (phi.size() >= 2) F.neg(p2, phi[ phi.size()-2]);

				trace(t, PAP);

				tryagain = (! F.areEqual( t, p2 ));
				if (res > rk)
					tryagain = true;
				else
					res = rk;
				++nbperm;
			}

			// F.write( F.write( commentator().report(Commentator::LEVEL_ALWAYS,INTERNAL_DESCRIPTION)
			//              << "end trace: ", t) << ", p2: ", p2) << std::endl;

			commentator().report(Commentator::LEVEL_ALWAYS,INTERNAL_DESCRIPTION) << "butterflies : " << nbperm << std::endl;

			commentator().stop ("success", NULL, "trace");
			commentator().stop ("done", NULL, "srank");

			return res;
		}
		else {

			commentator().start ("Rank", "wrank");

			BlasVector<Field> d1(F), d2(F);
			size_t i;

			VectorWrapper::ensureDim (d1, A.coldim ());
			VectorWrapper::ensureDim (d2, A.rowdim ());

			for (i = 0; i < A.coldim (); i++)
				do iter.random (d1[i]); while (F.isZero (d1[i]));

			for (i = 0; i < A.rowdim (); i++)
				do iter.random (d2[i]); while (F.isZero (d2[i]));

			Diagonal<Field> D1_i (d1), D2_i (d2);
			Transpose<Blackbox> AT_i (&A);

			Compose<Diagonal<Field>,Transpose<Blackbox> > B1_i (&D1_i, &AT_i);
			Compose<Compose<Diagonal<Field>,Transpose<Blackbox> >, Diagonal<Field> > B2_i (&B1_i, &D2_i);
			Compose<Compose<Compose<Diagonal<Field>,Transpose<Blackbox> >, Diagonal<Field> >, Blackbox> B3_i (&B2_i, &A);
			// Here there is an extra diagonal computation
			// The probability of success is also divided by two, as
			// D2_i^2 contains only squares and squares are half the total elements
			typedef Compose<Compose<Compose<Compose<Diagonal<Field>,Transpose<Blackbox> >, Diagonal<Field> >, Blackbox>, Diagonal<Field> > Blackbox0;
			Blackbox0 B_i (&B3_i, &D1_i);

			BlackboxContainerSymmetric<Field, Blackbox0> TF_i (&B_i, F, iter);
			MasseyDomain<Field, BlackboxContainerSymmetric<Field, Blackbox0> > WD (&TF_i, M.earlyTermThreshold ());

			BlasVector<Field> phi(F);
			WD.pseudo_minpoly (phi, res);
			commentator().report(Commentator::LEVEL_ALWAYS,INTERNAL_DESCRIPTION) << "Pseudo Minpoly degree: " << res << std::endl;
			commentator().start ("Monte Carlo certification (4)", "trace");

			typename Field::Element t, p2; F.assign(p2, F.zero);
			//             trace(t, B_i);
			WhisartTraceTranspose(t, F, D1_i, A, D2_i);
			if (phi.size() >= 2) F.neg(p2, phi[ phi.size()-2]);

			int nbperm = 0; unsigned long rk;
			int logn = (int)(2*(unsigned long)floor( log( (double)A.rowdim() ) ));
			bool tryagain = (! F.areEqual( t, p2 ));
			while( tryagain ) {
				commentator().stop ("fail", NULL, "trace");
				Permutation<Field> P(F,(int)A.rowdim());
				for (i = 0; i < A.rowdim (); ++i)
					P.permute( (size_t)rand() % A.rowdim() , (size_t)rand() % A.rowdim() );
				for (i = 0; i < A.rowdim (); ++i)
					P.permute( (size_t)rand() % A.rowdim() , (size_t)rand() % A.rowdim() );

				typedef Compose< Permutation<Field>, Blackbox > BlackboxP;
				BlackboxP BP(&P, &A);

				for (i = 0; i < A.coldim (); i++)
					do iter.random (d1[i]); while (F.isZero (d1[i]));

				for (i = 0; i < A.rowdim (); i++)
					do iter.random (d2[i]); while (F.isZero (d2[i]));

				Diagonal<Field> D1 (d1), D2 (d2);
				Transpose<BlackboxP> AT (&BP);

				Compose<Diagonal<Field>,Transpose<BlackboxP> > B1 (&D1, &AT);
				Compose<Compose<Diagonal<Field>,Transpose<BlackboxP> >, Diagonal<Field> > B2 (&B1, &D2);
				Compose<Compose<Compose<Diagonal<Field>,Transpose<BlackboxP> >, Diagonal<Field> >, BlackboxP> B3 (&B2, &BP);
				typedef Compose<Compose<Compose<Compose<Diagonal<Field>,Transpose<BlackboxP> >, Diagonal<Field> >, BlackboxP>, Diagonal<Field> > Blackbox1;
				Blackbox1 B (&B3, &D1);

				BlackboxContainerSymmetric<Field, Blackbox1> TF (&B, F, iter);
				MasseyDomain<Field, BlackboxContainerSymmetric<Field, Blackbox1> > MD (&TF, M.earlyTermThreshold ());

				MD.pseudo_minpoly (phi, rk);
				commentator().report(Commentator::LEVEL_ALWAYS,INTERNAL_DESCRIPTION) << "Permuted pseudo Minpoly degree: " << rk << std::endl;
				commentator().start ("Monte Carlo certification (5)", "trace");
				if (phi.size() >= 2) F.neg(p2, phi[ phi.size()-2]);

				//                 trace(t, B);
				WhisartTraceTranspose(t, F, D1, BP, D2);
				tryagain = (! F.areEqual( t, p2 ));
				if (res > rk)
					tryagain = true;
				else
					res = rk;
				if( ++nbperm > logn) break;
			}
			commentator().report(Commentator::LEVEL_ALWAYS,INTERNAL_DESCRIPTION) << "permutations : " << nbperm << std::endl;
			nbperm = 0;
			while(tryagain) {
				commentator().stop ("fail", NULL, "trace");
				typename Field::RandIter r (F);
				typename CekstvSwitch<Field>::Factory factory (r);
				typedef Butterfly<Field, CekstvSwitch<Field> > ButterflyP;
				ButterflyP P (F, A.rowdim(), factory);

				typedef Compose< ButterflyP, Blackbox > BlackboxP;
				BlackboxP BP(&P, &A);

				for (i = 0; i < A.coldim (); i++)
					do iter.random (d1[i]); while (F.isZero (d1[i]));

				for (i = 0; i < A.rowdim (); i++)
					do iter.random (d2[i]); while (F.isZero (d2[i]));

				Diagonal<Field> D1 (d1), D2 (d2);
				Transpose<BlackboxP> AT (&BP);

				Compose<Diagonal<Field>,Transpose<BlackboxP> > B1 (&D1, &AT);
				Compose<Compose<Diagonal<Field>,Transpose<BlackboxP> >, Diagonal<Field> > B2 (&B1, &D2);
				Compose<Compose<Compose<Diagonal<Field>,Transpose<BlackboxP> >, Diagonal<Field> >, BlackboxP> B3 (&B2, &BP);
				typedef Compose<Compose<Compose<Compose<Diagonal<Field>,Transpose<BlackboxP> >, Diagonal<Field> >, BlackboxP>, Diagonal<Field> > Blackbox1;
				Blackbox1 B (&B3, &D1);

				BlackboxContainerSymmetric<Field, Blackbox1> TF (&B, F, iter);
				MasseyDomain<Field, BlackboxContainerSymmetric<Field, Blackbox1> > MD (&TF, M.earlyTermThreshold ());

				MD.pseudo_minpoly (phi, rk);
				commentator().report(Commentator::LEVEL_ALWAYS,INTERNAL_DESCRIPTION) << "Butterfly pseudo Minpoly degree: " << rk << std::endl;
				commentator().start ("Monte Carlo certification (6)", "trace");
				if (phi.size() >= 2) F.neg(p2, phi[ phi.size()-2]);

				//                 trace(t, B);
				WhisartTraceTranspose(t, F, D1, BP, D2);
				// std::cout << t << ',' << p2 << std::endl;
				tryagain = (! F.areEqual( t, p2 ));
				if (res > rk)
					tryagain = true;
				else
					res = rk;
				++nbperm;
			}
			commentator().report(Commentator::LEVEL_ALWAYS,INTERNAL_DESCRIPTION) << "butterflies : " << nbperm << std::endl;
			commentator().stop ("success", NULL, "trace");
			commentator().stop ("done", NULL, "wrank");

			return res;
		}

	}

	template <class Field>
	inline unsigned long &rankin (
        unsigned long       &r,
        SparseMatrix<Field, SparseMatrixFormat::SparseSeq>  &A,
        const RingCategories::ModularTag  &tag,
        const Method::Elimination         &m)
	{
        return rankin(r, A, tag, Method::SparseElimination( m ));
	}


	/// M may be <code>Method::SparseElimination()</code>.
	template <class Field>
	inline unsigned long &rank (unsigned long                       &r,
				    const SparseMatrix<Field, SparseMatrixFormat::SparseSeq>  &A,
				    const RingCategories::ModularTag    &tag,
				    const Method::SparseElimination     &M)
	{
		// We make a copy as these data will be destroyed
		SparseMatrix<Field, SparseMatrixFormat::SparseSeq> A1 (A);
		return rankin(r, A1, tag, M);
	}

	// Change of representation to be able to call the sparse elimination
	template <class Blackbox, class DomainCategory>
	inline unsigned long &rank (unsigned long                       &r,
				    const Blackbox                      &A,
				    const DomainCategory		&tag,
				    const Method::SparseElimination     &M)
	{
//         typename GaussDomain<typename Blackbox::Field>::Matrix copyA(A);
        typename GaussDomain<typename Blackbox::Field>::Matrix copyA(A.field(),A.rowdim(), A.coldim());
        MatrixHom::map(copyA, A);
		return rankin(r, copyA, tag, M);
	}

	// M may be <code>Method::BlasElimination()</code>.
	template <class Blackbox>
	inline unsigned long &rank (unsigned long                      &r,
				    const Blackbox                     &A,
				    const RingCategories::ModularTag   &tag,
				    const Method::BlasElimination      &M)
	{

		commentator().start ("Blas Rank", "blasrank");
		typedef typename Blackbox::Field Field;
		const Field F = A.field();
		integer a, b; F.characteristic(a); F.cardinality(b);
		linbox_check( a == b );
		linbox_check( a < LinBox::BlasBound);
		BlasMatrix<Field> B(A);
		BlasMatrixDomain<Field> D(F);
		r = D.rankin(B);
		commentator().stop ("done", NULL, "blasrank");
		return r;
	}


	template <class Blackbox, class MyMethod>
	inline unsigned long &integral_rank (unsigned long	&r,
                                         const Blackbox	&A,
                                         const MyMethod	&M)
	{
		commentator().start ("Integer Rank", "iirank");
		typedef Givaro::Modular<double> projField;
		integer mmodulus;
		FieldTraits<projField>::maxModulus(mmodulus);
		RandomPrimeIterator genprime( (unsigned) floor (log((double)mmodulus) ) );
		++genprime;
		typedef typename Blackbox::template rebind< projField >::other FBlackbox;
		const projField Fp(*genprime);
		FBlackbox Ap(A, Fp );

// 		FBlackbox Ap(Fp, A.rowdim(), A.coldim() );
//         typename Blackbox::template rebind<projField>()(Ap,A);
        

		commentator().report (Commentator::LEVEL_ALWAYS,INTERNAL_DESCRIPTION) << "Integer Rank is done modulo " << *genprime << std::endl;

		rankin(r, Ap, RingCategories::ModularTag(), M);
		commentator().stop ("done", NULL, "iirank");
		return r;
	}


	template <class Blackbox, class MyMethod>
	inline unsigned long &rank (unsigned long                     &r,
				    const Blackbox                    &A,
				    const RingCategories::IntegerTag  &tag,
				    const MyMethod                    &M)
	{
        return integral_rank(r,A,M);
    }

	// error hanlder for rational domain
	template <class Blackbox, class Method>
	inline unsigned long &rank (unsigned long                           &r,
				    const Blackbox                          &A,
				    const RingCategories::RationalTag     &tag,
				    const Method                           &M)
	{
		commentator().start ("Rational Rank", "Rrank");
		// Same mapping as the integer one
		integral_rank(r, A, M);
		commentator().stop ("done", NULL, "Rrank");
		return r;
	}


	// More specialized to avoid ambiguity and force rational rank
	template <class Blackbox>
	inline unsigned long &rank (unsigned long           &r,
				    const Blackbox                      &A,
				    const RingCategories::RationalTag   &tag,
				    const Method::SparseElimination     &M)
	{
		commentator().start ("Rational Rank", "Rrank");
		return integral_rank(r, A, M);
		commentator().stop ("done", NULL, "Rrank");
	}

} // LinBox


#ifndef LINBOX_EXTENSION_DEGREE_MAX
#define LINBOX_EXTENSION_DEGREE_MAX 19
#endif

namespace LinBox
{
	template <class Blackbox>
	inline unsigned long &rank (unsigned long                     &r,
				    const Blackbox                    &A,
				    const RingCategories::ModularTag  &tag,
				    const Method::Blackbox            & m)
	{
		commentator().start ("BB Rank", "extend");
		if (m.certificate()) {
			typedef typename Blackbox::Field Field;
			const Field& F = A.field();
			integer a,c; F.cardinality(a); F.characteristic(c);
			if (a != c) {
				uint64_t extend = (uint64_t)Givaro::FF_EXPONENT_MAX(a,(integer)LINBOX_EXTENSION_DEGREE_MAX);
				if (extend > 1) {
					commentator().report (Commentator::LEVEL_ALWAYS,INTERNAL_WARNING) << "Extension of degree " << extend << std::endl;
					Givaro::Extension<Field> EF( F, typename Givaro::Extension<Field>::Residu_t(extend));
					typedef typename Blackbox::template rebind< Givaro::Extension<Field>  >::other FBlackbox;
					FBlackbox Ap(A, EF);
					rank(r, Ap, tag, Method::Wiedemann(m));
				}
				else
					rank(r, A, tag, Method::Wiedemann(m));
			}
			else {
				uint64_t extend = (uint64_t)Givaro::FF_EXPONENT_MAX(c,(integer)LINBOX_EXTENSION_DEGREE_MAX);
				if (extend > 1) {
					commentator().report (Commentator::LEVEL_ALWAYS,INTERNAL_WARNING) << "Word size extension : " << extend << std::endl;
					const Givaro::GFqDom<int64_t> EF( (uint64_t)c, extend);
					typedef typename Blackbox::template rebind< Givaro::GFqDom<int64_t> >::other FBlackbox;
					FBlackbox Ap(A, EF);
					rank(r, Ap, tag, Method::Wiedemann(m));
				}
				else
					rank(r, A, tag, Method::Wiedemann(m));
			}
		}
		else
			rank(r, A, tag, Method::Wiedemann(m));
		commentator().stop ("done", NULL, "extend");
		return r;
	}
}

/*namespace LinBox
{
	template <class Blackbox>
	inline unsigned long &rank (unsigned long                      &r,
				    const Blackbox                     &A,
				    const  RingCategories::ModularTag  &tag,
				    const Method::Blackbox             & m)
	{
		return rank(r, A, tag, Method::Wiedemann(m));
	}
}*/

namespace LinBox { /*  rankin */

	template <class Field, class Method>
	inline unsigned long &rankin (unsigned long                   &r,
				      SparseMatrix<Field, SparseMatrixFormat::SparseSeq>  &A,
				      const Method                    &M)
	{
		return rankin(r, A, typename FieldTraits<Field>::categoryTag(), M);
	}


	template <class Blackbox, class Ring>
	inline unsigned long &rankin (unsigned long                       &r,
				      Blackbox                            &A,
				      const RingCategories::IntegerTag    &tag,
				      const Method::SparseElimination     &M)
	{
		commentator().start ("Integer Rank inplace", "irank");
		typedef Givaro::Modular<double> Field;
		integer mmodulus;
		FieldTraits<Field>::maxModulus(mmodulus);
		RandomPrimeIterator genprime( (unsigned int) floor (log((double)mmodulus) ) );
		++genprime;
		typedef typename Blackbox::template rebind< Field >::other FBlackbox;
		const Field Fp(*genprime);
		FBlackbox Ap(A, Fp);
		commentator().report (Commentator::LEVEL_ALWAYS,INTERNAL_WARNING) << "Integer Rank is done modulo " << *genprime << std::endl;
		rankin(r, Ap, RingCategories::ModularTag(), M);
		commentator().stop ("done", NULL, "irank");
		return r;
	}

	/// specialization to \f$ \mathbf{F}_2 \f$
	inline unsigned long &rankin (unsigned long                       &r,
				      GaussDomain<GF2>::Matrix            &A,
				      const Method::SparseElimination     &)//M
	{
		commentator().start ("Sparse Elimination Rank over GF2", "serankmod2");
		GaussDomain<GF2> GD ( A.field() );
		GD.rankin (r, A, Specifier::PIVOT_LINEAR);
		commentator().stop ("done", NULL, "serankmod2");
		return r;
	}

	/// specialization to \f$ \mathbf{F}_2 \f$
	inline unsigned long &rankin (unsigned long                       &r,
				      GaussDomain<GF2>::Matrix            &A,
				      const RingCategories::ModularTag    &,//tag
				      const Method::SparseElimination     &M)
	{
		return rankin(r, A, M);
	}


	/// A is modified.
	template <class Field>
	inline unsigned long &rankin (unsigned long                     &r,
				      BlasMatrix<Field>               &A,
				      const RingCategories::ModularTag  &tag,
				      const Method::BlasElimination     &M)
	{

		commentator().start ("BlasBB Rank", "blasbbrank");
		const Field F = A.field();
		BlasMatrixDomain<Field> D(F);
		r = D.rankin(static_cast< BlasMatrix<Field>& >(A));
		commentator().stop ("done", NULL, "blasbbrank");
		return r;
	}

	template <class Field>
	inline unsigned long &rankin (unsigned long       &r,
				      BlasMatrix<Field>               &A,
				    const RingCategories::ModularTag  &tag,
				    const Method::Elimination         &m)
	{
			return rankin(r, A, tag, Method::BlasElimination(m));
	}





	template <class Blackbox>
	inline unsigned long &rankin (unsigned long                    &r,
				    Blackbox                   &A,
				    const RingCategories::ModularTag &tag,
				    const Method::Hybrid             &m)
	{ 
        return rankin(r, A, tag, Method::Elimination( m ));
	}


	// A is modified.
	template <class Blackbox>
	inline unsigned long &rankin (unsigned long                      &r,
				      Blackbox                             &A,
				      const RingCategories::ModularTag   &tag,
				      const Method::SparseElimination    &M)
	{
		commentator().start ("Sparse Elimination Rank", "serank");
		GaussDomain<typename Blackbox::Field> GD (A.field());
		GD.rankin( r, A, M.strategy ());
		commentator().stop ("done", NULL, "serank");
		return r;
	}

} // LinBox

#endif // __LINBOX_rank_INL


// Local Variables:
// mode: C++
// tab-width: 4
// indent-tabs-mode: nil
// c-basic-offset: 4
// End:
// vim:sts=4:sw=4:ts=4:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
