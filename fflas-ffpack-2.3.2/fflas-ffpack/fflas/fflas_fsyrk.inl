/* -*- mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
// vim:sts=4:sw=4:ts=4:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
/*
 * Copyright (C) 2016 the FFLAS-FFPACK group
 *
 * Written by Clement Pernet <Clement.Pernet@imag.fr>
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

#ifndef __FFLASFFPACK_fflas_fsyrk_INL
#define __FFLASFFPACK_fflas_fsyrk_INL

namespace FFLAS {
    template<class Field>
    inline typename Field::Element_ptr
    fsyrk (const Field& F,
           const FFLAS_UPLO UpLo,
           const FFLAS_TRANSPOSE trans,
           const size_t N,
           const size_t K,
           const typename Field::Element alpha,
           typename Field::ConstElement_ptr A, const size_t lda,
           const typename Field::Element beta,
           typename Field::Element_ptr C, const size_t ldc){
        
            //@TODO: write an optimized iterative basecase
        if (N==1){ // Base case
            F.mulin (*C, beta);
			size_t incA = (trans==FFLAS::FflasNoTrans)?1:lda;
            F.axpyin (*C, alpha, fdot (F, K, A, incA, A, incA));
            return C;
        } else if (K==1){
            if (!F.isOne(beta))
                fscalin (F, N, N, beta, C, ldc);
			size_t incA = (trans==FFLAS::FflasNoTrans)?lda:1;
            fger (F, N, N, alpha, A, incA, A, incA, C, ldc);
            return C;
        } else {
            size_t N1 = N>>1;
            size_t N2 = N - N1;
            size_t K1 = K>>1;
            size_t K2 = K - K1;
                // Comments written for the case UpLo==FflasUpper, trans==FflasNoTrans
			size_t incRow,incCol;
			FFLAS_TRANSPOSE oppTrans;
			if (trans==FflasNoTrans) {incRow=lda,incCol=1;oppTrans=FflasTrans;}
			else {incRow = 1; incCol = lda;oppTrans=FflasNoTrans;}

            typename Field::ConstElement_ptr A21 = A + N1*incRow;
            typename Field::ConstElement_ptr A22 = A21 + K1*incCol;
            typename Field::ConstElement_ptr A12 = A + K1*incCol;
            typename Field::Element_ptr C12 = C + N1;
            typename Field::Element_ptr C21 = C + N1*ldc;
            typename Field::Element_ptr C22 = C12 + N1*ldc;
                // C11 <- alpha A11 x A11^T + beta C11
            fsyrk (F, UpLo, trans, N1, K1, alpha, A, lda, beta, C, ldc);
                // C11 <- alpha A12 x A12^T + C11
            fsyrk (F, UpLo, trans, N1, K2, alpha, A12, lda, F.one, C, ldc);
                // C22 <- alpha A21 x A21^T + beta C22
            fsyrk (F, UpLo, trans, N2, K1, alpha, A21, lda, beta, C22, ldc);
                // C22 <- alpha A22 x A22^T + C22
            fsyrk (F, UpLo, trans, N2, K2, alpha, A22, lda, F.one, C22, ldc);

            if (UpLo == FflasUpper) {
						// C12 <- alpha A11 * A21^T + beta C12
					fgemm (F, trans, oppTrans, N1, N2, K1, alpha, A, lda, A21, lda,
						   beta, C12, ldc);
						// C12 <- alpha A12 * A22^T + C12
					fgemm (F, trans, oppTrans, N1, N2, K2, alpha, A12, lda, A22, lda,
						   F.one, C12, ldc);
			} else {
					// C21 <- alpha A21 * A11^T + beta C21
				fgemm (F, trans, oppTrans, N2, N1, K1, alpha, A21, lda, A, lda,
					   beta, C21, ldc);
					// C21 <- alpha A12 * A22^T + C21
				fgemm (F, trans, oppTrans, N2, N1, K2, alpha, A22, lda, A12, lda,
					   F.one, (UpLo==FflasUpper)?C12:C21, ldc);

			}
			return C;
        }
    }
    template<class Field>
    inline typename Field::Element_ptr
    fsyrk (const Field& F,
           const FFLAS_UPLO UpLo,
           const FFLAS_TRANSPOSE trans,
           const size_t N,
           const size_t K,
           const typename Field::Element alpha,
           typename Field::Element_ptr A, const size_t lda,
           typename Field::ConstElement_ptr D, const size_t incD,
		   const typename Field::Element beta,
           typename Field::Element_ptr C, const size_t ldc, const size_t threshold){

		size_t incRow,incCol;
		FFLAS_TRANSPOSE oppTrans;
		if (trans==FflasNoTrans) {incRow=lda;incCol=1;oppTrans=FflasTrans;}
		else {incRow = 1; incCol = lda;oppTrans=FflasNoTrans;}

		if (N <= threshold){
			typename Field::Element_ptr temp = fflas_new (F, N, K);
			size_t ldt;
			if (trans==FFLAS::FflasNoTrans){
				ldt =K;
				fassign(F, N, K, A, lda, temp, ldt);
			} else{
				ldt = N;
				fassign(F, K, N, A, lda, temp, ldt);
			}
			typename Field::Element_ptr Ai = A;
			typename Field::ConstElement_ptr Di = D;
			for (; Ai != A + K*incCol; Ai += incCol, Di+=incD){
				fscalin (F, N, *Di, Ai, incRow);
			}
			FFLAS::fgemm (F, trans, oppTrans, N, N, K, alpha, A, lda, temp, ldt, beta, C, ldc); 
			FFLAS::fflas_delete(temp);
			return C;

		}else {
            size_t N1 = N>>1;
            size_t N2 = N - N1;
                // Comments written for the case UpLo==FflasUpper, trans==FflasNoTrans

			typename Field::Element_ptr A2 = A + N1*incRow;
            typename Field::Element_ptr C12 = C + N1;
            typename Field::Element_ptr C21 = C + N1*ldc;
            typename Field::Element_ptr C22 = C12 + N1*ldc;

			typename Field::Element_ptr temp = fflas_new (F, std::max(N2,N1),K);
			size_t ldt, incRowT,incColT;
			if (trans==FflasNoTrans) {ldt=K; incRowT=ldt; incColT=1;}
			else {ldt = N2; incRowT=1; incColT=ldt;}

				// temp <- A2 x D1
			typename Field::Element_ptr Ai = A2, Ti = temp;
			typename Field::ConstElement_ptr Di = D;
			for (; Ai != A2 + K*incCol; Ai += incCol, Ti += incColT, Di+=incD){
				fscal (F, N2, *Di, Ai, incRow, Ti, incRowT);
			}
			if (UpLo == FflasUpper) {
					// C12 <- alpha A1 x temp^T + beta C12
				fgemm (F, trans, oppTrans, N1, N2, K, alpha, A, lda, temp, ldt, beta, C12, ldc);
			} else {
					// C21 <- alpha temp x A11^T + beta C21
				fgemm (F, trans, oppTrans, N2, N1, K, alpha, temp, ldt, A, lda, beta, C21, ldc);
			}
			fflas_delete (temp);

                // C11 <- alpha A1 x D1 x A1^T + beta C11 and A1 <- A1 x D1
            fsyrk (F, UpLo, trans, N1, K, alpha, A, lda, D, incD, beta, C, ldc, threshold);
                // C22 <- alpha A2 x D1 x A2^T + beta C22 and A2 <- A2 x D1
            fsyrk (F, UpLo, trans, N2, K, alpha, A2, lda, D, incD, beta, C22, ldc, threshold);

			return C;
        }
    }
}

#endif //__FFLASFFPACK_fflas_fsyrk_INL
