/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    ntl-hankel.inl     NTL_Hankel.cpp file
 *    Copyright (C) 2003 Austin Lobo, B. David Saunders
 *
 *    Author: Austin Lobo
 *    Linbox version 2001 and 2002
 *
 *    This file is included in the template description of ntl-Hankel.h
 *    it contains the implementations of templatized member functions in the
 *    partial template  specialization for hankel matrices that
 *    are manipulated in fields and rings according to the arithmetic
 *    in the ntl package from V. Shoup
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

 *    Everything is in the Linbox namespace by virtue of the #include
 *    in ntl-Hankel.h
 *-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#ifndef __LINBOX_bb_ntl_hankel_INL
#define __LINBOX_bb_ntl_hankel_INL

#include <iostream>
#include <fstream>
#include <NTL/ZZ_pX.h>

namespace LinBox
{
	/*-----------------------------------------------------------------
	 *----    Destructor
	 *----------------------------------------------------------------*/
	template <class Field>
	inline Hankel<Field>::~Hankel()
	{
#ifdef DBGMSGS
		std::cout << "Hankel::~Hankel():\tDestroyed a " << this->rowDim << "x"<< this->colDim<<
						 " Hankel matrix "<< std::endl;
#endif
	}//---- Destructor ----



#if 0
	/*-----------------------------------------------------------------
	 *----    Default Constructor
	 *----------------------------------------------------------------*/
	template <class Field>
	Hankel<Field>::Hankel()
	{
		this->shape.shape(BlackboxSpecifier::HANKEL);
#ifdef DBGMSGS
		std::cout << "Hankel::Hankel():\tCreated a " << this->rowDim << "x"<< this->colDim<<
						" Hankel matrix "<< std::endl;
#endif

	}//----- Zero Param Constructor ---- [Tested 6/14/02 -- Works]
#endif




	/*-----------------------------------------------------------------
	 *----- Constructor With User-Supplied First Row And Column
	 *----------------------------------------------------------------*/
	template <class Field>
	Hankel<Field>::Hankel( const Field F,
			       const std::vector<typename Field::Element>&v) : Father_t(F)
	{
		// Assumes that the input is a vector of ZZ_p else things will FAIL
		if ( isEven(v.size()) )
		{
			std::cout << "There must be an ODD number of entries in the input vector " <<
			"The length given is " << v.size();
		}
		assert( isOdd(v.size()) );

		this->rowDim = (1+v.size())/2; // The vector is 0..2n-2;
		this->colDim = (1+v.size())/2;
		this->sysDim = (1+v.size())/2;

		this->pdata.SetMaxLength( (long) v.size());
		//		rpdata.SetMaxLength( v.size());
		for (unsigned int i=0; i< v.size(); i++)
		{
			this->P.setCoeff( this->pdata, i, v[i]);
			//SetCoeff( rpdata, i, v[v.size()-1-i]);
		}

#ifdef DBGMSGS
		std::cout << "Hankel::Hankel(F,V):\tCreated a " << this->rowDim << "x"<< this->colDim<<
						   " Hankel matrix "<< std::endl;
#endif

	}//----- Constructor given a vector----

	/*-----------------------------------------------------------------
	 *----- Constructor With User-Supplied First Row And Column
	 *----------------------------------------------------------------*/
	template <class Field>
	Hankel<Field>::Hankel( const BlasVector<Field>&v ) :
	       	Father_t(v.field())
	{
		//! @warning Assumes that the input is a vector of ZZ_p else things will FAIL
		if ( isEven( v.size() ) )
		{
			std::cout << "There must be an ODD number of entries in the input vector " <<
			"The length given is " << v.size();
		}
		assert(  isOdd(v.size())  );

		this->rowDim = (1+v.size())/2; // The vector is 0..2n-2;
		this->colDim = (1+v.size())/2;
		this->sysDim = (1+v.size())/2;

		this->pdata.SetMaxLength( (long) v.size());
		for (unsigned int i=0; i< v.size(); i++)
		{
			this->P.setCoeff( this->pdata, i, v[i]);
		}

#ifdef DBGMSGS
		std::cout << "Hankel::Hankel(F,V):\tCreated a " << this->rowDim << "x"<< this->colDim<<
						   " Hankel matrix "<< std::endl;
#endif

	}//----- Constructor given a vector----


	/*-----------------------------------------------------------------
	 *-----    Print The Matrix To Screen
	 *----------------------------------------------------------------*/
	template <class Field>
	void Hankel<Field>::print(std::ostream& os) const
	{
		size_t N;

		os<< this->rowDim << " " << this->colDim << " " << this->shape.shape() << std::endl;
		N = (this->rowDim-1)<<1;

		if ( N < 20 ) {            // Print small matrices in dense format
			Element tmp;
			size_t i;
			for (i = N ; i >= this->colDim-1; i--) {
				size_t j;
				for ( j = 0; j < this->colDim ; j++)
					os << " " << this->P.getCoeff(tmp, this->pdata,i-j) ;
				os << std::endl;
			}
		}
		else {
			// Print large matrices' first row and col
			os << "<Hankel<";
			this->P.write(os, this->pdata) << ">>\n";
		} //[v(2n-2),....,v(0)]; where v(0) is the top right entry of the matrix

		return;
	} //---- print()-----




	/*-----------------------------------------------------------------
	 *----    The infamous clone has been created here
	 *----------------------------------------------------------------*/
	//template <class Field, class Vector>
	//BlackboxArchetype<Vector>* Hankel<Field, Vector>::clone() const
	//{
	//return new Hankel(*this);
	//}// ------ This is not tested.


	/*-----------------------------------------------------------------
	 *----    Save To File, Given Destination Filename
	 *----------------------------------------------------------------*/
	template <class Field>
	void Hankel<Field>::print( char *outFileName) const
	{
		// int i, j, N;

		std::cout << "Printing hankel matrix to " << outFileName << std::endl;

		if ( outFileName == NULL )
			print();    // Print to stdout if no file is specified
		else {
			std::ofstream o_fp(outFileName, std::ios::out);
			o_fp << this->rowDim << " " << this->colDim << " " << this->shape.shape() << std::endl ;
			o_fp << "<Hankel<";
			this->P.write(o_fp, this->pdata) << ">>\n";

			o_fp.close();
		}
		return;
	} // print(char *)



	/*-----------------------------------------------------------------
	 *    Make the matrix LOWER triangular with determinant 1.
	 *    i.e. clear the last this->coldim-1 elements in the this->data vector
	 *----------------------------------------------------------------*/
#if 0
	template <class Field>
	void Hankel<Field>::setToUniModLT()
	{
		int L = int(this->rowDim-1)<<1;
		this->shape.shape(BlackboxSpecifier::UNIMOD_LT);

		for (int i= int(this->rowDim)-1; i <= L; i++ ) {
			// zero out the below-diagonal entries
			this->P.setCoeff(this->pdata,i,field().zero);
		}
		// set the antidiagonal to 1
		this->P.setCoeff( this->pdata, this->rowDim-1, field().one);       // update the corresponding coeff of this->pdata
		//reverse(rpdata,this->pdata);        // no need to construct the transpose
		return;
	}//



	/*-----------------------------------------------------------------
	 *    Make matrix a unimodular UPPER Triangular with det 1
	 *    i.e. clear the first N-1 elements in the this->data vector
	 *    and make the elements below the anti-diagonal all zero
	 *----------------------------------------------------------------*/
	template <class Field>
	void Hankel<Field>::setToUniModUT()
	{
		this->shape.shape(BlackboxSpecifier::UNIMOD_UT);

		for (size_t i=0; i < this->rowDim-1; i++ ) {
			// zero out the below-antidiagonal entries
			this->P.setCoeff(this->pdata, i , field().zero);
		}

		// set antidiagonal to 1
		this->P.setCoeff(this->pdata,this->rowDim-1, field().one);      // update the corresponding coeff of this->pdata
		//reverse(rpdata,this->pdata);    // no need to construct the transpose

		return;
	}//

#endif

	/*-----------------------------------------------------------------
	 *    Apply the matrix to a vector Here the input and output
	 *    vectors are both over the SAME prime ZZ_p field as the
	 *    Hankel matrix itself.
	 *----------------------------------------------------------------*/
	template <class Field>
	template<class OutVector, class InVector>
	OutVector& Hankel<Field>::apply( OutVector &v_out,
					 const InVector& v_in) const
	{
		if (v_out.size() != this->rowdim())
			std::cout << "\tHankel::apply()\t output vector not correct size, at "
			<< v_out.size() << ". System rowdim is" <<  this->rowdim() << std::endl;
		if ( v_out.size() != v_in.size())
			std::cout << "\tHankel::apply()\t input vector not correct size at "
			<< v_in.size() << ". System coldim is" <<  this->coldim() << std::endl;
		assert((v_out.size() == this->rowdim()) &&
		       (v_in.size() == this->coldim()))  ;

		NTL::ZZ_pX pxOut, pxIn;
		pxIn.SetMaxLength( (long) v_in.size()-1);
		for (unsigned int i=0; i< v_in.size(); i++)
			this->P.setCoeff( pxIn, i, v_in[i]);

#ifdef DBGMSGS
		std::cout << "\npX in is " << pxIn << std::endl;
		std::cout << "multiplied by " << this->pdata << std::endl;
#endif
		mul(pxOut,pxIn,this->pdata);

#ifdef DBGMSGS
		std::cout <<"pxOut is " << pxOut << std::endl;
#endif
		int N = (int)this->rowdim();
		for ( int i= 0; i < N; i++)
			this->P.getCoeff(v_out[(size_t)(N-1-i)], pxOut, (size_t)(N-1+i));

		return v_out;

	}


	/*-----------------------------------------------------------------
	 *    Apply the transposed matrix to a vector Here the input and output
	 *    vectors are both over the SAME prime ZZ_p field as the
	 *    Hankel matrix itself. Calls the multiply from the Toeplitz matrix
	 *    Since Hankel is symmetric, this is the same as apply
	 *----------------------------------------------------------------*/
	template <class Field>
	template <class OutVector, class InVector>
	OutVector& Hankel<Field>::applyTranspose( OutVector &v_out,
						  const InVector& v_in) const
	{
		return(v_out = apply(v_out,v_in));

	}



} // namespace LinBox

#endif //__LINBOX_bb_ntl_hankel_INL

// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
