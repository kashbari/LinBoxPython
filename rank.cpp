/*
 * examples/sparseelimrank.C
 *
 * Copyright (C) 2006, 2010  J-G Dumas
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
 */

/** \file examples/sparseelimrank.C
 * @example  examples/sparseelimrank.C
 \brief Gaussian elimination Rank of sparse matrix over Z or Zp.
 \ingroup examples
 */

#include <linbox/linbox-config.h>

#include <iostream>
#include <vector>
#include <utility>
#include <givaro/zring.h>
#include <givaro/givrational.h>

#include <givaro/modular.h>
#include <linbox/field/gf2.h>
#include <linbox/matrix/sparse-matrix.h>
#include <linbox/blackbox/zero-one.h>
#include <linbox/solutions/rank.h>
#include <linbox/util/matrix-stream.h>

using namespace LinBox;
using namespace std;

int main (int argc, char **argv)
{
	commentator().setMaxDetailLevel (-1);
	commentator().setMaxDepth (-1);
	commentator().setReportStream (std::cerr);
	commentator().setBriefReportStream (std::cout);

	long unsigned int r;

	Givaro::QField<Givaro::Rational> QQ;
	MatrixStream<Givaro::QField<Givaro::Rational>> ms( QQ, cin );
	SparseMatrix<Givaro::QField<Givaro::Rational>, SparseMatrixFormat::SparseSeq > A ( ms );
	if (A.rowdim() <= 20 && A.coldim() <= 20) A.write(std::cerr << "A:=",Tag::FileFormat::Maple) << ';' << std::endl;


	cerr << "A is " << A.rowdim() << " by " << A.coldim() << endl;

	LinBox::rank (r, A, Method::SparseElimination() );

	cout << r << endl;
	return 0;
}

