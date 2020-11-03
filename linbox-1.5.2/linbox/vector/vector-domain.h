/* linbox/vector/vector-domain.h
 * Copyright (C) 2001-2002 Bradford Hovinen
 *
 * Written by Bradford Hovinen <hovinen@cis.udel.edu>
 *
 * ------------------------------------
 * 2002-04-10 Bradford Hovinen <hovinen@cis.udel.edu>
 *
 * There are now two vector domain types: VectorDomainBase and
 * VectorDomain. VectorDomainBase, which in principle can be used independently,
 * contains all functions that require only one vector type (such as axpy, mul,
 * read, and write). VectorDomain inherits VectorDomainBase and implements
 * dotprod, which requires two vector types.
 *
 * ------------------------------------
 * Modified by Dmitriy Morozov <linbox@foxcub.org>. May 27, 2002.
 *
 * Added the modifications for categories and vector traits that were designed
 * at the Rootbeer meeting. Added parametrization of VectorTags by VectorTraits.
 *
 * ------------------------------------
 * 2002-06-04 Bradford Hovinen <hovinen@cis.udel.edu>
 *
 * Reverted change of 2002-04-10, reintegrating VectorDomain and
 * VectorDomainBase. Now using template specialization on the functions, now
 * that I know how to do it.
 *
 * ------------------------------------
 * 2002-06-21 Bradford Hovinen <hovinen@cis.udel.edu>
 *
 * Added methods add, addin, sub, subin, areEqual, isZero, and copy.
 *
 * ------------------------------------
 * 2012Aug -bds
 * VectorDomain<F> inherits from DotProductDomain<F>, which inherits from VectorDomainBase<F>.
 * DotProductDomain<> has a generic definition here and
 * has specializations in field/Modular/ and field/Givaro/.
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

#ifndef __LINBOX_field_vector_domain_H
#define __LINBOX_field_vector_domain_H

#include <iostream>

#include "linbox/linbox-config.h"
#include "linbox/util/debug.h"
#include "linbox/vector/vector-traits.h"
#include "linbox/util/field-axpy.h"

namespace LinBox
{ /*  VectorDomainBase */
	/** @name Vector domain base
	 *
	 * This class provides a virtual base for the VectorDomain and the
	 * DotProductDomain. Its purpose is to provide the field of computation
	 * in a single location.
	 \ingroup vector
	 */
	template <class Field>
	class VectorDomainBase {
	public:
		VectorDomainBase () : _faxpy(nullptr) {std::cout<<"CST DEFAULT VDB"<<std::endl;}
		VectorDomainBase (const Field &F) :  _faxpy(new FieldAXPY<Field>(F))
                { /*std::cerr << "VDD cstor " << this << std::endl;*/}
                ~VectorDomainBase()	{ if (_faxpy != nullptr) delete _faxpy; }

		void init(const Field &F) { if (_faxpy != nullptr) delete _faxpy; _faxpy = new FieldAXPY<Field>(F); }
		inline const Field & field() const { return _faxpy->field(); }
		inline const FieldAXPY<Field>& faxpy() const { return *_faxpy; }
	
	protected:
		const FieldAXPY<Field> * _faxpy;
	};
}

namespace LinBox
{ /*  Dot Product */
	/** @name Dot product domain
	 * @brief Performance-critical dotproducts
	 *
	 * This class contains all of the "high-performance" dot product types
	 * for a vector domain, i.e. dense/dense and dense/sparse parallel. It
	 * allows these dot products to be specialized with very highly tuned
	 * and tightly optimized field-specific implementations, possibly in
	 * assembly language. The other dot products are not considered as
	 * critical for performance, so their implementations are in the derived
	 * class VectorDomain.
	 */
	template <class Field_>
        //	class DotProductDomain : public virtual VectorDomainBase<Field_> {
        class DotProductDomain : public VectorDomainBase<Field_> {
	public:
		//DotProductDomain () { /*std::cerr << "DPD def cstor" << std::endl;*/ }// no def cstor allowed

		typedef Field_ Field;
		typedef typename Field::Element Element;

		DotProductDomain (const Field &F) :
			VectorDomainBase<Field> (F)
		{  /*std::cerr << "DPD cstor " << this << std::endl; */}

		using VectorDomainBase<Field>::field;
		using VectorDomainBase<Field>::init;


	protected:
		template <class Vector1, class Vector2>
		inline Element &dotSpecializedDD (Element &res, const Vector1 &v1, const Vector2 &v2) const;

		template <class Vector1, class Vector2>
		inline Element &dotSpecializedDSP (Element &res, const Vector1 &v1, const Vector2 &v2) const;

	};

}

namespace LinBox
{ /*  Vector Domain */
/* public members
VectorDomain<F>()
VectorDomain<F>(VD)
VectorDomain (F)
VD1 = VD2
VD.field()
VD.write<V>(os, v)
VD.read<V>(os, v)
VD.copy<V1,V2>(v1,v2)
VD.copy<V1,V2>(v1,v2,i,n)
VD.areEqual<V1,V2>(v1,v2)
VD.isZero<V>(v)
VD.dot<V1,V2>(e,v1,v2) aka dotprod
VD.add<V1,V2,V3>(v1,v2,v3)
VD.addin<V1,V2>(v1,v2)
VD.sub<V1,V2,V3>(v1,v2,v3)
VD.subin<V1,V2>(v1,v2)
VD.neg<V1,V2>(v1,v2)
VD.negin<V>(v)
VD.mul<V1,V2>(v1,v2,e)
VD.mulin<V>(v,e)
VD.axpy<V1,V2,V3>(v1,e,v2,v3)
VD.axpyin<V1,V2>(v1,e,v2)
VD.swap<V1,V2>(v1,v2)
VD.random<V>(v)
class Transposition;
class Permutation;
VD.permute<V,PI>(v1,pb,pe)
*/

	/** @name Vector Domain
	 * @brief Vector arithmetic
	 *
	 * This is a generic wrapper around classes matching the
	 * \ref FieldArchetype  interface. It implements vector-vector
	 * operations such as axpy, mul, and dotprod. It also contains an
	 * interface to the underlying field whereby calls simply pass
	 * through. Template specializations permit optimizations to be done on
	 * these operations based on the characteristics of the field.
	 *
	 * This class is usable by itself. Simply supply any preexisting field
	 * as a template parameter and it will work as intended, though its
	 * operation may not be fully optimized.
	 */
	// JGD 01.10.2003 : Why inherit twice from VectorDomainBase<Field> ???
	// bds 2004Apr25 : well, g++ 3.4.3 wants explicit base domains on everything - eases that.
	template <class Field_>
        //	class VectorDomain : public virtual DotProductDomain<Field_> , public virtual VectorDomainBase<Field_> {
        class VectorDomain : public  DotProductDomain<Field_> {
	public:

		typedef Field_ Field;

		typedef typename Field::Element         Element;

		// VectorDomain(): DotProductDomain<Field>() { /*std::cerr << "VD def cstor" << std::endl;*/ }
		//VectorDomain():DotProductDomain<Field>() {}

                //using VectorDomainBase<Field>::init;
                using DotProductDomain<Field>::init;

		/** Copy constructor.
		 * Constructs VectorDomain object by copying the domain.
		 * This is required to allow vector domain objects to be passed
		 * by value into functions.
		 * @param  VD VectorDomain object.
		 */
		VectorDomain (const VectorDomain &VD) :
                        //VectorDomainBase<Field> (VD.field()),
                        DotProductDomain<Field> (VD.field())
		{}

                /** Construct from a field.
		 * @param F Field or ring
		 */
		VectorDomain (const Field &F) :
			//VectorDomainBase<Field> (F), DotProductDomain<Field> (F)
                        DotProductDomain<Field> (F)
		{/* std::cerr << "VD cstor " << this << std::endl;*/}



		/** Assignment operator.
		 * Assigns VectorDomain object MD to field.
		 * @param  VD VectorDomain object.
		 */
		VectorDomain &operator = (const VectorDomain &VD)
		{
			this->init(VD.field());
			//this->_field = VD._field;
			//VectorDomainBase<Field>:: accu = VD.accu;
			return *this;
		}

		/** Retrieve the underlying field
		 * Return a reference to the field that this vector domain
		 * object uses
		 * @return reference to field
		 */

		//using VectorDomainBase<Field>::field;
                using DotProductDomain<Field>::field;

		/** Vector input/output operations
		 * These routines are useful for reading and writing vectors to
		 * and from file streams. They are analagous to field read and
		 * write operations.
		 */

		//@{

		/** Print vector of field elements.
		 * This function assumes the field element has already been
		 * constructed and initialized.
		 * @return output stream to which field element is written.
		 * @param  os  output stream to which field element is written.
		 * @param  x   field element.
		 */
		template <class Vector>
		inline std::ostream &write (std::ostream &os, const Vector &x) const
		{
			return writeSpecialized (os, x, typename VectorTraits<Vector>::VectorCategory ());
		}

		/** Read vector of field elements.
		 * This function assumes the field element has already been
		 * constructed and initialized.
		 * @return input stream from which field element is read.
		 * @param  is  input stream from which field element is read.
		 * @param  x   field element.
		 */
		template <class Vector>
		inline std::istream &read (std::istream &is, Vector &x) const
		{
			return readSpecialized (is, x, typename VectorTraits<Vector>::VectorCategory ());
		}

		//@} Input/Output Operations

		/** @name Vector arithmetic operations
		 * These routes are analogs of field arithmetic operations, but
		 * they take vectors of elements as input. Vector-vector dot
		 * product and vector-vector axpy are supported here.
		 */

		//@{

		/** Vector copy.
		 * Copy a vector to another vector, possibly converting to a
		 * different representation
		 * @param res Output vector
		 * @param v Input vector
		 * @returns reference to output vector
		 */
		template <class Vector1, class Vector2>
		inline Vector1 &copy (Vector1 &res, const Vector2 &v) const
		{
			return copySpecialized (res, v,
						typename VectorTraits<Vector1>::VectorCategory (),
						typename VectorTraits<Vector2>::VectorCategory ());
		}

		/** Vector copy.
		 * Copy a vector to a portion of another vector, possibly
		 * converting to a different representation
		 * @param res Output vector
		 * @param i Index to which to copy
		 * @param len Length (in indices) of output vector to
		 *            invalidate. If len == 0, then copy the whole vector v
		 * @param v Input vector
		 * @returns reference to output vector
		 */
		template <class Vector1, class Vector2>
		inline Vector1 &copy (Vector1 &res, const Vector2 &v, size_t i, size_t len = 0) const
		{
			return copySpecialized (res, v, i, len,
						typename VectorTraits<Vector1>::VectorCategory ());
		}

		/** Vector equality.
		 * @param v1 Input vector
		 * @param v2 Input vector
		 * @returns true if and only if the vectors v1 and v2 are equal
		 */
		template <class Vector1, class Vector2>
		inline bool areEqual (const Vector1 &v1, const Vector2 &v2) const
		{
			return areEqualSpecialized (v1, v2,
						    typename VectorTraits<Vector1>::VectorCategory (),
						    typename VectorTraits<Vector2>::VectorCategory ());
		}

		/** Vector equality with zero.
		 * @param v Input vector
		 * @returns true if and only if the vector v is zero
		 */
		template <class Vector>
		inline bool isZero (const Vector &v) const
		{
			return isZeroSpecialized (v, typename VectorTraits<Vector>::VectorCategory ());
		}

		/** Vector-vector dot product.
		 * @param res element into which to store result
		 * @param v1 Input vector
		 * @param v2 Input vector
		 */
		template <class Vector1, class Vector2>
		inline Element &dot (Element &res, const Vector1 &v1, const Vector2 &v2) const
		{
			return dotSpecialized (res, v1, v2,
					       typename VectorTraits<Vector1>::VectorCategory (),
					       typename VectorTraits<Vector2>::VectorCategory ());
		}

		/** Vector-vector dot product.
		 * Alias for the above, to avoid source incompatibility.
		 * @param res element into which to store result
		 * @param v1 Input vector
		 * @param v2 Input vector
		 */
		template <class Vector1, class Vector2>
		inline Element &dotprod (Element &res, const Vector1 &v1, const Vector2 &v2) const
		{
			return dot (res, v1, v2);
		}

		/** Vector add.
		 * res <- y + x
		 * @param res Vector into which to store result
		 * @param y Input vector y
		 * @param x Input vector x
		 */
		template <class Vector1, class Vector2, class Vector3>
		inline Vector1 &add (Vector1 &res, const Vector2 &y, const Vector3 &x) const
		{
			return addSpecialized (res, y, x,
					       typename VectorTraits<Vector1>::VectorCategory (),
					       typename VectorTraits<Vector2>::VectorCategory (),
					       typename VectorTraits<Vector3>::VectorCategory ());
		}

		/** Vector in-place add.
		 * y <- y + x
		 * @param y Input vector y; result is stored here
		 * @param x Input vector x
		 */
		template <class Vector1, class Vector2>
		inline Vector1 &addin (Vector1 &y, const Vector2 &x) const
		{
			return addinSpecialized (y, x,
						 typename VectorTraits<Vector1>::VectorCategory (),
						 typename VectorTraits<Vector2>::VectorCategory ());
		}

		/** Vector subtract.
		 * res <- y - x
		 * @param res Vector into which to store result
		 * @param y Input vector y
		 * @param x Input vector x
		 */
		template <class Vector1, class Vector2, class Vector3>
		inline Vector1 &sub (Vector1 &res, const Vector2 &y, const Vector3 &x) const
		{
			return subSpecialized (res, y, x,
					       typename VectorTraits<Vector1>::VectorCategory (),
					       typename VectorTraits<Vector2>::VectorCategory (),
					       typename VectorTraits<Vector3>::VectorCategory ());
		}

		/** Vector in-place subtract.
		 * y <- y - x
		 * @param y Input vector y; result is stored here
		 * @param x Input vector x
		 */
		template <class Vector1, class Vector2>
		inline Vector1 &subin (Vector1 &y, const Vector2 &x) const
		{
			return subinSpecialized (y, x,
						 typename VectorTraits<Vector1>::VectorCategory (),
						 typename VectorTraits<Vector2>::VectorCategory ());
		}

		/** Vector negate.
		 * res <- -x
		 * @param res Vector into which to store result
		 * @param x Input vector x
		 */
		template <class Vector1, class Vector2>
		inline Vector1 &neg (Vector1 &res, const Vector2 &x) const
		{
			return negSpecialized (res, x,
					       typename VectorTraits<Vector1>::VectorCategory (),
					       typename VectorTraits<Vector2>::VectorCategory ());
		}

		/** Vector in-place negate.
		 * y <- -y
		 * @param y Input vector y; result is stored here
		 */
		template <class Vector>
		inline Vector &negin (Vector &y) const
		{
			return neginSpecialized (y, typename VectorTraits<Vector>::VectorCategory ());
		}

		/** Scalar-vector multiplication.
		 * res <- a * x
		 * @param res Vector into which to store result
		 * @param x Input vector x
		 * @param a Input element a
		 */
		template <class Vector1, class Vector2>
		inline Vector1 &mul (Vector1 &res, const Vector2 &x, const Element &a) const
		{
			return mulSpecialized (res, x, a, typename VectorTraits<Vector1>::VectorCategory ());
		}

		/** In-place scalar-vector multiplication.
		 * x <- a * x
		 * @param x Input vector x
		 * @param a Input element a
		 */
		template <class Vector>
		inline Vector &mulin (Vector &x, const Element &a) const
		{
			return mulinSpecialized (x, a, typename VectorTraits<Vector>::VectorCategory ());
		}

		/** Vector axpy.
		 * res <- y + a*x
		 * @param res Vector into which to store result
		 * @param a Scalar element a
		 * @param x Input vector x
		 * @param y Input vector y
		 */
		template <class Vector1, class Vector2, class Vector3>
		inline Vector1 &axpy (Vector1 &res, const Element &a, const Vector2 &x, const Vector3 &y) const
		{
			return axpySpecialized (res, y, a, x, typename VectorTraits<Vector1>::VectorCategory ());
		}

		/** Vector in-place axpy.
		 * y <- y + a*x
		 * @param y Input vector y; result is stored here
		 * @param a Scalar element a
		 * @param x Input vector x
		 */
		template <class Vector1, class Vector2>
		inline Vector1 &axpyin (Vector1 &y, const Element &a, const Vector2 &x) const
		{
			return axpyinSpecialized (y, a, x,
						  typename VectorTraits<Vector1>::VectorCategory (),
						  typename VectorTraits<Vector2>::VectorCategory ());
		}

		//@} Vector arithmetic operations

		/** @name Reordering and permutation operations
		 * These routines provide support for swapping vectors and
		 * permuting their entries.
		 */

		//@{

		/** Permutation.
		 *
		 * A permutation is represented as a vector of pairs, each
		 * pair representing a transposition.
		 */
		typedef std::pair<unsigned int, unsigned int> Transposition;
		typedef std::vector<Transposition> Permutation;

		/** Swap the contents of the two given vectors.
		 *
		 * @param v1 First vector
		 * @param v2 Second vector
		 */
		template <class Vector>
		inline void swap (Vector &v1, Vector &v2) const
		{
			swapSpecialized (v1, v2, typename VectorTraits<Vector>::VectorCategory ());
		}

		/** Permute the entries of a given vector using the given
		 * permutation.
		 *
		 * @param v Vector to permute
		 * @param P_start Iterator of the start of the permutation to apply
		 * @param P_end Iterator of the end of the permutation to apply
		 * @return Reference to v
		 */
		template <class Vector, class Iterator>
		inline Vector &permute (Vector   &v,
					Iterator  P_start,
					Iterator  P_end) const
		{
			return permuteSpecialized (v, P_start, P_end,
						   typename VectorTraits<Vector>::VectorCategory ());
		}

		//@}

	
		/*! Random vector.
		 * @param v vector to be randomized.
		 */
		template <class Vector>
		Vector& random(Vector& v)
		{
			typename Field::RandIter r(field());

			typedef typename Vector::iterator iterator;
			for (iterator p = v.begin(); p != v.end(); ++p) r.random(*p);
			return v;
		}

	protected:

		// Specialized function implementations
		template <class Vector>
		std::ostream &writeSpecialized (std::ostream &os, const Vector &x,
						VectorCategories::DenseVectorTag) const;
		template <class Vector>
		std::ostream &writeSpecialized (std::ostream &os, const Vector &x,
						VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector>
		std::ostream &writeSpecialized (std::ostream &os, const Vector &x,
						VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector>
		std::ostream &writeSpecialized (std::ostream &os, const Vector &x,
						VectorCategories::SparseParallelVectorTag) const;

		template <class Vector>
		std::istream &readSpecialized (std::istream &is, Vector &x,
					       VectorCategories::DenseVectorTag) const;
		template <class Vector>
		std::istream &readSpecialized (std::istream &is, Vector &x,
					       VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector>
		std::istream &readSpecialized (std::istream &is, Vector &x,
					       VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector>
		std::istream &readSpecialized (std::istream &is, Vector &x,
					       VectorCategories::SparseParallelVectorTag) const;

		template <class Vector1, class Vector2>
		bool areEqualSpecialized (const Vector1 &v1, const Vector2 &v2,
					  VectorCategories::DenseVectorTag,
					  VectorCategories::DenseVectorTag) const;
		template <class Vector1, class Vector2>
		bool areEqualSpecialized (const Vector1 &v1, const Vector2 &v2,
					  VectorCategories::SparseSequenceVectorTag,
					  VectorCategories::DenseVectorTag) const;
		template <class Vector1, class Vector2>
		bool areEqualSpecialized (const Vector1 &v1, const Vector2 &v2,
					  VectorCategories::SparseAssociativeVectorTag,
					  VectorCategories::DenseVectorTag) const;
		template <class Vector1, class Vector2>
		bool areEqualSpecialized (const Vector1 &v1, const Vector2 &v2,
					  VectorCategories::SparseParallelVectorTag,
					  VectorCategories::DenseVectorTag) const;

		template <class Vector1, class Vector2>
		bool areEqualSpecialized (const Vector1 &v1, const Vector2 &v2,
					  VectorCategories::DenseVectorTag,
					  VectorCategories::SparseSequenceVectorTag) const
		{
			return areEqual (v2, v1);
		}
		template <class Vector1, class Vector2>
		bool areEqualSpecialized (const Vector1 &v1, const Vector2 &v2,
					  VectorCategories::SparseSequenceVectorTag,
					  VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector1, class Vector2>
		bool areEqualSpecialized (const Vector1 &v1, const Vector2 &v2,
					  VectorCategories::SparseAssociativeVectorTag,
					  VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector1, class Vector2>
		bool areEqualSpecialized (const Vector1 &v1, const Vector2 &v2,
					  VectorCategories::SparseParallelVectorTag,
					  VectorCategories::SparseSequenceVectorTag) const;

		template <class Vector1, class Vector2>
		bool areEqualSpecialized (const Vector1 &v1, const Vector2 &v2,
					  VectorCategories::DenseVectorTag,
					  VectorCategories::SparseAssociativeVectorTag) const
		{
			return areEqual (v2, v1);
		}
		template <class Vector1, class Vector2>
		inline bool areEqualSpecialized (const Vector1 &v1, const Vector2 &v2,
						 VectorCategories::SparseSequenceVectorTag,
						 VectorCategories::SparseAssociativeVectorTag) const
		{
			return areEqual (v2, v1);
		}
		template <class Vector1, class Vector2>
		bool areEqualSpecialized (const Vector1 &v1, const Vector2 &v2,
					  VectorCategories::SparseAssociativeVectorTag,
					  VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector1, class Vector2>
		bool areEqualSpecialized (const Vector1 &v1, const Vector2 &v2,
					  VectorCategories::SparseParallelVectorTag,
					  VectorCategories::SparseAssociativeVectorTag) const;

		template <class Vector1, class Vector2>
		bool areEqualSpecialized (const Vector1 &v1, const Vector2 &v2,
					  VectorCategories::DenseVectorTag,
					  VectorCategories::SparseParallelVectorTag) const
		{
			return areEqual (v2, v1);
		}
		template <class Vector1, class Vector2>
		inline bool areEqualSpecialized (const Vector1 &v1, const Vector2 &v2,
						 VectorCategories::SparseSequenceVectorTag,
						 VectorCategories::SparseParallelVectorTag) const
		{
			return areEqual (v2, v1);
		}
		template <class Vector1, class Vector2>
		inline bool areEqualSpecialized (const Vector1 &v1, const Vector2 &v2,
						 VectorCategories::SparseAssociativeVectorTag,
						 VectorCategories::SparseParallelVectorTag) const
		{
			return areEqual (v2, v1);
		}
		template <class Vector1, class Vector2>
		bool areEqualSpecialized (const Vector1 &v1, const Vector2 &v2,
					  VectorCategories::SparseParallelVectorTag,
					  VectorCategories::SparseParallelVectorTag) const;

		template <class Vector>
		bool isZeroSpecialized (const Vector &v, VectorCategories::DenseVectorTag) const;
		template <class Vector>
		bool isZeroSpecialized (const Vector &v, VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector>
		bool isZeroSpecialized (const Vector &v, VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector>
		bool isZeroSpecialized (const Vector &v, VectorCategories::SparseParallelVectorTag) const;

		template <class Vector1, class Vector2>
		inline Vector1 &copySpecialized (Vector1 &res, const Vector2 &v,
						 VectorCategories::DenseVectorTag,
						 VectorCategories::DenseVectorTag) const
		{ std::copy (v.begin (), v.end (), res.begin ()); return res;
		}
		template <class Vector1, class Vector2>
		Vector1 &copySpecialized (Vector1 &res, const Vector2 &v,
					  VectorCategories::SparseSequenceVectorTag,
					  VectorCategories::DenseVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &copySpecialized (Vector1 &res, const Vector2 &v,
					  VectorCategories::SparseAssociativeVectorTag,
					  VectorCategories::DenseVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &copySpecialized (Vector1 &res, const Vector2 &v,
					  VectorCategories::SparseParallelVectorTag,
					  VectorCategories::DenseVectorTag) const;

		template <class Vector1, class Vector2>
		Vector1 &copySpecialized (Vector1 &res, const Vector2 &v,
					  VectorCategories::DenseVectorTag,
					  VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector1, class Vector2>
		inline Vector1 &copySpecialized (Vector1 &res, const Vector2 &v,
						 VectorCategories::SparseSequenceVectorTag,
						 VectorCategories::SparseSequenceVectorTag) const
		{
			res.resize (v.size ()); std::copy (v.begin (), v.end (), res.begin ()); return res;
		}
		template <class Vector1, class Vector2>
		Vector1 &copySpecialized (Vector1 &res, const Vector2 &v,
					  VectorCategories::SparseAssociativeVectorTag,
					  VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &copySpecialized (Vector1 &res, const Vector2 &v,
					  VectorCategories::SparseParallelVectorTag,
					  VectorCategories::SparseSequenceVectorTag) const;

		template <class Vector1, class Vector2>
		Vector1 &copySpecialized (Vector1 &res, const Vector2 &v,
					  VectorCategories::DenseVectorTag,
					  VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &copySpecialized (Vector1 &res, const Vector2 &v,
					  VectorCategories::SparseSequenceVectorTag,
					  VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &copySpecialized (Vector1 &res, const Vector2 &v,
					  VectorCategories::SparseAssociativeVectorTag,
					  VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &copySpecialized (Vector1 &res, const Vector2 &v,
					  VectorCategories::SparseParallelVectorTag,
					  VectorCategories::SparseAssociativeVectorTag) const;

		template <class Vector1, class Vector2>
		Vector1 &copySpecialized (Vector1 &res, const Vector2 &v,
					  VectorCategories::DenseVectorTag,
					  VectorCategories::SparseParallelVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &copySpecialized (Vector1 &res, const Vector2 &v,
					  VectorCategories::SparseSequenceVectorTag,
					  VectorCategories::SparseParallelVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &copySpecialized (Vector1 &res, const Vector2 &v,
					  VectorCategories::SparseAssociativeVectorTag,
					  VectorCategories::SparseParallelVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &copySpecialized (Vector1 &res, const Vector2 &v,
					  VectorCategories::SparseParallelVectorTag,
					  VectorCategories::SparseParallelVectorTag) const;

		template <class Vector1, class Vector2>
		Vector1 &copySpecialized (Vector1 &res, const Vector2 &v, size_t i, size_t len,
					  VectorCategories::DenseVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &copySpecialized (Vector1 &res, const Vector2 &v, size_t i, size_t len,
					  VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &copySpecialized (Vector1 &res, const Vector2 &v, size_t i, size_t len,
					  VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &copySpecialized (Vector1 &res, const Vector2 &v, size_t i, size_t len,
					  VectorCategories::SparseParallelVectorTag) const;

		// These versions are optimized for the case where one is
		// copying between vectors of the same type. It avoids
		// additional memory allocation and copying.
		template <class Vector>
		Vector &copySpecialized (Vector &res, const Vector &v, size_t i, size_t len,
					 VectorCategories::DenseVectorTag) const;
		template <class Vector>
		Vector &copySpecialized (Vector &res, const Vector &v, size_t i, size_t len,
					 VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector>
		Vector &copySpecialized (Vector &res, const Vector &v, size_t i, size_t len,
					 VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector>
		Vector &copySpecialized (Vector &res, const Vector &v, size_t i, size_t len,
					 VectorCategories::SparseParallelVectorTag) const;

		template <class Vector1, class Vector2>
		inline Element &dotSpecialized (Element &res, const Vector1 &v1, const Vector2 &v2,
						VectorCategories::DenseVectorTag,
						VectorCategories::DenseVectorTag) const
		{
			return DotProductDomain<Field>:: dotSpecializedDD (res, v1, v2);
		}

		template <class Vector1, class Vector2>
		inline Element &dotSpecialized (Element &res, const Vector1 &v1, const Vector2 &v2,
						VectorCategories::SparseSequenceVectorTag,
						VectorCategories::DenseVectorTag) const;
		template <class Vector1, class Vector2>
		inline Element &dotSpecialized (Element &res, const Vector1 &v1, const Vector2 &v2,
						VectorCategories::SparseAssociativeVectorTag,
						VectorCategories::DenseVectorTag) const;
		template <class Vector1, class Vector2>
		inline Element &dotSpecialized (Element &res, const Vector1 &v1, const Vector2 &v2,
						VectorCategories::SparseParallelVectorTag,
						VectorCategories::DenseVectorTag) const
		{
			return DotProductDomain<Field>::dotSpecializedDSP (res, v1, v2);
		}

		template <class Vector1, class Vector2>
		inline Element &dotSpecialized (Element &res, const Vector1 &v1, const Vector2 &v2,
						VectorCategories::DenseVectorTag,
						VectorCategories::SparseSequenceVectorTag) const
		{
			return dot (res, v2, v1);
		}
		template <class Vector1, class Vector2>
		inline Element &dotSpecialized (Element &res, const Vector1 &v1, const Vector2 &v2,
						VectorCategories::SparseSequenceVectorTag,
						VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector1, class Vector2>
		inline Element &dotSpecialized (Element &res, const Vector1 &v1, const Vector2 &v2,
						VectorCategories::SparseAssociativeVectorTag,
						VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector1, class Vector2>
		inline Element &dotSpecialized (Element &res, const Vector1 &v1, const Vector2 &v2,
						VectorCategories::SparseParallelVectorTag,
						VectorCategories::SparseSequenceVectorTag) const;

		template <class Vector1, class Vector2>
		inline Element &dotSpecialized (Element &res, const Vector1 &v1, const Vector2 &v2,
						VectorCategories::DenseVectorTag,
						VectorCategories::SparseAssociativeVectorTag) const
		{
			return dot (res, v2, v1);
		}
		template <class Vector1, class Vector2>
		inline Element &dotSpecialized (Element &res, const Vector1 &v1, const Vector2 &v2,
						VectorCategories::SparseSequenceVectorTag,
						VectorCategories::SparseAssociativeVectorTag) const
		{
			return dot (res, v2, v1);
		}
		template <class Vector1, class Vector2>
		inline Element &dotSpecialized (Element &res, const Vector1 &v1, const Vector2 &v2,
						VectorCategories::SparseAssociativeVectorTag,
						VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector1, class Vector2>
		inline Element &dotSpecialized (Element &res, const Vector1 &v1, const Vector2 &v2,
						VectorCategories::SparseParallelVectorTag,
						VectorCategories::SparseAssociativeVectorTag) const;

		template <class Vector1, class Vector2>
		inline Element &dotSpecialized (Element &res, const Vector1 &v1, const Vector2 &v2,
						VectorCategories::DenseVectorTag,
						VectorCategories::SparseParallelVectorTag) const
		{
			return dot (res, v2, v1);
		}
		template <class Vector1, class Vector2>
		inline Element &dotSpecialized (Element &res, const Vector1 &v1, const Vector2 &v2,
						VectorCategories::SparseSequenceVectorTag,
						VectorCategories::SparseParallelVectorTag) const
		{
			return dot (res, v2, v1);
		}
		template <class Vector1, class Vector2>
		inline Element &dotSpecialized (Element &res, const Vector1 &v1, const Vector2 &v2,
						VectorCategories::SparseAssociativeVectorTag,
						VectorCategories::SparseParallelVectorTag) const
		{
			return dot (res, v2, v1);
		}
		template <class Vector1, class Vector2>
		inline Element &dotSpecialized (Element &res, const Vector1 &v1, const Vector2 &v2,
						VectorCategories::SparseParallelVectorTag,
						VectorCategories::SparseParallelVectorTag) const;

		template <class Vector1, class Vector2, class Vector3>
		Vector1 &addSpecialized (Vector1 &res, const Vector2 &y, const Vector3 &x,
					 VectorCategories::DenseVectorTag,
					 VectorCategories::DenseVectorTag,
					 VectorCategories::DenseVectorTag) const;
		template <class Vector1, class Vector2, class Vector3>
		Vector1 &addSpecialized (Vector1 &res, const Vector2 &y, const Vector3 &x,
					 VectorCategories::SparseSequenceVectorTag,
					 VectorCategories::SparseSequenceVectorTag,
					 VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector1, class Vector2, class Vector3>
		Vector1 &addSpecialized (Vector1 &res, const Vector2 &y, const Vector3 &x,
					 VectorCategories::SparseAssociativeVectorTag,
					 VectorCategories::SparseAssociativeVectorTag,
					 VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector1, class Vector2, class Vector3>
		Vector1 &addSpecialized (Vector1 &res, const Vector2 &y, const Vector3 &x,
					 VectorCategories::SparseParallelVectorTag,
					 VectorCategories::SparseParallelVectorTag,
					 VectorCategories::SparseParallelVectorTag) const;

		template <class Vector1, class Vector2>
		Vector1 &addinSpecialized (Vector1 &y, const Vector2 &x,
					   VectorCategories::DenseVectorTag,
					   VectorCategories::DenseVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &addinSpecialized (Vector1 &y, const Vector2 &x,
					   VectorCategories::SparseSequenceVectorTag,
					   VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &addinSpecialized (Vector1 &y, const Vector2 &x,
					   VectorCategories::SparseAssociativeVectorTag,
					   VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &addinSpecialized (Vector1 &y, const Vector2 &x,
					   VectorCategories::SparseParallelVectorTag,
					   VectorCategories::SparseParallelVectorTag) const;

		template <class Vector1, class Vector2, class Vector3>
		Vector1 &subSpecialized (Vector1 &res, const Vector2 &y, const Vector3 &x,
					 VectorCategories::DenseVectorTag,
					 VectorCategories::DenseVectorTag,
					 VectorCategories::DenseVectorTag) const;
		template <class Vector1, class Vector2, class Vector3>
		Vector1 &subSpecialized (Vector1 &res, const Vector2 &y, const Vector3 &x,
					 VectorCategories::SparseSequenceVectorTag,
					 VectorCategories::SparseSequenceVectorTag,
					 VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector1, class Vector2, class Vector3>
		Vector1 &subSpecialized (Vector1 &res, const Vector2 &y, const Vector3 &x,
					 VectorCategories::SparseAssociativeVectorTag,
					 VectorCategories::SparseAssociativeVectorTag,
					 VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector1, class Vector2, class Vector3>
		Vector1 &subSpecialized (Vector1 &res, const Vector2 &y, const Vector3 &x,
					 VectorCategories::SparseParallelVectorTag,
					 VectorCategories::SparseParallelVectorTag,
					 VectorCategories::SparseParallelVectorTag) const;

		template <class Vector1, class Vector2>
		Vector1 &subinSpecialized (Vector1 &y, const Vector2 &x,
					   VectorCategories::DenseVectorTag,
					   VectorCategories::DenseVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &subinSpecialized (Vector1 &y, const Vector2 &x,
					   VectorCategories::SparseSequenceVectorTag,
					   VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &subinSpecialized (Vector1 &y, const Vector2 &x,
					   VectorCategories::SparseAssociativeVectorTag,
					   VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &subinSpecialized (Vector1 &y, const Vector2 &x,
					   VectorCategories::SparseParallelVectorTag,
					   VectorCategories::SparseParallelVectorTag) const;

		template <class Vector1, class Vector2>
		Vector1 &negSpecialized (Vector1 &res, const Vector2 &x,
					 VectorCategories::DenseVectorTag,
					 VectorCategories::DenseVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &negSpecialized (Vector1 &res, const Vector2 &x,
					 VectorCategories::SparseSequenceVectorTag,
					 VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &negSpecialized (Vector1 &res, const Vector2 &x,
					 VectorCategories::SparseAssociativeVectorTag,
					 VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &negSpecialized (Vector1 &res, const Vector2 &x,
					 VectorCategories::SparseParallelVectorTag,
					 VectorCategories::SparseParallelVectorTag) const;

		template <class Vector>
		Vector &neginSpecialized (Vector &y,
					  VectorCategories::DenseVectorTag) const;
		template <class Vector>
		Vector &neginSpecialized (Vector &y,
					  VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector>
		Vector &neginSpecialized (Vector &y,
					  VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector>
		Vector &neginSpecialized (Vector &y,
					  VectorCategories::SparseParallelVectorTag) const;

		template <class Vector1, class Vector2>
		Vector1 &mulSpecialized (Vector1 &res, const Vector2 &x, const Element &a,
					 VectorCategories::DenseVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &mulSpecialized (Vector1 &res, const Vector2 &x, const Element &a,
					 VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &mulSpecialized (Vector1 &res, const Vector2 &x, const Element &a,
					 VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &mulSpecialized (Vector1 &res, const Vector2 &x, const Element &a,
					 VectorCategories::SparseParallelVectorTag) const;

		template <class Vector>
		Vector &mulinSpecialized (Vector &x, const Element &a,
					  VectorCategories::DenseVectorTag) const;
		template <class Vector>
		Vector &mulinSpecialized (Vector &x, const Element &a,
					  VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector>
		Vector &mulinSpecialized (Vector &x, const Element &a,
					  VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector>
		Vector &mulinSpecialized (Vector &x, const Element &a,
					  VectorCategories::SparseParallelVectorTag) const;

		template <class Vector1, class Vector2, class Vector3>
		Vector1 &axpySpecialized (Vector1 &res, const Vector2 &y,
					  const Element &a, const Vector3 &x,
					  VectorCategories::DenseVectorTag) const;
		template <class Vector1, class Vector2, class Vector3>
		Vector1 &axpySpecialized (Vector1 &res, const Vector2 &y,
					  const Element &a, const Vector3 &x,
					  VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector1, class Vector2, class Vector3>
		Vector1 &axpySpecialized (Vector1 &res, const Vector2 &y,
					  const Element &a, const Vector3 &x,
					  VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector1, class Vector2, class Vector3>
		Vector1 &axpySpecialized (Vector1 &res, const Vector2 &y,
					  const Element &a, const Vector3 &x,
					  VectorCategories::SparseParallelVectorTag) const;

		template <class Vector1, class Vector2>
		Vector1 &axpyinSpecialized (Vector1 &y, const Element &a, const Vector2 &x,
					    VectorCategories::DenseVectorTag,
					    VectorCategories::DenseVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &axpyinSpecialized (Vector1 &y, const Element &a, const Vector2 &x,
					    VectorCategories::DenseVectorTag,
					    VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &axpyinSpecialized (Vector1 &y, const Element &a, const Vector2 &x,
					    VectorCategories::DenseVectorTag,
					    VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &axpyinSpecialized (Vector1 &y, const Element &a, const Vector2 &x,
					    VectorCategories::DenseVectorTag,
					    VectorCategories::SparseParallelVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &axpyinSpecialized (Vector1 &y, const Element &a, const Vector2 &x,
					    VectorCategories::SparseSequenceVectorTag,
					    VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &axpyinSpecialized (Vector1 &y, const Element &a, const Vector2 &x,
					    VectorCategories::SparseAssociativeVectorTag,
					    VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector1, class Vector2>
		Vector1 &axpyinSpecialized (Vector1 &y, const Element &a, const Vector2 &x,
					    VectorCategories::SparseParallelVectorTag,
					    VectorCategories::SparseParallelVectorTag) const;

		// Specializations for the case where the two vectors are of
		// different representations. This is provided for the benefit
		// of MatrixDomain

		template <class Vector1, class Vector2, class Vector3>
		inline Vector1 &addSpecialized (Vector1 &res, const Vector2 &y, const Vector3 &x,
						VectorCategories::DenseVectorTag,
						VectorCategories::DenseVectorTag,
						VectorCategories::GenericVectorTag) const
		{
			typename LinBox::Vector<Field>::Dense v (res.size ());

			copy (v, x);
			add (res, y, v);

			return res;
		}

		template <class Vector1, class Vector2, class Vector3>
		inline Vector1 &addSpecialized (Vector1 &res, const Vector2 &y, const Vector3 &x,
						VectorCategories::DenseVectorTag,
						VectorCategories::GenericVectorTag,
						VectorCategories::DenseVectorTag) const
		{
			typename LinBox::Vector<Field>::Dense v (res.size ());

			copy (v, y);
			add (res, v, x);

			return res;
		}

		template <class Vector1, class Vector2, class Vector3>
		inline Vector1 &addSpecialized (Vector1 &res, const Vector2 &y, const Vector3 &x,
						VectorCategories::DenseVectorTag,
						VectorCategories::GenericVectorTag,
						VectorCategories::GenericVectorTag) const
		{
			typename LinBox::Vector<Field>::Dense v (res.size ());
			typename LinBox::Vector<Field>::Dense w (res.size ());

			copy (v, x);
			copy (w, y);
			add (res, w, v);

			return res;
		}

		template <class Vector1, class Vector2, class Vector3>
		inline Vector1 &addSpecialized (Vector1 &res, const Vector2 &y, const Vector3 &x,
						VectorCategories::GenericVectorTag,
						VectorCategories::GenericVectorTag,
						VectorCategories::GenericVectorTag) const
		{
			typename LinBox::Vector<Field>::Sparse v;
			typename LinBox::Vector<Field>::Sparse w;
			typename LinBox::Vector<Field>::Sparse u;

			copy (v, x);
			copy (w, y);
			add (u, w, v);
			copy (res, u);

			return res;
		}

		template <class Vector1, class Vector2>
		inline Vector1 &addinSpecialized (Vector1 &y, const Vector2 &x,
						  VectorCategories::DenseVectorTag,
						  VectorCategories::GenericVectorTag) const
		{
			typename LinBox::Vector<Field>::Dense v (y.size ());

			copy (v, x);
			addin (y, v);

			return y;
		}

		template <class Vector1, class Vector2>
		inline Vector1 &addinSpecialized (Vector1 &y, const Vector2 &x,
						  VectorCategories::GenericVectorTag,
						  VectorCategories::GenericVectorTag) const
		{
			typename LinBox::Vector<Field>::Sparse v;
			typename LinBox::Vector<Field>::Sparse w;

			copy (v, x);
			addin (w, v);
			copy (y, w);

			return y;
		}

		template <class Vector1, class Vector2, class Vector3>
		inline Vector1 &subSpecialized (Vector1 &res, const Vector2 &y, const Vector3 &x,
						VectorCategories::DenseVectorTag,
						VectorCategories::DenseVectorTag,
						VectorCategories::GenericVectorTag) const
		{
			typename LinBox::Vector<Field>::Dense v (res.size ());

			copy (v, x);
			sub (res, y, v);

			return res;
		}

		template <class Vector1, class Vector2, class Vector3>
		inline Vector1 &subSpecialized (Vector1 &res, const Vector2 &y, const Vector3 &x,
						VectorCategories::DenseVectorTag,
						VectorCategories::GenericVectorTag,
						VectorCategories::DenseVectorTag) const
		{
			typename LinBox::Vector<Field>::Dense v (res.size ());

			copy (v, y);
			sub (res, v, x);

			return res;
		}

		template <class Vector1, class Vector2, class Vector3>
		inline Vector1 &subSpecialized (Vector1 &res, const Vector2 &y, const Vector3 &x,
						VectorCategories::DenseVectorTag,
						VectorCategories::GenericVectorTag,
						VectorCategories::GenericVectorTag) const
		{
			typename LinBox::Vector<Field>::Dense v (res.size ());
			typename LinBox::Vector<Field>::Dense w (res.size ());

			copy (v, x);
			copy (w, y);
			sub (res, w, v);

			return res;
		}

		template <class Vector1, class Vector2, class Vector3>
		inline Vector1 &subSpecialized (Vector1 &res, const Vector2 &y, const Vector3 &x,
						VectorCategories::GenericVectorTag,
						VectorCategories::GenericVectorTag,
						VectorCategories::GenericVectorTag) const
		{
			typename LinBox::Vector<Field>::Sparse v;
			typename LinBox::Vector<Field>::Sparse w;
			typename LinBox::Vector<Field>::Sparse u;

			copy (v, x);
			copy (w, y);
			sub (u, w, v);
			copy (res, u);

			return res;
		}

		template <class Vector1, class Vector2>
		inline Vector1 &subinSpecialized (Vector1 &y, const Vector2 &x,
						  VectorCategories::DenseVectorTag,
						  VectorCategories::GenericVectorTag) const
		{
			typename LinBox::Vector<Field>::Dense v (y.size ());

			copy (v, x);
			subin (y, v);

			return y;
		}

		template <class Vector1, class Vector2>
		inline Vector1 &subinSpecialized (Vector1 &y, const Vector2 &x,
						  VectorCategories::GenericVectorTag,
						  VectorCategories::GenericVectorTag) const
		{
			typename LinBox::Vector<Field>::Sparse v;
			typename LinBox::Vector<Field>::Sparse w;

			copy (v, x);
			subin (w, v);
			copy (y, w);

			return y;
		}

		template <class Vector1, class Vector2>
		inline Vector1 &negSpecialized (Vector1 &y, const Vector2 &x,
						VectorCategories::DenseVectorTag,
						VectorCategories::GenericVectorTag) const
		{
			typename LinBox::Vector<Field>::Dense v (y.size ());

			copy (v, x);
			neg (y, v);

			return y;
		}

		template <class Vector1, class Vector2>
		inline Vector1 &negSpecialized (Vector1 &y, const Vector2 &x,
						VectorCategories::GenericVectorTag,
						VectorCategories::GenericVectorTag) const
		{
			typename LinBox::Vector<Field>::Sparse v;
			typename LinBox::Vector<Field>::Sparse w;

			copy (v, x);
			neg (w, v);
			copy (y, w);

			return y;
		}

		template <class Vector1, class Vector2>
		inline Vector1 &axpyinSpecialized (Vector1 &y, const Element &a, const Vector2 &x,
						   VectorCategories::GenericVectorTag,
						   VectorCategories::GenericVectorTag) const
		{
			typename LinBox::Vector<Field>::Sparse v;
			typename LinBox::Vector<Field>::Sparse w;

			copy (v, x);
			axpyin (w, a, v);
			copy (y, w);

			return y;
		}

		template<class Vector>
		inline void swapSpecialized (Vector &v1, Vector &v2,
					     VectorCategories::DenseVectorTag) const;

		template<class _Vector> // BB : nvcc not happy with class Vector (and I agree :))
		inline void swapSpecialized (_Vector &v1, _Vector &v2,
					     VectorCategories::SparseSequenceVectorTag) const
		{
			typename LinBox::Vector<Field>::SparseSeq t;
			t = v1; v1 = v2; v2 = t;
		}

		template <class _Vector>
		inline void swapSpecialized (_Vector &v1, _Vector &v2,
					     VectorCategories::SparseAssociativeVectorTag) const
		{
			typename LinBox::Vector<Field>::SparseMap t;
			t = v1; v1 = v2; v2 = t;
		}

		template <class _Vector>
		inline void swapSpecialized (_Vector &v1, _Vector &v2,
					     VectorCategories::SparseParallelVectorTag) const
		{
			typename LinBox::Vector<Field>::SparsePar t;
			t = v1; v1 = v2; v2 = t;
		}

		template <class Vector, class Iterator>
		inline Vector &permuteSpecialized (Vector   &v,
						   Iterator  P_start,
						   Iterator  P_end,
						   VectorCategories::DenseVectorTag) const;
		template <class Vector, class Iterator>
		inline Vector &permuteSpecialized (Vector   &v,
						   Iterator  P_start,
						   Iterator  P_end,
						   VectorCategories::SparseSequenceVectorTag) const;
		template <class Vector, class Iterator>
		inline Vector &permuteSpecialized (Vector   &v,
						   Iterator  P_start,
						   Iterator  P_end,
						   VectorCategories::SparseAssociativeVectorTag) const;
		template <class Vector, class Iterator>
		inline Vector &permuteSpecialized (Vector   &v,
						   Iterator  P_start,
						   Iterator  P_end,
						   VectorCategories::SparseParallelVectorTag) const;
	}; // class VectorDomain

} // namespace LinBox


#endif // __LINBOX_field_vector_domain_H

#include "linbox/vector/vector-domain.inl"
#include "linbox/vector/vector-domain-gf2.h"


// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
