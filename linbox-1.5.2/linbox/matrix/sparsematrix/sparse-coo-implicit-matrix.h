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

/*! @file matrix/sparsematrix/sparse-coo-implicit-matrix.h
 * @ingroup sparsematrix
 * @brief
 */

#ifndef __LINBOX_matrix_sparsematrix_sparse_coo_implicit_matrix_H
#define __LINBOX_matrix_sparsematrix_sparse_coo_implicit_matrix_H

#include <utility>
#include <iostream>
#include <algorithm>

#include "linbox/linbox-config.h"
#include "linbox/util/debug.h"
#include "sparse-domain.h"


namespace LinBox
{

	/** Sparse matrix, Coordinate storage.
	 *
	 * \ingroup matrix
	 * \ingroup sparse
	 */
	template<class _Field>
	class SparseMatrix<_Field, SparseMatrixFormat::COO::implicit> {
	public :
		typedef _Field                                 Field ; //!< Field
		typedef typename _Field::Element             Element ; //!< Element
		typedef const Element                   constElement ; //!< const Element
		typedef SparseMatrixFormat::COO::implicit   Storage ; //!< Matrix Storage Format
		typedef SparseMatrix<_Field,Storage>         Self_t ; //!< Self type

		/*! Constructors.
		 * @todo convert from other matrix.
		 *
		 */
		//@{
		SparseMatrix<_Field, SparseMatrixFormat::COO::implicit> () :
			_rownb(0),_colnb(0),
			_nbnz(0),
			_rowid(0),_colid(0)
			, _field()
		{
		}

		SparseMatrix<_Field, SparseMatrixFormat::COO::implicit> (const _Field & F) :
			_rownb(0),_colnb(0),
			_nbnz(0),
			_rowid(0),_colid(0),_data(F.zero)
			, _field(F)
		{
		}

		SparseMatrix<_Field, SparseMatrixFormat::COO::implicit> (const _Field & F, size_t m, size_t n) :
			_rownb(m),_colnb(n),
			_nbnz(0),
			_rowid(0),_colid(0),_data(F.zero)
			, _field(F)
		{
		}

		SparseMatrix<_Field, SparseMatrixFormat::COO::implicit> (const _Field & F,
							       size_t m, size_t n,
							       size_t z) :
			_rownb(m),_colnb(n),
			_nbnz(z),
			_rowid(z),_colid(z),_data(F.zero),
			_field(F)
		{
		}

		SparseMatrix<_Field, SparseMatrixFormat::COO::implicit> (const SparseMatrix<_Field, SparseMatrixFormat::COO::implicit> & S) :
			_rownb(S._rownb),_colnb(S._colnb),
			_nbnz(S._nbnz),
			_rowid(S._rowid),_colid(S._colid),_data(S._data),
			_field(S._field)
		{}

#if 0
		template<class _OtherField>
		SparseMatrix<_Field, SparseMatrixFormat::COO::implicit> (const SparseMatrix<_OtherField, SparseMatrixFormat::COO::implicit> & S) :
			_rownb(S._rownb),_colnb(S._colnb),
			_nbnz(S._nbnz),
			_rowid(S._rowid),_colid(S._colid),_data(S._data),
			_field(S._field)
		{}
#endif

		// XXX only for COO::implicit
		template<typename _Tp1, typename _Rw1 = SparseMatrixFormat::COO::implicit>
		struct rebind ;

		template<typename _Tp1>
		struct rebind<_Tp1/*  ,SparseMatrixFormat::COO::implicit */ > {
			typedef SparseMatrix<_Tp1, SparseMatrixFormat::COO::implicit> other;

			void operator() (other & Ap, const Self_t& A)
			{
				// Ap = new other(F, A.rowdim(), A.coldim());

				typename _Tp1::Element e;

				Hom<typename Self_t::Field, _Tp1> hom(A.field(), Ap.field());
				hom. image ( e, A.getData() );
				if (!Ap.field().isZero(e)) {
					Ap.setData(e);
				}
			}
		};

		template<typename _Tp1, typename _Rw1>
		SparseMatrix (const SparseMatrix<_Tp1, _Rw1> &S, const Field& F) :
			_rownb(S.rowdim()),_colnb(S.coldim()),
			_nbnz(S.size()),
			_rowid(S.size()),_colid(S.size()),_data(S.size()),
			_field(F)
		{
			typename SparseMatrix<_Tp1,_Rw1>::template rebind<Field,SparseMatrixFormat::COO::implicit>()(*this, S);
		}



#if  0

		template<class VectStream>
		SparseMatrix<_Field, SparseMatrixFormat::COO::implicit> (const _Field & F, VectStream & stream) :
			_rownb(stream.size()),_colnb(stream.dim()),
			_rowid(0),_colid(0),_data(F.zero)
			, _nbnz(0)
			, _field(F)
		{
			// VectStream == RandomSparseStream<Field, typename Vector<Field>::SparseSeq>
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
#endif


		void resize(size_t nn)
		{
#ifndef NDEBUG
			if (nn < _nbnz) {
				std::cerr << "*** Warning *** you are possibly loosing data (COO::implicit resize)" << std::endl;
				// could be a commentator()...
			}
#endif
			_rowid.resize(nn);
			_colid.resize(nn);
			_nbnz = nn ;
		}

		void resize(const size_t & mm, const size_t & nn, const size_t & zz)
		{
			_rownb = mm ;
			_colnb = nn ;
			_nbnz = zz;

			_rowid.resize(zz);
			_colid.resize(zz);
		}

		/*! Default converter.
		 * @param S a sparse matrix in any storage.
		 */
		template<class _OtherStorage>
		SparseMatrix<_Field, SparseMatrixFormat::COO::implicit> (const SparseMatrix<_Field, _OtherStorage> & S) :
			_rownb(S._rownb),_colnb(S._colnb),
			_rowid(S.size()),_colid(S.size()),_data(S.size()),
			_field(S._field)
		{
			SparseMatrix<_Field,SparseMatrixFormat::CSR> Temp(_field,_rownb,_colnb) ;
			S.exporte(Temp); // convert S to CSR
			this->importe(Temp); // convert Temp from COO::implicit
		}


		//@}
		/*! Conversions.
		 * Any sparse matrix has a converter to/from CSR.
		 * A specialisation can skip the temporary CSR matrix created.
		 */
		//@{
		/*! Import a matrix in CSR format to COO::implicit.
		 * @param S CSR matrix to be converted in COO::implicit
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

		void importe(const SparseMatrix<_Field,SparseMatrixFormat::COO::implicit> &S)
		{
			resize( S.rowdim(), S.coldim(), S.size() );

			setRowid(S.getRowid());
			setColid(S.getColid());
			setData(S.getData());
		}

		/*! Export a matrix in CSR format from COO::implicit.
		 * @param S CSR matrix to be converted from COO::implicit
		 */
		SparseMatrix<_Field,SparseMatrixFormat::CSR > &
		exporte(SparseMatrix<_Field,SparseMatrixFormat::CSR> &S)
		{
			S.resize(_rownb,_colnb,_nbnz) ;
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

		SparseMatrix<_Field,SparseMatrixFormat::COO::implicit > &
		exporte(SparseMatrix<_Field,SparseMatrixFormat::COO::implicit> &S)
		{
			S.resize(_rownb,_colnb,_nbnz) ;
			S.setData( _data ) ;
			S.setColid( _colid ) ;
			S.setRowid( _rowid ) ;
		}

		//@}

		/*! In place transpose. Not quite...
		*/
		void transposeIn()
		{
			// SparseMatrix<_Field,SparseMatrixFormat::CSR> Temp(*this);
			SparseMatrix<_Field,SparseMatrixFormat::COO::implicit> Temp(*this);
			Temp.transposeIn();
			importe(Temp);
		}

		/*! Transpose the matrix.
		 *  @param S [out] transpose of self.
		 *  @return a reference to \p S.
		 */
		SparseMatrix<_Field,SparseMatrixFormat::COO::implicit> &
		transpose(SparseMatrix<_Field,SparseMatrixFormat::COO::implicit> &S)
		{
			linbox_check(S.rowdim() == _colnb);
			linbox_check(S.coldim() == _rownb);

			// outStart
			std::vector<size_t> start (_colnb+1,0);
			for (size_t i = 0 ; i < size() ; ++i)
				start[_colid[i]+1] += 1 ;
			for (size_t i = 0 ; i < _colnb ; ++i)
				start[i+1] += start[i] ;
			// inStart;
			std::vector<size_t> _start (_rownb+1,0);
			for (size_t i = 0 ; i < size() ; ++i)
				_start[_rowid[i]+1] += 1 ;
			for (size_t i = 0 ; i < _rownb ; ++i)
				_start[i+1] += _start[i] ;

			{
				size_t i = 0 ;
				std::vector<size_t> done_col(_colnb,0);
				for (size_t nextlig = 1 ; nextlig <= _rownb ; ++nextlig) {
					// treating line before nextlig
					while (i < _start[nextlig]){
					size_t cur_place ;
						cur_place = start[_colid[i]] + done_col[_colid[i]] ;
						S._data[ cur_place ]  = _data[i] ;
						S._colid[ cur_place ] = nextlig-1 ;
						done_col[_colid[i]] += 1 ;
						++i;
					}
				}
			}

			std::swap(_rownb,_colnb);
			for (size_t i = 0 ; i < _rownb ; ++i)
				for (size_t j = start[i] ; j < start[i+1]; ++j)
					S._rowid[j] = i ;
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

		/** Get a read-only individual entry from the matrix.
		 * @param i Row _colid
		 * @param j Column _colid
		 * @return Const reference to matrix entry
		 */
		constElement &getEntry(const size_t &i, const size_t &j) const
		{
			// std::cout << "get entry : " << i << ',' << j << std::endl;
			linbox_check(i<_rownb);
			linbox_check(j<_colnb);
			// Could be improved by adding an initial guess j/rodim*size()
			// typedef typename std::vector<size_t>::iterator myIterator ;
			typedef typename std::vector<size_t>::const_iterator myConstIterator ;

			std::pair<myConstIterator,myConstIterator> bounds = std::equal_range (_rowid.begin(), _rowid.end(), i);
			size_t ibeg = (size_t)(bounds.first-_rowid.begin());
			size_t iend = (size_t)(bounds.second-_rowid.begin())-ibeg;
			if (!iend) {
				return field().zero;
			}

			myConstIterator beg = _colid.begin()+(ptrdiff_t)ibeg ;
			myConstIterator low = std::lower_bound (beg, beg+(ptrdiff_t)iend, j);
			if (low == beg+(ptrdiff_t)iend) {
				return field().zero;
			}
			else {
				// not sure
				size_t la = (size_t)(low-_colid.begin()) ;
				return _data[la] ;
			}
		}

		Element      &getEntry (Element &x, size_t i, size_t j) const
		{
			return x = getEntry (i, j);
		}

		/** Set an individual entry.
		 * Setting the entry to 0 will not remove it from the matrix
		 * @param i Row _colid of entry
		 * @param j Column _colid of entry
		 * @param value Value of the new entry
		 * @todo make it faster if i is 0 or m-1 ?
		 */
		void setEntry(const size_t &i, const size_t &j, const Element& e)
		{
	linbox_check(i<_rownb);
			linbox_check(j<_colnb);

			if (field().isZero(e)) {
				return clearEntry(i,j);
			}
			typedef typename std::vector<size_t>::iterator myIterator ;
			std::pair<myIterator,myIterator> bounds = std::equal_range (_rowid.begin(), _rowid.end(), i);
			size_t ibeg = bounds.first-_rowid.begin();
			size_t iend = (bounds.second-_rowid.begin())-ibeg;
			if (!iend) {
				_rowid.insert(_rowid.begin()+ibeg,i);
				_colid.insert(_colid.begin()+ibeg,j);
				_data.insert( _data.begin() +ibeg,e);
				return ;
			}
			myIterator beg = _colid.begin()+(ptrdiff_t)ibeg ;
			myIterator low = std::lower_bound (beg, beg+(ptrdiff_t)iend, j);
			ibeg = low-_colid.begin();
			if (low == beg+(ptrdiff_t)iend) {
				_rowid.insert(_rowid.begin()+(ptrdiff_t)ibeg,i);
				_colid.insert(_colid.begin() +(ptrdiff_t)ibeg,j);
				_data.insert( _data. begin() +(ptrdiff_t)ibeg,e);
				return ;
			}
			else {
				_data[ibeg] = e ;
				return ;
			}
		}


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
			// Could be improved by adding an initial guess j/rodim*size()
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

		/** Write a matrix to the given output stream using field read/write.
		 * @param os Output stream to which to write the matrix
		 * @param format Format with which to write
		 */
		template<class Format>
		std::ostream & write(std::ostream &os,
				     Format = SparseFileFormat::CSR()) const
		{
			return this->writeSpecialized(os,Format());
		}

		std::ostream & write(std::ostream &os,
				     enum LINBOX_enum(Tag::FileFormat) ff  = Tag::FileFormat::Maple) const
		{
			return this->writeSpecialized(os,ff);
		}


		/** Read a matrix from the given input stream using field read/write
		 * @param file Input stream from which to read the matrix
		 * @param format Format of input matrix
		 * @return ref to \p file.
		 */
		template<class Format>
		std::istream& read (std::istream &file,
				    Format = SparseFileFormat::CSR())
		{
			return readSpecialized(file,Format());
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
			// Could be improved by adding an initial guess j/rodim*size()
			typedef typename std::vector<size_t>::iterator myIterator ;

			std::pair<myIterator,myIterator> bounds = std::equal_range (_rowid.begin(), _rowid.end(), i);
			size_t ibeg = bounds.first-_rowid.begin();
			size_t iend = (bounds.second-_rowid.begin())-ibeg;
			if (!iend)
				return ;

			myIterator beg = _colid.begin()+ibeg ;
			myIterator low = std::lower_bound (beg, beg+(ptrdiff_t)iend, j);
			if (low == beg+(ptrdiff_t)iend)
				return ;
			else {
				// not sure
				size_t la = low-_colid.begin() ;
				_rowid.erase(_rowid.begin()+la);
				_colid.erase(_colid.begin()+la);
				_data. erase(_data. begin()+la);
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

			prepare(field(),y,a);

			for (size_t i = 0 ; i < _nbnz ; ++i)
				field().axpyin( y[_rowid[i]], _data[i], x[_colid[i]] ); //!@bug may be 0...

			return y;
		}

			class Helper ; // transpose

		template<class Vector>
		Vector& applyTranspose(Vector &y, const Vector& x, const Element & a ) const
		{
			prepare(field(),y,a);

			for (size_t i = 0 ; i < _nbnz ; ++i)
				field().axpyin( y[_colid[i]], _data[i], x[_rowid[i]] );

			return y;
		}

		template<class Vector>
		Vector& apply(Vector &y, const Vector& x ) const
		{
			return apply(y,x,field().zero);
		}
		template<class Vector>
		Vector& applyTranspose(Vector &y, const Vector& x ) const
		{
			return apply(y,x,field().zero);
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

		std::ostream & writeSpecialized(std::ostream &os,
						LINBOX_enum(Tag::FileFormat) format) const
		{
			switch (format) {
			case (Tag::FileFormat::Maple):
				{

					linbox_check(_colnb > 0);
					os << "[";
					bool firstrow=true;
					size_t idx = 0 ;

					linbox_check(_nbnz == _rowid.size());
					linbox_check(_nbnz == _data.size());
					linbox_check(_nbnz == _colid.size());
					for (size_t i = 0 ; i < _rownb ; ++i ) {
						if (firstrow) {
							os << "[";
							firstrow =false;
						}
						else
							os << ", [";


						for (size_t j = 0; j < _colnb ; ++j) {
							if (idx == _nbnz)
								field().write (os, field().zero);
							else if (_colid[idx] == j &&
								 _rowid[idx] ==i) {
								field().write (os, _data[idx]);
								++idx;
							}
							else {
								field().write (os, field().zero);
							}

							if (j < _colnb - 1)
								os << ", ";
						}

						os << " ]";
					}

					os << "]";
					linbox_check(idx == _nbnz);

					break;
				}
			default :
				os << "I don't know" << std::endl;

			}
			return os ;

		}

#if 0 /*  not updated and to be cleaned */
		/*! @internal
		 * write for CSR format.
		 */
		std::ostream & writeSpecialized(std::ostream &os,
						SparseFileFormat::CSR) const
		{
			os << _rownb << ' ' << _colnb  << ' ' << size() << std::endl;
			size_t lig = 0 ;
			size_t i = 0 ;
			while(i < size()) {
				while(lig < _rowid[i]) {
					os << "-1" << std::endl;
					++lig ;
				}
				while (lig == _rowid[i]) {
					field().write(_data[i], os << _colid[i] << ' ') << std::endl;
					++i;
				}
				++lig ;
			}
			return os << "0 0 0" << std::endl;
		}

		/*! @internal
		 * write for COO::implicit format.
		 */
		std::ostream & writeSpecialized(std::ostream &os,
						SparseFileFormat::COO::implicit) const
		{
			os << _rownb << ' ' << _colnb  << ' ' << size() << std::endl;
			size_t i = 0 ;
			while(i < size()) {
				field().write(_data[i], os << _rowid[i] << ' ' << _colid[i] << ' ') << std::endl;
				++i;
			}
			return os << "0 0 0" << std::endl;
		}



		/*! @internal
		 * Read for CSR format.
		 */
		std::istream & readSpecialized(std::istream &is,
					       SparseFileFormat::CSR)
		{
			size_t nnz = 0 ;
			bool sms = true ;
			std::string firstLine ;
			std::string x ;
			getline(is, firstLine);
			std::istringstream line(firstLine);
			line >> _rownb >> _colnb >> x ;
			size_t mem = 10 ;
			if (!_rownb || _colnb)
				throw LinBoxError("bad input");
			if (x.empty() || x.compare("M")) {  /* SMS */
				// mem = m ;
				_rowid.reserve(mem);
				_colid.reserve(mem);
				_data.reserve(mem);
			}
			else { /* SMF */
				sms = false ;
				std::istringstream (x) >> nnz ;
				if (!nnz)
					throw LinBoxError("bad input");
				mem = nnz ;
				_rowid.reserve(nnz);
				_colid.reserve(nnz);
				_data.reserve(nnz);
			}
			Element z ;
			if (sms) { /*  SMS */
				size_t lig = 0 ;
				nnz = 0 ;
				int n ;
				while (is>>n) {
					if (n == 0)
						break;
					while (n == -1) {
						++lig ;
						is >> n ;
					}
					field().read(is,z)  ;
					if (n<0 || lig >=_rownb || n >> _colnb)
						throw LinBoxError("bad input");
					if (!field().isZero(z)){
						if (mem == nnz) {
							mem+=20 ;
							_rowid.resize(mem);
							_colid.resize(mem);
							_data.resize (mem);
						}

						_rowid[nnz]= lig ;
						_colid[nnz]= n ;
						_data[nnz] = z ;
						++nnz ;
					}
				}
				_rowid.resize(nnz);
				_colid.resize(nnz);
				_data.resize (nnz);

			}
			else { /*  SMF */
				size_t lig = 0 ;
				int n ;
				size_t loc = 0;
				while (is>>n) {
					if (n == 0)
						break;
					while (n == -1) {
						++lig ;
						is >> n ;
					}
					field().read(is,z)  ;
					if (n<0 || lig >=_rownb || n >> _colnb)
						throw LinBoxError("bad input");
					if (!field().isZero(z)){
						_rowid[loc]= lig ;
						_colid[loc]= n ;
						_data[loc] = z ;
						++loc ;
					}
				}
				if (loc > nnz)
					throw LinBoxError("bad input");
				_rowid.resize(loc);
				_colid.resize(loc);
				_data.resize (loc);
			}
			return is ;
		}

		/*! @internal
		 * Read for COO::implicit format.
		 */
		std::istream & readSpecialized(std::istream &is,
					       SparseFileFormat::COO::implicit)
		{
			size_t nnz = 0;
			bool sms = true ;
			std::string firstLine ;
			getline(is, firstLine);
			std::istringstream line(firstLine);
			std::string x ;
			line >> _rownb >> _colnb >> x ;
			size_t mem  = 10 ;
			if (!_rownb || _colnb)
				throw LinBoxError("bad input");
			if (x.empty() || x.compare("M")) {  /* SMS */
				// mem = m ;
				_rowid.reserve(mem);
				_colid.reserve(mem);
				_data.reserve(mem);
			}
			else { /* SMF */
				sms = false ;
				std::istringstream (x) >> nnz ;
				if (!nnz)
					throw LinBoxError("bad input");
				mem = nnz ;
				_rowid.reserve(nnz);
				_colid.reserve(nnz);
				_data.reserve(nnz);
			}
			Element z ;
			if (sms) { /*  SMS */
				// size_t lig = 0 ;
				nnz = 0 ;
				int m,n ;
				while (is>>m >> n) {
					if (m == 0 && n == 0)
						break;
					if (n<0 || m<0 ||  m >=_rownb || n >> _colnb)
						throw LinBoxError("bad input");
					field().read(is,z)  ;
					if (!field().isZero(z)){
						if (mem == nnz) {
							mem+=20 ;
							_rowid.resize(mem);
							_colid.resize(mem);
							_data.resize (mem);
						}
						_rowid[nnz]= m ;
						_colid[nnz]= n ;
						_data[nnz] = z ;
						++nnz ;
					}
				}
				_rowid.resize(nnz);
				_colid.resize(nnz);
				_data.resize (nnz);

			}
			else { /*  SMF */
				size_t loc = 0 ;
				int m,n ;
				while (is>>m >> n) {
					if (m == 0 && n == 0)
						break;
					if (n<0 || m<0 ||  m >=_rownb || n >> _colnb)
						throw LinBoxError("bad input");
					field().read(is,z)  ;
					if (!field().isZero(z)){
						_rowid[loc]= m ;
						_colid[loc]= n ;
						_data[loc] = z ;
						++loc ;
					}
				}

				if (loc > nnz)
					throw LinBoxError("bad input");
				_rowid.resize(loc);
				_colid.resize(loc);
				_data.resize (loc);

			}
			return is ;
		}
#endif


	public:
		// pseudo iterators
		size_t getRowid(const size_t & i) const
		{
			return _rowid[i];
		}

		void setRowid(const size_t &i, const size_t & j)
		{
			if (i>=_nbnz) this->resize(i);
			_rowid[i]=j;
		}

		void setRowid(const std::vector<size_t> &  new_rowid)
		{
			// linbox_check(_rowid.size() == new_rowid.size());
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

		void setColid(const size_t & i, const size_t & j)
		{
			if (i>=_nbnz) this->resize(i);
			_colid[i]=j;
		}

		void setColid(std::vector<size_t> new_colid)
		{
			// linbox_check(_colid.size() == new_colid.size());
			_colid = new_colid ;
		}

		std::vector<size_t>  getColid( ) const
		{
			return _colid ;
		}



		void setData(Element & new_data)
		{
			_data = new_data ;
		}

		Element  getData( ) const
		{
			return _data ;
		}

	protected :

		size_t              _rownb ;
		size_t              _colnb ;
		size_t               _nbnz ;

		std::vector<size_t> _rowid ;
		std::vector<size_t> _colid ;
		Element              _data ;

		const _Field & _field;
	};




} // namespace LinBox

#endif // __LINBOX_matrix_sparsematrix_sparse_coo_implicit_matrix_H

// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
