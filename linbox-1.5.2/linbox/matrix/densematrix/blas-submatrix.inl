/*
 * Copyright (C) 2004 Pascal Giorgi, Clément Pernet
 *               2013, 2014 the LinBox group
 *
 * Written by :
 *               Pascal Giorgi  <pascal.giorgi@ens-lyon.fr>
 *               Clément Pernet <clement.pernet@imag.fr>
 *               Brice Boyer (briceboyer) <boyer.brice@gmail.com>
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

/*!@internal
 * @file matrix/densematrix/blas-matrix.inl
 * @ingroup densematrix
 * A \c BlasMatrix<\c _Field > represents a matrix as an array of
 * <code>_Field</code>s.
 */

#ifndef __LINBOX_densematrix_blas_submatrix_INL
#define __LINBOX_densematrix_blas_submatrix_INL

/////////////////
//   PRIVATE   //
/////////////////

namespace LinBox
{
}

//////////////////
// CONSTRUCTORS //
//////////////////

namespace LinBox
{
#if 0
	template < class _Matrix >
	BlasSubmatrix<_Matrix>::BlasSubmatrix () :
			_Mat(NULL),_row(0),_col(0),_r0(0),_c0(0),_stride(0),_off(0)
		{
		}
#endif

	template < class _Matrix >
	BlasSubmatrix<_Matrix>::BlasSubmatrix (typename BlasSubmatrix<_Matrix>::matrixType &Mat,
						size_t row,
						size_t col,
						size_t Rowdim,
						size_t Coldim) :
		_Mat (Mat),
		_row (Rowdim), _col(Coldim),
		_r0(row),_c0(col),
		_stride(Mat.coldim()),_off(row*_stride+col)
		,_AD(Mat.field())
		,_VD(Mat.field())
	{
		// std::cout << "sub cstor 0 called" << std::endl;
		linbox_check ( _r0  <= Mat.rowdim() ); // allow for NULL matrix
		linbox_check ( _c0  <= Mat.coldim() );
		// linbox_check ( _row <= Mat.rowdim() );
		linbox_check ( _off <= Mat.rowdim()*Mat.coldim() );
		linbox_check ( _col <= Mat.coldim() );
	}

	template < class _Matrix >
	BlasSubmatrix<_Matrix>::BlasSubmatrix (typename BlasSubmatrix<_Matrix>::constMatrixType &Mat,
						size_t row,
						size_t col,
						size_t Rowdim,
						size_t Coldim) :
		_Mat (Mat),
		_row (Rowdim), _col(Coldim),
		_r0(row),_c0(col),
		_stride(Mat.coldim()),_off(row*_stride+col)
		,_AD(Mat.field())
		,_VD(Mat.field())
	{
		// std::cout << "sub const cstor 0 called" << std::endl;
		linbox_check ( _r0  <= Mat.rowdim() ); // allow for NULL matrix
		linbox_check ( _c0  <= Mat.coldim() );
		// linbox_check ( _row <= Mat.rowdim() );
		linbox_check ( _off <= Mat.rowdim()*Mat.coldim() );
		linbox_check ( _col <= Mat.coldim() );
	}


	template < class _Matrix >
	BlasSubmatrix<_Matrix>::BlasSubmatrix (typename BlasSubmatrix<_Matrix>::matrixType &Mat) :
		_Mat (Mat),
		_row(Mat.rowdim()), _col(Mat.coldim()),
		_r0(0), _c0(0),
		_stride(Mat.coldim()),_off(0)
		,_AD(Mat.field())
		,_VD(Mat.field())
	{
		// std::cout << "sub cstor 2 called" << std::endl;
	}

	template < class _Matrix >
	BlasSubmatrix<_Matrix>::BlasSubmatrix (typename BlasSubmatrix<_Matrix>::constMatrixType &Mat) :
		_Mat (Mat),
		_row(Mat.rowdim()), _col(Mat.coldim()),
		_r0(0), _c0(0),
		_stride(Mat.coldim()),_off(0)
		,_AD(Mat.field())
		,_VD(Mat.field())
	{
		// std::cout << "sub const cstor 2 called" << std::endl;
	}

	template < class _Matrix >
	BlasSubmatrix<_Matrix>::BlasSubmatrix (typename BlasSubmatrix<_Matrix>::constSubMatrixType &SM,
						size_t row,
						size_t col,
						size_t Rowdim,
						size_t Coldim) :
		_Mat (SM._Mat),
		_row ( Rowdim ),      _col ( Coldim ) ,
		_r0  ( SM._r0 + row ), _c0 ( SM._c0 + col ),
		_stride(SM._stride),_off(SM._off+(row*_stride+col))
		,_AD(SM.field())
		,_VD(SM.field())
	{
		// std::cout << "sub const cstor 3 called" << std::endl;
		linbox_check ( _r0  <= SM._Mat.rowdim() ); // allow for NULL matrix
		linbox_check ( _c0  <= SM._stride );
		linbox_check ( _row <= SM._Mat.rowdim() );
		linbox_check ( _col <= SM._stride );
		linbox_check ( _off <= SM._Mat.rowdim()*(SM._Mat.coldim()+1) );
	}


	template < class _Matrix >
	BlasSubmatrix<_Matrix>::BlasSubmatrix (typename BlasSubmatrix<_Matrix>::subMatrixType &SM,
						size_t row,
						size_t col,
						size_t Rowdim,
						size_t Coldim) :
		_Mat (SM._Mat),
		_row ( Rowdim ),      _col ( Coldim ) ,
		_r0  ( SM._r0 + row ), _c0 ( SM._c0 + col ),
		_stride(SM._stride),_off(SM._off+(row*_stride+col))
		,_AD(SM.field())
		,_VD(SM.field())
	{
		// std::cout << "sub cstor 3 called" << std::endl;
		linbox_check ( _r0  <= SM._Mat.rowdim() ); // allow for NULL matrix
		linbox_check ( _c0  <= SM._stride );
		linbox_check ( _row <= SM._Mat.rowdim() );
		linbox_check ( _col <= SM._stride );
		linbox_check ( _off <= SM._Mat.rowdim()*(SM._Mat.coldim()+1) );
	}

	template < class _Matrix >
	BlasSubmatrix<_Matrix>::BlasSubmatrix (typename BlasSubmatrix<_Matrix>::subMatrixType &SM) :
		_Mat (SM._Mat),
		_row ( SM._row ),  _col ( SM._col ) ,
		_r0  ( SM._r0 ),    _c0 (SM._c0 ),
		_stride(SM._stride),_off(SM._off)
		,_AD(SM.field())
		,_VD(SM.field())
	{
		// std::cout << "sub cstor 4 called" << std::endl;
		linbox_check ( _r0  <=  SM._Mat.rowdim() ); // allow for NULL matrix
		linbox_check ( _c0  <=  SM._stride );
		linbox_check ( _row <= SM._Mat.rowdim() );
		linbox_check ( _col <= SM._stride );
		linbox_check ( _off <= SM._Mat.rowdim()*(SM._Mat.coldim()+1) );
	}

	template < class _Matrix >
	BlasSubmatrix<_Matrix>::BlasSubmatrix (typename BlasSubmatrix<_Matrix>::constSubMatrixType &SM) :
		_Mat (SM._Mat),
		_row ( SM._row ),  _col ( SM._col ) ,
		_r0  ( SM._r0 ),    _c0 (SM._c0 ),
		_stride(SM._stride),_off(SM._off)
		,_AD(SM.field())
		,_VD(SM.field())
	{
		// std::cout << "sub const cstor 4 called" << std::endl;
		linbox_check ( _r0  <=  SM._Mat.rowdim() ); // allow for NULL matrix
		linbox_check ( _c0  <=  SM._stride );
		linbox_check ( _row <= SM._Mat.rowdim() );
		linbox_check ( _col <= SM._stride );
		linbox_check ( _off <= SM._Mat.rowdim()*(SM._Mat.coldim()+1) );
	}

	// shallow copy
	template < class _Matrix >
	BlasSubmatrix<_Matrix>& BlasSubmatrix<_Matrix>::operator=(const BlasSubmatrix<_Matrix> &SM)
	{
		if ( &SM == this)
			return *this ;

		_Mat   = SM._Mat  ;
		_r0  = SM._r0 ;
		_row = SM._row;
		_c0  = SM._c0 ;
		_col = SM._col;
		_stride = SM._stride;
		_off = SM._off ;
		_AD  = SM._AD ;
		_VD  = SM._VD ;

		return *this;
	}

	// function for repurposing Submatrices.
	template < class _Matrix >
	BlasSubmatrix<_Matrix>& BlasSubmatrix<_Matrix>::submatrix(typename BlasSubmatrix<_Matrix>::constSelf_t &SM,
			size_t row, size_t col, size_t Rowdim, size_t Coldim)
	{
		_Mat   = SM._Mat;
		_row = Rowdim; _col = Coldim;
		_r0  = SM._r0 + row; _c0 = SM._c0 + col;
		_stride = SM._stride; _off = SM._off+(row*_stride+col);
		_AD = SM._AD;
		_VD = SM._VD;

		return *this;
	}

	// deep copy
	template < class _Matrix >
	template<class Matrix>
	BlasSubmatrix<_Matrix>& BlasSubmatrix<_Matrix>::copy( const Matrix & B)
	{
		for (size_t i = 0 ; i < rowdim() ; ++i)
		for (size_t j = 0 ; j < coldim() ; ++j) {
			setEntry(i,j,B.getEntry(i,j));
		}
		return *this;
	}

	template < class _Matrix >
	BlasSubmatrix<_Matrix> &BlasSubmatrix<_Matrix>::swap( typename BlasSubmatrix<_Matrix>::Self_t & B)
	{
		Element temp; _Mat.field().init(temp);
		Element hold; _Mat.field().init(hold);
		for (size_t i = 0 ; i < rowdim() ; ++i)
		for (size_t j = 0 ; j < coldim() ; ++j) {
			getEntry(hold,i,j);
			setEntry(i,j,B.getEntry(temp,i,j));
			B.setEntry(i,j,hold);
		}
		return *this;
	}

	template < class _Matrix >
	BlasSubmatrix<_Matrix> &BlasSubmatrix<_Matrix>::zero()
	{
		for (size_t i = 0 ; i < rowdim() ; ++i)
		for (size_t j = 0 ; j < coldim() ; ++j) {
			setEntry(i,j,_Mat.field().zero);
		}
		return *this;
	}

	template < class _Matrix >
	void BlasSubmatrix<_Matrix>::random()
	{
		typename Field::RandIter r(_Mat.field());
		Element temp; _Mat.field().init(temp);
		for (size_t i = 0 ; i < rowdim() ; ++i)
			for (size_t j = 0 ; j < coldim() ; ++j) {
				setEntry(i,j,r.random(temp));
		}
		return ;
	}

} // LinBox


//////////////////
//  DIMENSIONS  //
//////////////////

namespace LinBox
{

	template < class _Matrix >
	size_t BlasSubmatrix<_Matrix>::rowdim() const
	{
		return _row ;
	}

	template < class _Matrix >
	size_t BlasSubmatrix<_Matrix>::coldim() const
	{
		return _col ;
	}

	template < class _Matrix >
	size_t  BlasSubmatrix<_Matrix>::getStride() const
	{
		return _stride;
	}


} // LinBox

//////////////////
//   ELEMENTS   //
//////////////////

namespace LinBox
{


	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::pointer
	BlasSubmatrix<_Matrix>::getPointer() const
	{
		return _Mat.getPointer()+_off;
	}

	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::const_pointer &
	BlasSubmatrix<_Matrix>::getConstPointer() const
	{
		return _Mat.getConstPointer()+_off;
	}


	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::pointer
	BlasSubmatrix<_Matrix>::getWritePointer()
	{
		return (_Mat.getWritePointer()+_off);
	}

	template < class _Matrix >
	void BlasSubmatrix<_Matrix>::setEntry (size_t i, size_t j, const Element &a_ij)
	{
		_Mat.setEntry (_r0 + i, _c0 + j, a_ij);
	}

	template < class _Matrix >
		typename  LinBox::BlasSubmatrix<_Matrix>::Element &BlasSubmatrix<_Matrix>::refEntry (size_t i, size_t j)
	{
		return _Mat.refEntry ( _r0+i, _c0+j );
	}

	template < class _Matrix >
	const typename  LinBox::BlasSubmatrix<_Matrix>::Element &BlasSubmatrix<_Matrix>::getEntry (size_t i, size_t j) const
	{
		return _Mat.getEntry ( _r0+i , _c0+j );
	}

	template < class _Matrix >
typename  LinBox::BlasSubmatrix<_Matrix>::Element &BlasSubmatrix<_Matrix>::getEntry (Element &x, size_t i, size_t j) const
	{
		return _Mat.getEntry (x, _r0+i , _c0+j );
	}

}

///////////////////
// TRANSPOSE &AL //
///////////////////

namespace LinBox
{
}

///////////////////
//   ITERATORS   //
///////////////////

namespace LinBox
{


	/*! Raw Iterators.
	 * @ingroup iterators
	 *
	 * The raw iterator is a method for accessing all entries in the matrix
	 * in some unspecified order. This can be used, e.g. to reduce all
	 * matrix entries modulo a prime before passing the matrix into an
	 * algorithm.
	 */
	template < class _Matrix >
	class BlasSubmatrix<_Matrix>::Iterator {
	public:
		Iterator (){}

		/*! @internal
		 * @brief NO DOC
		 */
		Iterator (const typename BlasMatrix<typename _Matrix::Field, typename _Matrix::Rep>::Iterator& cur,
			     const size_t c_dim,
			     const size_t stride,
			     const size_t c_idx) :
			_cur (cur), _c_dim (c_dim), _stride(stride), _c_idx (c_idx)
		{}

		/*! @internal
		 * @brief copy operator.
		 * @param r Iterator to copy.
		 */
		Iterator& operator = (const Iterator& r)
		{
			_cur    = r._cur;
			_c_dim  = r._c_dim;
			_stride = r._stride;
			_c_idx  = r._c_idx;
			return *this;
		}

		/*! @internal
		 * increment.
		 * ??
		 */
		Iterator& operator ++()
		{
			if (_c_idx < _c_dim - 1){
				++_cur; ++_c_idx;
			}
			else {
				_cur = _cur + _stride - _c_dim + 1;
				_c_idx = 0;
			}

			return *this;
		}

		/*! @internal
		 * increment.
		 * ??
		 */
		Iterator& operator++ (int)
		{
			return this->operator++ ();
		}


		/*! @internal
		 * @brief  operator !=.
		 * @param r Iterator to test inequaltity from.
		 */
		bool operator != (const Iterator& r) const
		{
			return (_cur != r._cur || _c_dim != r._c_dim) || (_stride != r._stride) || (_c_idx != r._c_idx);
		}

		//! @internal operator *.
		typename _Matrix::Element& operator * ()
		{
			return *_cur;
		}

		//! @internal operator *.
		const typename _Matrix::Element& operator * () const
		{
			return *_cur;
		}

	protected:
		typename BlasMatrix<typename _Matrix::Field,typename _Matrix::Rep>::Iterator _cur;
		size_t _c_dim;
		size_t _stride;
		size_t _c_idx;
	};

	/*! Raw Iterators (const version).
	 * @ingroup iterators
	 * The raw iterator is a method for accessing all entries in the matrix
	 * in some unspecified order. This can be used, e.g. to reduce all
	 * matrix entries modulo a prime before passing the matrix into an
	 * algorithm.
	 */
	template < class _Matrix >
	class BlasSubmatrix<_Matrix>::ConstIterator {
	public:
		//! @internal Null constructor
		ConstIterator (){}


		/*! @internal
		 * @brief NO DOC
		 */
		ConstIterator (const typename BlasMatrix<typename _Matrix::Field, typename _Matrix::Rep>::ConstIterator& cur,
				  const size_t c_dim,
				  const size_t stride,
				  const size_t c_idx) :
			_cur (cur), _c_dim (c_dim), _stride(stride), _c_idx (c_idx)
		{}

		/*! @internal
		 * @brief copy operator.
		 * @param r Iterator to copy.
		 */
		ConstIterator& operator = (const ConstIterator& r)
		{
			_cur = r._cur;
			_c_dim = r._c_dim;
			_stride = r._stride;
			_c_idx = r._c_idx;
			return *this;
		}

		/*! @internal
		 * increment.
		 * ??
		 */
		ConstIterator& operator ++()
		{
			if (_c_idx < _c_dim - 1){
				++_cur; ++_c_idx;
			}
			else {
				linbox_check(_stride > _c_dim);
				_cur = _cur + (ptrdiff_t)(_stride - _c_dim + 1);
				_c_idx = 0;
			}

			return *this;
		}

		/*! @internal
		 * increment.
		 * ??
		 */
		ConstIterator& operator++ (int)
		{
			return this->operator++ ();
		}

		/*! @internal
		 * @brief  operator !=.
		 * @param r Iterator to test inequaltity from.
		 */
		bool operator != (const ConstIterator& r) const
		{
			return (_cur != r._cur) || (_c_dim != r._c_dim) || (_stride != r._stride) || (_c_idx != r._c_idx);
		}

		//! @internal operator *.
		const typename BlasSubmatrix<_Matrix>::Element& operator * () const
		{
			return *_cur;
		}

	protected:
		typename BlasMatrix<typename _Matrix::Field, typename _Matrix::Rep>::ConstIterator _cur;
		size_t _c_dim;
		size_t _stride;
		size_t _c_idx;
	};

#if 0
	template < class _Matrix >
	class BlasSubmatrix<_Matrix>::ConstIterator {
	public:
		ConstIterator (){}

		ConstIterator ( const typename BlasMatrix< _Field>::ConstIterator& cur,
				   size_t cont_len,
				   size_t gap_len) :
			_beg (beg), _cur (cur), _cont_len (cont_len), _gap_len (gap_len)
		{}

		ConstIterator& operator = (const Iterator& r)
		{
			_cur = r._cur;
			_beg = r._beg;
			_cont_len = r._cont_len;
			_gap_len = r._gap_len;
			return *this;
		}

		ConstIterator& operator = (const ConstIterator& r)
		{
			_cur = r._cur;
			_beg = r._beg;
			_cont_len = r._cont_len;
			_gap_len = r._gap_len;
			return *this;
		}

		ConstIterator& operator++()
		{
			if (((_cur - _beg + 1) % _cont_len) != 0)
				++_cur;
			else {
				_cur = _cur + _gap_len + 1;
				_beg = _beg + _gap_len + _cont_len;
			}
			return *this;
		}

		ConstIterator operator++(int)
		{
			ConstIterator tmp = *this;
			this->operator++();
			return tmp;
		}

		bool operator != (const ConstIterator& r) const
		{
			return (_cur != r._cur) || (_beg != r._beg) || (_cont_len != r._cont_len) || (_gap_len != r._gap_len);
		}

		const typename _Field::Element& operator*()
		{ return *_cur; }

	 _Field& operator*()
		{ return *_cur; }

		const _Field& operator*() const
		{ return *_cur; }

	protected:
		typename BlasMatrix< _Field>::ConstIterator _beg;
		typename BlasMatrix< _Field>::ConstIterator _cur;
		size_t _cont_len;
		size_t _gap_len;
	};
#endif

	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::Iterator BlasSubmatrix<_Matrix>::Begin ()
	{
		return Iterator (_Mat.Begin () + (ptrdiff_t)( _off ),
				    _col, _stride, 0);
	}

	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::Iterator BlasSubmatrix<_Matrix>::End ()
	{
		return Iterator (_Mat.Begin () +(ptrdiff_t) ( (_row) * _stride + _off ),
				    _col, _stride, 0);
	}

	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::ConstIterator BlasSubmatrix<_Matrix>::Begin () const
	{
		return ConstIterator (_Mat.Begin () +(ptrdiff_t) ( _off ),
					 _col, _stride, 0);
	}

	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::ConstIterator BlasSubmatrix<_Matrix>::End () const
	{
		return ConstIterator (_Mat.Begin () +(ptrdiff_t) ( (_row) * _stride + _off ),
					 _col, _stride, 0);
	}

#if 0
	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::ConstIterator BlasSubmatrix<_Matrix>::Begin () const
	{
		return ConstIterator (_Mat.Begin () +(ptrdiff_t) ( _off ),
					 _Mat.Begin () +(ptrdiff_t) ( _off ),
					 _col, _stride - _col);
	}

	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::ConstIterator BlasSubmatrix<_Matrix>::End () const
	{
		return ConstIterator (_Mat.Begin () +(ptrdiff_t) ( (_row) * _stride + _off ),
					 _Mat.Begin () +(ptrdiff_t) ( (_row) * _stride + _off ),
					 _col, _stride - _col);
	}
#endif

	/*! Raw Indexed Iterator.
	 * @ingroup iterators
	 *
	 * Like the raw iterator, the indexed iterator is a method for
	 * accessing all entries in the matrix in some unspecified order.
	 * At each position of the the indexed iterator, it also provides
	 * the row and column indices of the currently referenced entry.
	 * This is provided through it's \c rowIndex() and \c colIndex() functions.
	 */
	template < class _Matrix >
	class BlasSubmatrix<_Matrix>::IndexedIterator {
	public:
		IndexedIterator (){}

		IndexedIterator (const typename BlasMatrix<typename _Matrix::Field,typename _Matrix::Rep>::Iterator& cur,
				    size_t c_dim,
				    size_t stride,
				    size_t r_idx,
				    size_t c_idx) :
			_cur (cur), _c_dim (c_dim), _stride (stride), _r_idx(r_idx), _c_idx (c_idx)
		{}

		IndexedIterator& operator = (const IndexedIterator& r)
		{
			_cur = r._cur;
			_stride = r._stride;
			_c_dim = r._c_dim;
			_r_idx = r._r_idx;
			_c_idx = r._c_idx;
			return *this;
		}

		IndexedIterator& operator++()
		{
			if (_c_idx < _c_dim - 1){
				++_c_idx;
				++_cur;
			}
			else {
				_cur = _cur + _stride - _c_dim + 1;
				_c_idx = 0;
				++_r_idx;
			}
			return *this;
		}

		IndexedIterator& operator--()
		{
			if (_c_idx > 0){
				--_c_idx;
				--_cur;
			}
			else {
				_cur = _cur - _stride + _c_dim -1;
				_c_idx = 0;
				--_r_idx;
			}
			return *this;
		}

		IndexedIterator operator++(int)
		{
			IndexedIterator tmp = *this;
			this->operator++();
			return tmp;
		}

		IndexedIterator operator--(int)
		{
			IndexedIterator tmp = *this;
			this->operator--();
			return tmp;
		}

		bool operator != (const IndexedIterator& r) const
		{
			return ((_c_idx != r._c_idx) || (_r_idx != r._r_idx) ||(_stride != r._stride) || (_c_dim != r._c_dim) );
		}

		const typename _Matrix::Field& operator*() const {return *_cur;}

		typename _Matrix::Element& operator*() {return *_cur;}

		size_t rowIndex () const { return _r_idx; }

		size_t colIndex () const { return _c_idx; }

		const typename _Matrix::Element& value () const {return *_cur;}

	protected:
		typename BlasMatrix<typename _Matrix::Field, typename _Matrix::Rep>::Iterator _cur;
		size_t _stride;
		size_t _c_dim;
		size_t _r_idx;
		size_t _c_idx;
	};

	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::IndexedIterator BlasSubmatrix<_Matrix>::IndexedBegin ()
	{
		return IndexedIterator (_Mat.Begin () +(ptrdiff_t) ( (_off) ),
					   _col , _stride, 0, 0);
	}

	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::IndexedIterator BlasSubmatrix<_Matrix>::IndexedEnd ()
	{
		return IndexedIterator (_Mat.Begin () +(ptrdiff_t) ( (_row) * _stride + (_col+_off) ),
					   _col, _stride, _row-1, _col-1);
	}

	/*! Raw Indexed Iterator (const version).
	 * @ingroup iterators
	 *
	 * Like the raw iterator, the indexed iterator is a method for
	 * accessing all entries in the matrix in some unspecified order.
	 * At each position of the the indexed iterator, it also provides
	 * the row and column indices of the currently referenced entry.
	 * This is provided through it's \c rowIndex() and \c colIndex() functions.
	 */
	template < class _Matrix >
	class BlasSubmatrix<_Matrix>::ConstIndexedIterator {
	public:
		ConstIndexedIterator (){}

		ConstIndexedIterator (const typename BlasMatrix<typename _Matrix::Field, typename _Matrix::Rep>::ConstIterator& cur,
					 size_t c_dim,
					 size_t stride,
					 size_t r_idx,
					 size_t c_idx) :
			_cur (cur), _stride (stride), _c_dim (c_dim), _r_idx(r_idx), _c_idx (c_idx)
		{}

		ConstIndexedIterator& operator = (const IndexedIterator& r)
		{
			_cur = r._cur;
			_stride = r._stride;
			_c_dim = r._c_dim;
			_r_idx = r._r_idx;
			_c_idx = r._c_idx;
			return *this;
		}

		ConstIndexedIterator& operator = (const ConstIndexedIterator& r)
		{
			_cur = r._cur;
			_stride = r._stride;
			_c_dim = r._c_dim;
			_r_idx = r._r_idx;
			_c_idx = r._c_idx;
			return *this;
		}

		ConstIndexedIterator& operator++()
		{
			if (_c_idx < _c_dim - 1){
				++_c_idx;
				++_cur;
			}
			else {
				_cur = _cur + _stride - _c_dim +1;
				_c_idx = 0;
				++_r_idx;
			}
			return *this;
		}

		IndexedIterator& operator--()
		{
			if (_c_idx > 0){
				--_c_idx;
				--_cur;
			}
			else {
				_cur = _cur - _stride + _c_dim -1;
				_c_idx = 0;
				--_r_idx;
			}
			return *this;
		}

		ConstIndexedIterator operator++(int)
		{
			ConstIndexedIterator tmp = *this;
			this->operator++();
			return tmp;
		}

		ConstIndexedIterator operator--(int)
		{
			ConstIndexedIterator tmp = *this;
			this->operator--();
			return tmp;
		}

		size_t rowIndex () const
		{
			return _r_idx;
		}

		size_t colIndex () const
		{
			return _c_idx;
		}

		bool operator != (const ConstIndexedIterator& r) const
		{
			return ((_c_idx != r._c_idx) || (_r_idx != r._r_idx) ||(_stride != r._stride) || (_c_dim != r._c_dim) );
		}

		const typename _Matrix::Element& operator*() const
		{
			return *_cur;
		}


		friend std::ostream& operator<<(std::ostream& out, const ConstIndexedIterator m)
		{
			return out /* << m._cur << ' ' */
			<< m._stride << ' '
			<< m._c_dim << ' '
			<< m._r_idx << ' '
			<< m._c_idx;
		}

		const typename _Matrix::Element & value() const
		{
			return this->operator*();

		}

	protected:
		typename BlasMatrix<typename _Matrix::Field, typename _Matrix::Rep>::ConstIterator _cur;
		size_t _stride;
		size_t _c_dim;
		size_t _r_idx;
		size_t _c_idx;
	};

	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::ConstIndexedIterator BlasSubmatrix<_Matrix>::IndexedBegin () const
	{
		return ConstIndexedIterator (_Mat.Begin () +(ptrdiff_t) ( _off ),
						_row, _stride, 0, 0);
	}

	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::ConstIndexedIterator BlasSubmatrix<_Matrix>::IndexedEnd () const
	{
		return ConstIndexedIterator (_Mat.Begin () +(ptrdiff_t) ( (_row) * _stride + (_off+_col) ),
						_col, _stride, _row-1, _col-1);
	}

	////////
	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::RowIterator BlasSubmatrix<_Matrix>::rowBegin ()
	{
		return RowIterator (_Mat.Begin () +(ptrdiff_t) ( _off  ),
				    _col, _stride);
	}

	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::RowIterator BlasSubmatrix<_Matrix>::rowEnd ()
	{
		return RowIterator (_Mat.Begin () +(ptrdiff_t) ( (_row) * _stride + _off ),
				    _col, _stride);
	}

	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::ConstRowIterator BlasSubmatrix<_Matrix>::rowBegin () const
	{
		return ConstRowIterator (_Mat.Begin () + (ptrdiff_t)( _off ),
					 _col, _stride);
	}

	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::ConstRowIterator BlasSubmatrix<_Matrix>::rowEnd () const
	{
		return ConstRowIterator (_Mat.Begin () + (ptrdiff_t)( (_row) * _stride + _off ),
					 _col, _stride);
	}

	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::ColIterator BlasSubmatrix<_Matrix>::colBegin ()
	{
		return ColIterator (_Mat.Begin () + (ptrdiff_t)( _off ),
				    _stride, _row);
	}

	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::ColIterator BlasSubmatrix<_Matrix>::colEnd ()
	{
		return ColIterator (_Mat.Begin () + (ptrdiff_t)( (_col) + _off ),
				    _stride, _row);
	}

	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::ConstColIterator BlasSubmatrix<_Matrix>::colBegin () const
	{
		return ConstColIterator (_Mat.Begin () + (ptrdiff_t)( _off ),
					 _stride, _row);
	}

	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::ConstColIterator BlasSubmatrix<_Matrix>::colEnd () const
	{
		return ConstColIterator (_Mat.Begin () + (ptrdiff_t)( (_col) + _off ),
					 _stride, _row);
	}

	/*  operators */
	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::Row BlasSubmatrix<_Matrix>::operator[] (size_t i)
	{
		return Row (_Mat.Begin () +(ptrdiff_t) (_r0+i) * _stride, _Mat.Begin () + (ptrdiff_t)((_r0+i) * _stride + _stride) );
	}

	template < class _Matrix >
	typename BlasSubmatrix<_Matrix>::ConstRow BlasSubmatrix<_Matrix>::operator[] (size_t i) const
	{
		return Row (_Mat.Begin () + (ptrdiff_t)(_r0+i) * _stride, _Mat.Begin () + (ptrdiff_t)((_r0+i) * _stride + _stride) );
	}

} // LinBox

///////////////////
//      I/O      //
///////////////////

namespace LinBox
{

	template < class _Matrix >
	std::istream& BlasSubmatrix<_Matrix>::read (std::istream &file)
	{
#if 0
		Iterator p;
		int m,n;
		char c;
		file>>m>>n>>c;

		if (m*n < _row*_col)
			cerr<<"NOT ENOUGH ELEMENT TO READ\n";
		else {
			for (p = Begin (); p != End (); ++p) {
				integer tmp;
				file>>tmp;cout<<tmp<<endl;
				//file.ignore(1);
				_Mat.field().read (file, *p);
			}
		}
#endif


		Iterator p;
		int m=0,n=0;
		char c='\0';
		file>>m>>n>>c;
		// std::cout << m << 'x' << n << ':' << c << std::endl;

		// this is bogus!! -bds
		_row = m; _col = n;

		// resize(_row,_col);

		if ((c != 'M') && (c != 'm')) {
		for (p = Begin (); p != End (); ++p) {
				//file.ignore(1);
				_Mat.field().read (file, *p);
			}

		}
		else { // sparse file format - needs fixing
			int i=0, j=0;
			while (true)
			{
				file >> i >> j;
				//file.ignore(1);
				//if (! file) break;
				if (i+j <= 0) break;
				// std::cout << i << ',' << j << ':' ;
				_Mat.field().read (file, _Mat.refEntry(i-1, j-1));
			}
		}

		return file;
	}

	template <class _Matrix>
	std::ostream &BlasSubmatrix< _Matrix >::write (std::ostream &os,
						     LINBOX_enum (Tag::FileFormat) f ) const
	{

		ConstRowIterator p;
		switch(f) {
			case (Tag::FileFormat::MatrixMarket ) : /* Matrix Market */
				{
					writeMMArray(os, *this, "BlasSubmatrix");
				}
				break;
			case (Tag::FileFormat::Plain) : /*  raw output */
				{
					integer c;
					int wid;

					_Mat.field().cardinality (c);

					if (c >0)
						wid = (int) ceil (log ((double) c) / M_LN10);
					else {
// 						integer tmp;
// 						size_t max=0;
// 						ConstIterator it = Begin();
// 						for (; it != End(); ++it){
// 							_Mat.field().convert(tmp,*it);
// 							if (tmp.bitsize() > max)
// 								max= tmp.bitsize();
// 						}
// 						wid= (int) ceil ((double)max / M_LN10)+1;
                                            wid=1000;

					}

					for (p = rowBegin (); p != rowEnd ();++p) {
						typename ConstRow::const_iterator pe;

						os << "  [ ";

						for (pe = p->begin (); pe != p->end (); ++pe) {
							os.width (wid);
							/*!  @warning
							 * matrix base does not provide this field(), maybe should?
							 * _Mat.field ().write (os, *pe);
							 * os << *pe;
							 * fixed by using extra field
							 */

							_Mat.field().write (os, *pe);
							os << " ";
						}

						os << "]" << std::endl;
					}
				}
				break;
			case (Tag::FileFormat::Maple) : /*  maple format */
				{

					os << "Matrix( " << rowdim() << ',' << coldim() << ",\n[" ;
					for (p = rowBegin (); p != rowEnd (); ) {
						typename ConstRow::const_iterator pe;
                                                if (p!=rowBegin()) os << ' ';
						os << "[ ";

						for (pe = p->begin (); pe != p->end (); ) {
							_Mat.field().write (os, *pe);
							++pe ;
							if (pe != p->end())
								os << ", ";
						}

						os << "]" ;
						++p ;
						if (p != rowEnd() )
							os << ',' << std::endl;;

					}
					os << "])" ;
				}
				break;
			case (Tag::FileFormat::HTML) : /*  HTML format */
				{

					os << "<table border=\"1\">" ;
					for (p = rowBegin (); p != rowEnd (); ) {
						typename ConstRow::const_iterator pe;

						os << "<tr>";

						for (pe = p->begin (); pe != p->end (); ) {
							_Mat.field().write (os<< "<td>", *pe)<<"</td>";
							++pe ;
						}

						os << "</tr>" << std::endl;
						++p ;

					}
					os << "</table>" ;
				}
				break;
			case (Tag::FileFormat::LaTeX) : /*  LaTex format */
				{
					os << "\\begin{pmatrix} " << std::endl;
					for (p = rowBegin (); p != rowEnd (); ) {
						typename ConstRow::const_iterator pe;

						for (pe = p->begin (); pe != p->end (); ) {
							_Mat.field().write (os, *pe);
							++pe ;
							if (pe != p->end())
								os << "& ";
						}

						os << "\\\\" << std::endl;
						++p ;

					}
					os << "\\end{pmatrix}" ;
				}
				break;
			default : /*  this is an error */
				{
					throw LinBoxError("unknown format to write matrix in");
				}
		}

		return os;
	}

	template <class _Matrix>
	template<typename _Tp1, class _Rep2>
	struct BlasSubmatrix< _Matrix>::rebind {
		typedef BlasMatrix<_Tp1,_Rep2> other;

		void operator() (other & Ap, const Self_t& A) {
			typedef typename BlasSubmatrix<_Matrix>::ConstIterator ConstSelfIterator ;
			typedef typename other::Iterator OtherIterator ;
			OtherIterator    Ap_i;
			ConstSelfIterator A_i;
			Hom<Field, _Tp1> hom(A. field(), Ap. field());
			for (A_i = A. Begin(), Ap_i = Ap.Begin();
			     A_i != A. End(); ++ A_i, ++ Ap_i)
				hom.image (*Ap_i, *A_i);
		}
	};

} // LinBox

//////////////////
//     MISC     //
//////////////////

namespace LinBox
{
} // LinBox
#endif // __LINBOX_densematrix_blas_submatrix_INL

// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

