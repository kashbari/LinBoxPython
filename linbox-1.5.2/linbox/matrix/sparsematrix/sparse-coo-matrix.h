/* linbox/matrix/sparse-csr-matrix.h
 * Copyright (C) 2013 the LinBox
 *
 * Written by :
 * Brice Boyer (briceboyer) <boyer.brice@gmail.com>
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

/*! @file matrix/sparsematrix/sparse-coo-matrix.h
 * @ingroup sparsematrix
 * @brief
 */


#ifndef __LINBOX_matrix_sparsematrix_sparse_coo_matrix_H
#define __LINBOX_matrix_sparsematrix_sparse_coo_matrix_H

#include <utility>
#include <iostream>
#include <algorithm>

#include "linbox/linbox-config.h"
#include "linbox/util/debug.h"
#include "linbox/field/hom.h"
#include "sparse-domain.h"

#ifndef LINBOX_COO_TRANSPOSE
#define LINBOX_COO_TRANSPOSE 1000
#endif

namespace LinBox
{


	/** Sparse matrix, Coordinate storage.
	 *
	 * \ingroup matrix
	 * \ingroup sparse
	 */
	template<class _Field>
	class SparseMatrix<_Field, SparseMatrixFormat::COO> {
	public :
		typedef _Field                             Field ; //!< Field
		typedef typename _Field::Element         Element ; //!< Element
		typedef const Element               constElement ; //!< const Element
		typedef SparseMatrixFormat::COO         Storage ; //!< Matrix Storage Format
		typedef SparseMatrix<_Field,Storage>     Self_t ; //!< Self type
		typedef typename Vector<Field>::SparseSeq    Row ; //!< @warning this is not the row type. Just used for streams.
		// typedef Vector<_Field,VectorStorage::Sparse> Rep ;

		/*! Constructors.
		 * @todo convert from other matrix.
		 *
		 */
		//@{
#if 0 /*  No empty CSTOR */
		SparseMatrix<_Field, SparseMatrixFormat::COO> () :
			_rownb(0),_colnb(0)
			,_nbnz(0)
			,_rowid(0),_colid(0),_data(0)
			, _field()
			, _helper()
		{
		}
#endif

		SparseMatrix<_Field, SparseMatrixFormat::COO> (const _Field & F) :
			_rownb(0),_colnb(0)
			,_nbnz(0)
			,_rowid(0)
			,_colid(0)
			,_data(0)
			, _field(F)
			, _helper()
		{
		}

		SparseMatrix<_Field, SparseMatrixFormat::COO> (const _Field & F, size_t m, size_t n) :
			_rownb(m),_colnb(n)
			,_nbnz(0)
			,_rowid(0)
			,_colid(0)
			,_data(0)
			, _field(F)
			, _helper()
		{
		}

		SparseMatrix<_Field, SparseMatrixFormat::COO> (const _Field & F,
							       size_t m, size_t n,
							       size_t z) :
			_rownb(m),_colnb(n)
			, _nbnz(z)
			,_rowid(z)
			, _colid(z)
			,_data(z)
			, _field(F)
			, _helper()
		{
		}

		SparseMatrix<_Field, SparseMatrixFormat::COO> (const SparseMatrix<_Field, SparseMatrixFormat::COO> & S) :
			_rownb(S._rownb),_colnb(S._colnb)
			,_nbnz(S._nbnz)
			,_rowid(S._rowid)
			, _colid(S._colid)
			,_data(S._data)
			, _field(S._field)
			, _helper()
		{
		}

#if 0
		template<class _OtherField>
		SparseMatrix<_Field, SparseMatrixFormat::COO> (const SparseMatrix<_OtherField, SparseMatrixFormat::COO> & S) :
			_rownb(S._rownb),_colnb(S._colnb),
			_nbnz(S._nbnz),
			_rowid(S._rowid),_colid(S._colid),_data(S._data),
			_field(S._field)
		{}
#endif

		template<typename _Tp1, typename _Rw1 = SparseMatrixFormat::COO>
		struct rebind {
			typedef SparseMatrix<_Tp1, _Rw1> other;
		private:

			template<class _Rw>
			void rebindMethod(SparseMatrix<_Tp1, _Rw> & Ap, const Self_t & A  /*, IndexedCategory::HasNext */)
			{
				typename _Tp1::Element e;
				Hom<typename Self_t::Field, _Tp1> hom(A.field(), Ap.field());

				size_t i, j ;
				Element f ;
				A.firstTriple();
				while ( A.nextTriple(i,j,f) ) {
					linbox_check(i < A.rowdim() && j < A.coldim()) ;
					hom. image ( e, f) ;
					if (! Ap.field().isZero(e) )
						Ap.appendEntry(i,j,e);
				}
				A.firstTriple();
				Ap.finalize();
			}

			void rebindMethod(SparseMatrix<_Tp1, SparseMatrixFormat::COO>  & Ap, const Self_t & A /*,  IndexedCategory::HasNext*/)
			{
				// we don't use nextTriple because we can do better.

				typename _Tp1::Element e;

				Hom<typename Self_t::Field, _Tp1> hom(A.field(), Ap.field());
				size_t j = 0 ;
				for (size_t i = 0 ; i < A.size() ; ++i) {
					hom. image ( e, A.getData(i) );
					if (!Ap.field().isZero(e)) {
						Ap.setTriple(j,A.getRowid(i),A.getColid(i),e);
						++j;
					}
				}


				if (j != Ap.size())
					Ap.resize(j);

			}

		public:

			void operator() (other & Ap, const Self_t& A)
			{
				rebindMethod(Ap, A );

			}

		};

		template<typename _Tp1, typename _Rw1>
		SparseMatrix (const SparseMatrix<_Tp1, _Rw1> &S, const Field& F) :
			_rownb(S.rowdim()),_colnb(S.coldim())
			,_nbnz(S.size())
			, _rowid(S.size())
			, _colid(S.size())
			,_data(S.size())
			, _field(F)
		{
			typename SparseMatrix<_Tp1,_Rw1>::template rebind<Field,Storage>()(*this, S);
			finalize();
		}




		template<class VectStream>
		SparseMatrix<_Field, SparseMatrixFormat::COO> (const _Field & F, VectStream & stream) :
			_rownb(stream.size()),_colnb(stream.dim())
			, _nbnz(0)
			, _rowid(0)
			, _colid(0)
			,_data(0)
			, _field(F)
		{
			{

			}
			for (size_t i = 0 ; i< _rownb ; ++i) {
				typename Vector<Field>::SparseSeq lig_i ;
				stream >> lig_i ;
				for (size_t j = 0 ; j < lig_i.size() ; ++j) {
					size_t nbnz = _nbnz++ ;
					resize(_nbnz);
					_rowid[nbnz] = i ;
					_colid[nbnz] = lig_i[j].first ;
					F.init(_data[nbnz], lig_i[j].second) ; //!@bug may be 0...
				}
			}
		}

		SparseMatrix<_Field, SparseMatrixFormat::COO> ( MatrixStream<Field>& ms ):
			_rownb(0),_colnb(0)
			,_nbnz(0)
			,_rowid(0)
			,_colid(0)
			,_data(0)
			,_field(ms.field())
		{
			firstTriple();

			Element val;
			size_t i, j;
			while( ms.nextTriple(i,j,val) ) {
				if (! field().isZero(val)) {
					if( i >= _rownb ) {
						_rownb = i + 1;
					}
					if( j >= _colnb ) {
						_colnb = j + 1;
					}
					appendEntry(i,j,val);
				}
			}
			if( ms.getError() > END_OF_MATRIX )
				throw ms.reportError(__func__,__LINE__);
			if( !ms.getDimensions( i, j ) )
				throw ms.reportError(__func__,__LINE__);
#ifndef NDEBUG
			if( i != _rownb  || j != _colnb) {
				std::cout << " ***Warning*** the sizes got changed" << __func__ << ',' << __LINE__ << std::endl;
				// _rownb = i;
				// _matA.resize(_m);
			}
#endif

			firstTriple();
			finalize();
		}

		void resize(size_t nn)
		{
#ifndef NDEBUG
			if (nn < _nbnz) {
				std::cerr << "*** Warning *** you are possibly loosing data (COO resize)" << std::endl;
				// could be a commentator()...
			}
#endif
			_rowid.resize(nn);
			_colid.resize(nn);
			_data.resize(nn);
			_nbnz = nn ;
		}

		void resize(const size_t & mm, const size_t & nn, const size_t & zz = 0)
		{
			_rownb = mm ;
			_colnb = nn ;

			resize(zz);
		}

		/*! Default converter.
		 * @param S a sparse matrix in any storage.
		 */
		template<class _OtherStorage>
		SparseMatrix<_Field, SparseMatrixFormat::COO> (const SparseMatrix<_Field, _OtherStorage> & S) :
			_rownb(S._rownb),_colnb(S._colnb),
			_rowid(S.size()),_colid(S.size()),_data(S.size()),
			_field(S._field)
		{
			SparseMatrix<_Field,SparseMatrixFormat::CSR> Temp(_field,_rownb,_colnb) ;
			S.exporte(Temp); // convert S to CSR
			this->importe(Temp); // convert Temp from COO
			finalize();
		}



		//@}
		/*! Conversions.
		 * Any sparse matrix has a converter to/from CSR.
		 * A specialisation can skip the temporary CSR matrix created.
		 */
		//@{
		/*! Import a matrix in CSR format to COO.
		 * @param S CSR matrix to be converted in COO
		 */
		void importe(const SparseMatrix<_Field,SparseMatrixFormat::CSR> &S)
		{
			resize( S.rowdim() , S.coldim() , S.size() );
			_colid = S.getColid();
			_data = S.getData();
			for (size_t i = 0 ; i < _rownb ; ++i)
				for (size_t j = S.getStart(i) ; j < S.getEnd(i); ++j)
					_rowid[j] = i ;

		}

		void importe(const SparseMatrix<_Field,SparseMatrixFormat::COO> &S)
		{
			resize( S.rowdim(), S.coldim(), S.size() );

			setRowid(S.getRowid());
			setColid(S.getColid());
			setData(S.getData());
		}

		/*! Export a matrix in CSR format from COO.
		 * @param S CSR matrix to be converted from COO
		 */
		SparseMatrix<_Field,SparseMatrixFormat::CSR > &
		exporte(SparseMatrix<_Field,SparseMatrixFormat::CSR> &S)
		{
			linbox_check(consistent());

			S.resize(_rownb, _colnb, _nbnz);
			S.setData( _data ) ;
			S.setColid (_colid );

			for(size_t i = 0 ; i <= _rownb ; ++i)
				S.setStart(i,0);

			for (size_t i = 0 ; i < _nbnz ; ++i) {
				size_t idx = _rowid[i]+1 ;
				S.setStart(idx,S.getStart(idx) + 1) ;
			}
			for (size_t i= 0 ; i < _rownb ; ++i)
				S.setStart(i+1, S.getEnd(i)+ S.getStart(i)) ;

			return S ;
		}

		SparseMatrix<_Field,SparseMatrixFormat::COO > &
		exporte(SparseMatrix<_Field,SparseMatrixFormat::COO> &S)
		{
			S.resize(_rownb, _colnb, _nbnz);
			S.setData( _data ) ;
			S.setColid( _colid ) ;
			S.setRowid( _rowid ) ;
			S.finalize();

			return S ;
		}


		//@}

		/*! In place transpose. Not quite...
		*/
		void transposeIn()
		{
			Self_t Temp(*this);
			Temp.transpose(*this);
		}

		/*! Transpose the matrix.
		 *  @param S [out] transpose of self.
		 *  @return a reference to \p S.
		 */
		Self_t &
		transpose(Self_t &S) const
		{
			// outStart
			std::vector<size_t> start (coldim()+1,0);

			for (size_t i = 0 ; i < size() ; ++i)
				start[_colid[i]+1] += 1 ;

			for (size_t i = 0 ; i < coldim() ; ++i)
				start[i+1] += start[i] ;

			// inStart;
			std::vector<size_t> _start (rowdim()+1,0);
			for (size_t i = 0 ; i < size() ; ++i)
				_start[_rowid[i]+1] += 1 ;

			for (size_t i = 0 ; i < rowdim() ; ++i)
				_start[i+1] += _start[i] ;

			S.resize((size_t)_colnb, (size_t)_rownb, (size_t)_nbnz ); // necessary copy to temp, no const ref

			{
				size_t i = 0 ;
				std::vector<size_t> done_col(coldim(),0);
				for (size_t nextlig = 1 ; nextlig <= rowdim() ; ++nextlig) {
					// treating line before nextlig
					while (i < _start[nextlig]){
						size_t cur_place ;
						cur_place = start[_colid[i]] + done_col[_colid[i]] ;
						linbox_check(cur_place < size());
						S._data [ cur_place ] = _data[i] ;
						S._colid[ cur_place ] = nextlig-1 ;
						done_col[ _colid[i] ] += 1 ;
						++i;
					}
				}
			}

			for (size_t i = 0 ; i < coldim() ; ++i)
				for (size_t j = start[i] ; j < start[i+1]; ++j)
					S._rowid[j] = i ;

			S.finalize();

			return S ;
		}

		/*! number of rows.
		 * @return row dimension.
		 */
		size_t rowdim() const
		{
			return _rownb ;
		}

		/*! number of columns.
		 * @return column dimension
		 */
		size_t coldim() const
		{
			return _colnb ;
		}

		/*! Number of non zero elements in the matrix.
		 * or at least the size of the _data if
		 * @return size of the _data.
		 */
		size_t size() const
		{
			// return _data.size();
			return _nbnz ;
		}

		void setSize(const size_t & z)
		{
			_nbnz = z ;
		}


		/** Get a read-only individual entry from the matrix.
		 * @param i Row _colid
		 * @param j Column _colid
		 * @return Const reference to matrix entry
		 */
		constElement &getEntry(const size_t &i, const size_t &j) const
		{
			linbox_check(i<_rownb);
			linbox_check(j<_colnb);
			ptrdiff_t nnz =  _triples.next();

			// linbox_check(nnz < _colid.size());
			// linbox_check(nnz < _rowid.size());
			// linbox_check(nnz < _data.size());

			if ( nnz < (ptrdiff_t)_nbnz && _colid[nnz] == j && i ==_rowid[nnz] ) { /* sort of nextTriple */
				linbox_check(!field().isZero(_data[nnz]));
				return _data[nnz];
			}
			else { /* searching */
				typedef typename std::vector<size_t>::const_iterator myConstIterator ;
				std::pair<myConstIterator,myConstIterator> bounds = std::equal_range (_rowid.begin(), _rowid.end(), i);
				size_t ibeg = bounds.first-_rowid.begin();
				size_t iend = bounds.second-_rowid.begin();
				if (ibeg == iend) {
					return field().zero;
				}
				// element may exist
				myConstIterator beg = _colid.begin() + (ptrdiff_t)ibeg ;
				myConstIterator end = _colid.begin() + (ptrdiff_t)iend ;
				myConstIterator low = std::lower_bound (beg, end, j);
				ibeg = (size_t)(low-_colid.begin());
				// insert
				if ( low == end || j != _colid[ibeg] ) {
					return field().zero;
				}
				// replace
				else {
					_triples._nnz = ibeg ;// just in case it can be used, after all that work...
					return _data[ibeg];
				}
			}

		}

		Element      &getEntry (Element &x, size_t i, size_t j) const
		{
			return x = getEntry (i, j);
		}

		void appendEntry(const size_t &i, const size_t &j, const Element& e)
		{
			linbox_check(i < rowdim());
			linbox_check(j < coldim());

			if (field().isZero(e)) { /* probably already tested */
				return ;
			}

#if 0 /*  reserve is slow */
			if (_nbnz % 10 == 0 ) {
				_rowid.reserve(_nbnz+10);
				_colid.reserve(_nbnz+10);
				_data. reserve(_nbnz+10);
			}
#endif
			_rowid.push_back(i);
			_colid.push_back(j);
			_data .push_back(e);
			++_nbnz ;
			return;

		}

		/// make matrix ready to use after a sequence of setEntry calls.
		void finalize()
		{
			_triples.reset();
		} // end construction after a sequence of setEntry calls.

		/** Set an individual entry.
		 * Setting the entry to 0 will not remove it from the matrix
		 * @param i Row _colid of entry
		 * @param j Column _colid of entry
		 * @param value Value of the new entry
		 * @todo make it faster if i is 0 or m-1 ?
		 * @warning if this is used to build a matrix and this matrix is "well formed",
		 * it can be sped up (no checking that the entry already exists).
		 */
		void setEntry(const size_t &i, const size_t &j, const Element& e
			     )
		{
			linbox_check(i<_rownb);
			linbox_check(j<_colnb);

			if (field().isZero(e)) {
				return clearEntry(i,j);
			}

			// nothing has been done yet
			typedef typename std::vector<size_t>::iterator myIterator ;
			std::pair<myIterator,myIterator> bounds = std::equal_range (_rowid.begin(), _rowid.end(), i);
			size_t ibeg = bounds.first-_rowid.begin();
			size_t iend = bounds.second-_rowid.begin();
			// element does not exist, insert
			if (ibeg == iend) {
				// std::cout << "# insert " << i << ',' << j << ':' << e << std::endl;
				_rowid.insert(_rowid.begin()+ibeg,i);
				_colid.insert(_colid.begin()+ibeg,j);
				_data.insert( _data.begin() +ibeg,e);
				++_nbnz;
				return ;
			}
			// element may exist
			myIterator beg = _colid.begin() + (ptrdiff_t)ibeg ;
			myIterator end = _colid.begin() + (ptrdiff_t)iend ;
			myIterator low = std::lower_bound (beg, end, j);
			ibeg = (size_t)(low-_colid.begin());
			// insert
			if ( low == end || j != _colid[ibeg] ) {
				// std::cout << "# 2 insert " << i << ',' << j << ':' << e << std::endl;
				_rowid.insert(_rowid.begin() + (ptrdiff_t)ibeg,i);
				_colid.insert(_colid.begin() + (ptrdiff_t)ibeg,j);
				_data.insert (_data. begin() + (ptrdiff_t)ibeg,e);
				++_nbnz;
				return ;
			}
			// replace
			else {
				// std::cout << "# replace " << i << ',' << j << ':' << e << std::endl;
				_data[ibeg] = e ;
				return ;
			}
		}

#if 0
		/** Get a writeable reference to an entry in the matrix.
		 * If there is no entry at the position (i, j), then a new entry
		 * with a value of zero is inserted and a reference  to it is
		 * returned.
		 * @param i Row _colid of entry
		 * @param j Column _colid of entry
		 * @return Reference to matrix entry
		 */
		Element &refEntry(const size_t &i, const size_t&j)
		{
			linbox_check(i<_rownb);
			linbox_check(j<_colnb);
			// Could be improved by adding an initial guess j/rowdim*size()
			typedef typename std::vector<size_t>::iterator myIterator ;

			std::pair<myIterator,myIterator> bounds = std::equal_range (_rowid.begin(), _rowid.end(), i);
			size_t ibeg = bounds.first-_rowid.begin();
			size_t iend = (bounds.second-_rowid.begin())-ibeg;
			if (!iend) {
				_rowid.insert(_rowid.begin()+ibeg,i);
				_colid.insert(_colid.begin()+ibeg,j);
				_data.insert( _data.begin() +ibeg,field().zero);
				return _data[ibeg];
			}
			myIterator beg = _colid.begin()+ibeg ;
			myIterator low = std::lower_bound (beg, beg+(ptrdiff_t)iend, j);
			if (low == beg+(ptrdiff_t)iend) {
				_rowid.insert(_rowid.begin()+ibeg,i);
				_colid.insert(_colid.begin()+ibeg,j);
				_data.insert( _data.begin() +ibeg,field().zero);
				return _data[ibeg];
			}
			else {
				size_t la = low-_colid.begin() ;
				return _data[la] ;
			}
		}
#endif

		/** Write a matrix to the given output stream using field read/write.
		 * @param os Output stream to which to write the matrix
		 * @param format Format with which to write
		 */
		std::ostream & write(std::ostream &os
				     , LINBOX_enum(Tag::FileFormat) format  = Tag::FileFormat::MatrixMarket) const
		{
			return SparseMatrixWriteHelper<Self_t>::write(*this,os,format);
		}


		/** Read a matrix from the given input stream using field read/write
		 * @param is Input stream from which to read the matrix
		 * @param format Format of input matrix
		 * @return ref to \p is.
		 */
		std::istream& read (std::istream &is
				    , LINBOX_enum(Tag::FileFormat) format  = Tag::FileFormat::Detect)
		{
			return SparseMatrixReadHelper<Self_t>::read(*this,is,format);
		}

		/*! @internal
		 * @brief Deletes the entry.
		 * Deletes \c A(i,j) if it exists.
		 * @param i row _colid
		 * @param j col _colid
		 */
		void clearEntry(const size_t &i, const size_t &j)
		{
			linbox_check(i<_rownb);
			linbox_check(j<_colnb);
			typedef typename std::vector<size_t>::iterator myIterator ;

			std::pair<myIterator,myIterator> bounds = std::equal_range (_rowid.begin(), _rowid.end(), i);
			size_t ibeg = bounds.first-_rowid.begin();
			size_t iend = (bounds.second-_rowid.begin());
			if (ibeg == iend)
				return ;

			myIterator beg = _colid.begin() +(ptrdiff_t)ibeg;
			myIterator end = _colid.begin() +(ptrdiff_t)iend;
			myIterator low = std::lower_bound (beg, end, j);
			if (low == end)
				return ;
			else {
				// not sure
				size_t la = low-_colid.begin() ;
				_rowid.erase(_rowid.begin()+(ptrdiff_t)la);
				_colid.erase(_colid.begin()+(ptrdiff_t)la);
				_data. erase(_data. begin()+(ptrdiff_t)la);
				--_nbnz;
				return  ;
			}
		}

		/*! @internal
		 * @brief cleans 0 entries.
		 */
		void clean()
		{
			size_t i = 0 ;
			while ( i < _data.size() ) {
				if ( field().isZero(_data[i]) ) {
					_rowid.erase(_rowid.begin()+i);
					_colid.erase(_colid.begin()+i);
					_data. erase(_data. begin()+i);
				}
				else
					++i ;
			}
			return ;
		}

		// y = A x + a * y ;
		template<class Vector>
		Vector& apply(Vector &y, const Vector& x, const Element & a ) const
		{
			// linbox_check(consistent());
			prepare(field(),y,a);

			size_t z = 0 ;
			size_t last_i = 0 ;

#if 0
			for (size_t i = 0 ; i < _nbnz ; ++i)
				field().axpyin( y[_rowid[i]], _data[i], x[_colid[i]] );
#else
			FieldAXPY<Field> accu(field());
			while ( z < _nbnz) {
				if (_rowid[z] == last_i) {
					accu.mulacc(  _data[z], x[_colid[z]] );
				}
				else {
					accu.get(y[last_i]);
					last_i = _rowid[z] ;
					accu.reset();
					accu.mulacc(  _data[z], x[_colid[z]] );
				}
				++z ;
			}
			accu.get(y[last_i]);
#endif

			return y;
		}


		template<class Vector>
		Vector& applyTranspose(Vector &y, const Vector& x, const Element & a ) const
		{
			linbox_check(consistent());
			if (_helper.optimized(*this)) {
				return _helper.matrix().apply(y,x,a) ; // NEVER use applyTranspose on that thing.
			}

			prepare(field(),y,a);

#if 0
			for (size_t i = 0 ; i < _nbnz ; ++i)
				field().axpyin( y[_colid[i]], _data[i], x[_rowid[i]] );
#else
			const FieldAXPY<Field> accu0(field());
			std::vector<FieldAXPY<Field> > Y(_colnb, accu0);
			for (size_t i = 0 ; i < _nbnz ; ++i)
				Y[_colid[i]].mulacc( _data[i], x[_rowid[i]] );
			for (size_t i = 0 ; i < _colnb ; ++i)
				Y[i].get(y[i]) ;
#endif




			return y;
		}



		template<class inVector, class outVector>
		outVector& apply(outVector &y, const inVector& x ) const
		{
			return apply(y,x,field().zero);
		}

		template<class inVector, class outVector>
		outVector& applyTranspose(outVector &y, const inVector& x ) const
		{
			return applyTranspose(y,x,field().zero);
		}

		const Field & field()  const
		{
			return _field ;
		}

		//! @todo
		bool consistent() const
		{
			return true ;
		}

	private :

	class Helper {
			bool _useable ;
			bool _optimized ;
			bool blackbox_usage ;
			Self_t *_AT ;
		public:

			Helper() :
				_useable(false)
				,_optimized(false)
				, blackbox_usage(true)
				, _AT(NULL)
			{}

			~Helper()
			{
				if ( _AT ) {
					delete _AT ;
				}
			}

			bool optimized(const Self_t & A)
			{
				if (!_useable) {
					getHelp(A);
					_useable = true;
				}
				return	_optimized;
			}

			void getHelp(const Self_t & A)
			{
				if ( A.size() > LINBOX_COO_TRANSPOSE ) { // and/or A.rowDensity(), A.coldim(),...
					// std::cout << "optimizing..." ;
					_optimized = true ;
					_AT = new Self_t(A.field(),A.coldim(),A.rowdim());
					A.transpose(*_AT);
					// std::cout << "done!" << std::endl;
				}
			}

			const Self_t & matrix() const
			{
				return *_AT ;
			}

		};

	public:
		// pseudo iterators
		size_t getRowid(const size_t & i) const
		{
			return _rowid[i];
		}

		void setTriple(const size_t &loc, const size_t & i, const size_t & j, const Element & e)
		{
			linbox_check(loc <= _nbnz);
			linbox_check( _rowid.size() == _nbnz);
			linbox_check( _colid.size() == _nbnz);
			linbox_check( _data.size() == _nbnz);
			if (loc==_nbnz) {
#if 0 /*  reserve is slow */
				if (loc % 10 == 0) {
					_rowid.reserve(loc+10);
					_colid.reserve(loc+10);
					_data.reserve(loc+10);
				}
#endif
				_rowid.push_back(i);
				_colid.push_back(j);
				_data.push_back(e);
				++_nbnz;
				return;
			}
			_rowid[loc]=i;
			_colid[loc]=j;
			_data[loc]=e;

		}

		void setRowid(const std::vector<size_t> &  new_rowid)
		{
			_rowid = new_rowid ;
		}

		std::vector<size_t>  getRowid( ) const
		{
			return _rowid ;
		}

		size_t getColid(const size_t & i) const
		{
			return _colid[i];
		}


		void setColid(std::vector<size_t> new_colid)
		{
			_colid = new_colid ;
		}

		std::vector<size_t>  getColid( ) const
		{
			return _colid ;
		}

		const Element & getData(const size_t & i) const
		{
			return _data[i];
		}


		void setData(const std::vector<Element> & new_data)
		{
			_data = new_data ;
		}

		std::vector<Element>  getData( ) const
		{
			return _data ;
		}

		void firstTriple() const
		{
			_triples.reset();
		}

		bool nextTriple(size_t & i, size_t &j, Element &e) const
		{
			ptrdiff_t idx = _triples.next();
			if (idx >= (ptrdiff_t)_nbnz) {
				_triples.reset() ;
				return false;
			}

			i = _rowid[(size_t)idx];
			j = _colid[(size_t)idx];
			e = _data[(size_t)idx];

			return true;
		}

		template<class element_iterator, class Element>
		class _Iterator {
		private :
			element_iterator _data_it ;
			const element_iterator _data_beg ;
			const element_iterator _data_end ;
		public:
			typedef Element value_type ;
			_Iterator(element_iterator e_beg, element_iterator e_end) :
				_data_it(e_beg)
				, _data_beg(e_beg)
				, _data_end(e_end)
			{}

			_Iterator (const _Iterator &iter) :
				_data_it(iter._data_it)
				, _data_beg(iter._data_beg)
				, _data_end(iter._data_end)
			{}

			_Iterator &operator = (const _Iterator &iter)
			{
				_data_it  = iter._data_it  ;
				_data_beg  = iter._data_beg  ;
				_data_end  = iter._data_end  ;

				return *this;
			}

			bool operator == (const _Iterator &i) const
			{
				return  (_data_it == i._data_it) && (_data_beg == i._data_beg) && (_data_end == i._data_end);
			}

			bool operator != (const _Iterator &i) const
			{
				return  (_data_it != i._data_it) || (_data_beg != i._data_beg) || (_data_end != i._data_end);
			}

			_Iterator &operator ++ ()
			{
				++_data_it ;

				if (_data_it == _data_end)
					return *this ;
			}

			_Iterator operator ++ (int)
			{
				_Iterator tmp = *this;
				++(*this);
				return tmp;
			}

			_Iterator &operator -- ()
			{
				--_data_it ;

				if (_data_it == _data_beg)
					return *this ;
			}

			_Iterator operator -- (int)
			{
				_Iterator tmp = *this;
				--(*this);
				return tmp;
			}

			value_type &operator * ()
			{
				return *_data_it;
			}

			value_type *operator -> ()
			{
				return _data_it ;
			}

			const value_type &operator*() const
			{
				return *_data_it;
			}

			const value_type *operator -> () const
			{
				return _data_it ;
			}

			const value_type &value() const
			{
				return *_data_it;
			}

		};

		template<class index_iterator, class element_iterator, class Element>
		class _IndexedIterator {
		private :
			typedef  index_iterator    index_it ;
			typedef  element_iterator  data_it ;
			index_it _rowid_it ;
			index_it _colid_it ;
			data_it _data_it ;
			const data_it _data_beg ;
			const data_it _data_end ;
		public:
			typedef Element value_type ;
			_IndexedIterator( const index_it &i
					  , const index_it &j
					  , const data_it &e
					  , const data_it &e_e) :
				_rowid_it(i)
				, _colid_it(j)
				, _data_it(e)
				, _data_beg(e)
				, _data_end(e_e)
			{}

			_IndexedIterator (const _IndexedIterator &iter) :
				_rowid_it(iter._rowid_it)
				, _colid_it(iter._colid_it)
				, _data_it(iter._data_it)
				, _data_beg(iter._data_beg)
				, _data_end(iter._data_end)
			{}

			_IndexedIterator &operator = (const _IndexedIterator &iter)
			{
				_rowid_it = iter._rowid_it ;
				_colid_it = iter._colid_it ;
				_data_it  = iter._data_it  ;
				_data_beg = iter._data_beg ;
				_data_end  = iter._data_end  ;

				return *this;
			}

			bool operator == (const _IndexedIterator &i) const
			{
				// we assume consistency
				return (_data_it == i._data_it);
			}

			bool operator != (const _IndexedIterator &i) const
			{
				// we assume consistency
				return  (_data_it != i._data_it) ;
			}

			_IndexedIterator &operator ++ ()
			{

				++_data_it  ;
				if (_data_it == _data_end) {
					return *this ;
				}
				++_colid_it ;
				++_rowid_it ;

				return *this;
			}

			_IndexedIterator operator ++ (int)
			{
				_IndexedIterator tmp = *this;
				++(*this);
				return tmp;
			}

			_IndexedIterator &operator -- ()
			{
				throw NotImplementedYet("not sure");
				--_data_it  ;
				if (_data_it == _data_beg) {
					return *this ;
				}
				--_colid_it ;
				--_rowid_it ;

				return *this;
			}

			_IndexedIterator operator -- (int)
			{
				_IndexedIterator tmp = *this;
				--(*this);
				return tmp;
			}

			value_type &operator * ()
			{
				return *_data_it;
			}

			value_type *operator -> ()
			{
				return _data_it ;
			}

			const value_type &operator*() const
			{
				return *_data_it;
			}

			const value_type *operator -> () const
			{
				return _data_it ;
			}

			size_t rowIndex () const
			{
				return *_rowid_it;
			}

			size_t colIndex () const
			{
				return *_colid_it;
			}

			const value_type &value() const
			{
				return *_data_it;
			}


		};

		typedef _Iterator<typename std::vector<Element>::iterator, Element> Iterator;
		typedef _Iterator<typename std::vector<Element>::const_iterator, constElement> ConstIterator;

		typedef _IndexedIterator<std::vector<size_t>::iterator, typename std::vector<Element>::iterator, Element> IndexedIterator;
		typedef _IndexedIterator<std::vector<size_t>::const_iterator, typename std::vector<Element>::const_iterator, constElement> ConstIndexedIterator;


		Iterator      Begin ()
		{
			return Iterator(_data.begin(),_data.end()) ;
		}

		Iterator      End   ()
		{
			return Iterator(_data.end(), _data.end()) ;
		}

		ConstIterator      Begin () const
		{
			return ConstIterator(_data.begin(),_data.end()) ;
		}

		ConstIterator      End   () const
		{
			return ConstIterator(_data.end(), _data.end()) ;
		}

		IndexedIterator      IndexedBegin ()
		{
			return IndexedIterator(_rowid.begin(), _colid.begin(), _data.begin(),_data.end()) ;
		}

		IndexedIterator      IndexedEnd   ()
		{
			return IndexedIterator(_rowid.end(), _colid.end(), _data.end(),_data.end()) ;
		}

		ConstIndexedIterator      IndexedBegin () const
		{
			return ConstIndexedIterator(_rowid.begin(), _colid.begin(), _data.begin(),_data.end()) ;
		}

		ConstIndexedIterator      IndexedEnd   () const
		{
			return ConstIndexedIterator(_rowid.end(), _colid.end(), _data.end(),_data.end()) ;
		}


	protected :
		friend class SparseMatrixWriteHelper<Self_t >;
		friend class SparseMatrixReadHelper<Self_t >;


		size_t              _rownb ;
		size_t              _colnb ;
		size_t               _nbnz ;

		std::vector<size_t> _rowid ;
		std::vector<size_t> _colid ;
		std::vector<Element> _data ;

		const _Field & _field;

		mutable Helper _helper ;

		mutable struct _triples {
			ptrdiff_t _nnz ;
			_triples() :
				_nnz(-1)
			{}

			ptrdiff_t next()
			{
				return _nnz +=1 ;
			}

			void reset()
			{
				_nnz = -1 ;
			}
		}_triples;
	};





} // namespace LinBox

#endif // __LINBOX_matrix_sparsematrix_sparse_coo_matrix_H

// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
