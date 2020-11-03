/* -*- mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
/*
 * Copyright (C) 2014 FFLAS-FFACK group
 *
 * Written by Clement Pernet <Clement.Pernet@imag.fr>
 * Brice Boyer (briceboyer) <boyer.brice@gmail.com>
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

#ifndef __FFLASFFPACK_ffpack_fgetrs_INL
#define __FFLASFFPACK_ffpack_fgetrs_INL


namespace FFPACK {


	template <class Field>
	void
	fgetrs (const Field& F,
		const FFLAS::FFLAS_SIDE Side,
		const size_t M, const size_t N, const size_t R,
		typename Field::Element_ptr A, const size_t lda,
		const size_t *P, const size_t *Q,
		typename Field::Element_ptr B, const size_t ldb,
		int * info)
	{

		*info =0;
		if (Side == FFLAS::FflasLeft) { // Left looking solve A X = B

			solveLB2 (F, FFLAS::FflasLeft, M, N, R, A, lda, Q, B, ldb);

			applyP (F, FFLAS::FflasLeft, FFLAS::FflasNoTrans,
				N, 0,(int) R, B, ldb, Q);

			bool consistent = true;
			for (size_t i = R; i < M; ++i)
				for (size_t j = 0; j < N; ++j)
					if (!F.isZero (*(B + i*ldb + j)))
						consistent = false;
			if (!consistent) {
				std::cerr<<"System is inconsistent"<<std::endl;
				*info = 1;
			}
			// The last rows of B are now supposed to be 0
#if 0
			for (size_t i = R; i < M; ++i)
				for (size_t j = 0; j < N; ++j)
					*(B + i*ldb + j) = F.zero;
#endif

			ftrsm (F, FFLAS::FflasLeft, FFLAS::FflasUpper, FFLAS::FflasNoTrans, FFLAS::FflasNonUnit,
			       R, N, F.one, A, lda , B, ldb);

			applyP (F, FFLAS::FflasLeft, FFLAS::FflasTrans,
				N, 0,(int) R, B, ldb, P);

		}
		else { // Right Looking X A = B

			applyP (F, FFLAS::FflasRight, FFLAS::FflasTrans,
				M, 0,(int) R, B, ldb, P);

			ftrsm (F, FFLAS::FflasRight, FFLAS::FflasUpper, FFLAS::FflasNoTrans, FFLAS::FflasNonUnit,
			       M, R, F.one, A, lda , B, ldb);

			fgemm (F, FFLAS::FflasNoTrans, FFLAS::FflasNoTrans, M, N-R, R, F.one,
			       B, ldb, A+R, lda, F.mOne, B+R, ldb);

			bool consistent = true;
			for (size_t i = 0; i < M; ++i)
				for (size_t j = R; j < N; ++j)
					if (!F.isZero (*(B + i*ldb + j)))
						consistent = false;
			if (!consistent) {
				std::cerr<<"System is inconsistent"<<std::endl;
				*info = 1;
			}
			// The last cols of B are now supposed to be 0

			applyP (F, FFLAS::FflasRight, FFLAS::FflasNoTrans,
				M, 0,(int) R, B, ldb, Q);

			solveLB2 (F, FFLAS::FflasRight, M, N, R, A, lda, Q, B, ldb);
		}
	}

	template <class Field>
	typename Field::Element_ptr
	fgetrs (const Field& F,
		const FFLAS::FFLAS_SIDE Side,
		const size_t M, const size_t N, const size_t NRHS, const size_t R,
		typename Field::Element_ptr A, const size_t lda,
		const size_t *P, const size_t *Q,
		typename Field::Element_ptr X, const size_t ldx,
		typename Field::ConstElement_ptr B, const size_t ldb,
		int * info)
	{
		*info =0;
		typename Field::Element_ptr W;
		size_t ldw;

		if (Side == FFLAS::FflasLeft) { // Left looking solve A X = B

			// Initializing X to 0 (to be optimized)
			FFLAS::fzero(F,N,NRHS,X,ldx);
			// for (size_t i = 0; i <N; ++i)
			// for (size_t j=0; j< NRHS; ++j)
			// F.assign (*(X+i*ldx+j), F.zero);

			if (M > N){ // Cannot copy B into X
				W = FFLAS::fflas_new (F, M, NRHS);
				ldw = NRHS;
				FFLAS::fassign(F,M,NRHS,B,ldb,W,ldw);

				solveLB2 (F, FFLAS::FflasLeft, M, NRHS, R, A, lda, Q, W, ldw);

				applyP (F, FFLAS::FflasLeft, FFLAS::FflasNoTrans,
					NRHS, 0,(int) R, W, ldw, Q);

				bool consistent = true;
				for (size_t i = R; i < M; ++i)
					for (size_t j = 0; j < NRHS; ++j)
						if (!F.isZero (*(W + i*ldw + j)))
							consistent = false;
				if (!consistent) {
					std::cerr<<"System is inconsistent"<<std::endl;
					*info = 1;
					FFLAS::fflas_delete (W);
					return X;
				}
				// Here the last rows of W are supposed to be 0

				ftrsm (F, FFLAS::FflasLeft, FFLAS::FflasUpper, FFLAS::FflasNoTrans, FFLAS::FflasNonUnit,
				       R, NRHS, F.one, A, lda , W, ldw);

				FFLAS::fassign(F,R,NRHS,W,ldw,X,ldx);

				FFLAS::fflas_delete (W);
				applyP (F, FFLAS::FflasLeft, FFLAS::FflasTrans,
					NRHS, 0,(int) R, X, ldx, P);

			}
			else { // Copy B to X directly

				FFLAS::fassign(F,M,NRHS,B,ldb,X,ldx);

				solveLB2 (F, FFLAS::FflasLeft, M, NRHS, R, A, lda, Q, X, ldx);

				applyP (F, FFLAS::FflasLeft, FFLAS::FflasNoTrans,
					NRHS, 0,(int) R, X, ldx, Q);

				bool consistent = true;
				for (size_t i = R; i < M; ++i)
					for (size_t j = 0; j < NRHS; ++j)
						if (!F.isZero (*(X + i*ldx + j)))
							consistent = false;
				if (!consistent) {
					std::cerr<<"System is inconsistent"<<std::endl;
					*info = 1;
					return X;
				}
				// Here the last rows of W are supposed to be 0

				ftrsm (F, FFLAS::FflasLeft, FFLAS::FflasUpper, FFLAS::FflasNoTrans, FFLAS::FflasNonUnit,
				       R, NRHS, F.one, A, lda , X, ldx);

				applyP (F, FFLAS::FflasLeft, FFLAS::FflasTrans,
					NRHS, 0,(int) R, X, ldx, P);
			}
			return X;

		}
		else { // Right Looking X A = B

			FFLAS::fzero(F,NRHS,M,X,ldx);
			// for (size_t i = 0; i <NRHS; ++i)
			// for (size_t j=0; j< M; ++j)
			// F.assign (*(X+i*ldx+j), F.zero);

			if (M < N) {
				W = FFLAS::fflas_new (F, NRHS, N);
				ldw = N;
				FFLAS::fassign (F,NRHS, N, B, ldb, W, ldw);

				applyP (F, FFLAS::FflasRight, FFLAS::FflasTrans,
					NRHS, 0,(int) R, W, ldw, P);

				ftrsm (F, FFLAS::FflasRight, FFLAS::FflasUpper, FFLAS::FflasNoTrans, FFLAS::FflasNonUnit,
				       NRHS, R, F.one, A, lda , W, ldw);

				fgemm (F, FFLAS::FflasNoTrans, FFLAS::FflasNoTrans, NRHS, N-R, R, F.one,
				       W, ldw, A+R, lda, F.mOne, W+R, ldw);

				bool consistent = true;
				for (size_t i = 0; i < NRHS; ++i)
					for (size_t j = R; j < N; ++j)
						if (!F.isZero (*(W + i*ldw + j)))
							consistent = false;
				if (!consistent) {
					std::cerr<<"System is inconsistent"<<std::endl;
					*info = 1;
					FFLAS::fflas_delete (W);
					return X;
				}
				// The last N-R cols of W are now supposed to be 0
				FFLAS::fassign (F, NRHS,R,  W , ldb, X ,ldx);
				FFLAS::fflas_delete (W);
				applyP (F, FFLAS::FflasRight, FFLAS::FflasNoTrans,
					NRHS, 0,(int) R, X, ldx, Q);

				solveLB2 (F, FFLAS::FflasRight, NRHS, M, R, A, lda, Q, X, ldx);

			}
			else { // M >=N
				FFLAS::fassign(F,NRHS,N,B,ldb,X,ldx);

				applyP (F, FFLAS::FflasRight, FFLAS::FflasTrans,
					NRHS, 0,(int) R, X, ldx, P);

				ftrsm (F, FFLAS::FflasRight, FFLAS::FflasUpper, FFLAS::FflasNoTrans, FFLAS::FflasNonUnit,
				       NRHS, R, F.one, A, lda , X, ldx);

				fgemm (F, FFLAS::FflasNoTrans, FFLAS::FflasNoTrans, NRHS, N-R, R, F.one,
				       X, ldx, A+R, lda, F.mOne, X+R, ldx);

				bool consistent = true;
				for (size_t i = 0; i < NRHS; ++i)
					for (size_t j = R; j < N; ++j)
						if (!F.isZero (*(X + i*ldx + j)))
							consistent = false;
				if (!consistent) {
					std::cerr<<"System is inconsistent"<<std::endl;
					*info = 1;
					return X;
				}
				// The last N-R cols of W are now supposed to be 0

				applyP (F, FFLAS::FflasRight, FFLAS::FflasNoTrans,
					NRHS, 0,(int) R, X, ldx, Q);

				solveLB2 (F, FFLAS::FflasRight, NRHS, M, R, A, lda, Q, X, ldx);

			}
			return X;
		}
	}

} // FFPACK

#endif // __FFLASFFPACK_ffpack_fgetrs_INL
