/* linbox/algorithms/gauss-elim.inl
 * Copyright (C) 2009 The LinBox group
 *
 * Time-stamp: <21 Jan 10 15:06:11 Jean-Guillaume.Dumas@imag.fr>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========
 *.
 *
 * SparseElimination elimination routines
 */
#ifndef __LINBOX_gauss_elim_INL
#define __LINBOX_gauss_elim_INL

namespace LinBox
{
	template <class _Field>
	template <class Vector> inline void
	GaussDomain<_Field>::permute (Vector              &lignecourante,
				      const unsigned long &indcol,
				      const long &indpermut) const
	{
		const unsigned long k = indcol - 1;

#if 0
		std::cerr << "B PERMUTE: " << indpermut << " <--> " << k << " of  [";
		for(typename Vector::const_iterator refs =  lignecourante.begin();
		    refs != lignecourante.end() ;
		    ++refs )
			std::cerr << '(' << refs->first << ';' << refs->second << ')';
		std::cerr << "]" << std::endl;
#endif

		// precondition indpermut != k
		if (lignecourante.size () ) {
			typename Vector::iterator kin_it=lignecourante.begin();
			for (; kin_it != lignecourante.end(); ++kin_it)
				if (kin_it->first >= k) break;
			if (kin_it !=  lignecourante.end()) {
				typename Vector::iterator pin_it=kin_it;
				for (; pin_it != lignecourante.end(); ++pin_it)
					if (static_cast<long>(pin_it->first) >= indpermut) break;
				if ( kin_it->first == k) {
					if (pin_it != lignecourante.end()) {
						if ( static_cast<long>(pin_it->first) == indpermut) {
							// Both there
							std::swap( kin_it->second, pin_it->second);
						}
						else {
							// Only k there
							kin_it->first = (unsigned)indpermut;
							typename Vector::value_type etmp = *kin_it;
							--pin_it;
							typename Vector::iterator current = kin_it;
							typename Vector::iterator next = kin_it; ++next;
							for( ; current != pin_it; ++current, ++next)
								*current = *next;
							*pin_it = etmp;
						}
					}
					else {
						--pin_it;
						// Only k there
						kin_it->first = (unsigned)indpermut;
						typename Vector::value_type etmp = *kin_it;
						typename Vector::iterator current = kin_it;
						typename Vector::iterator next = kin_it; ++next;
						for( ; current != pin_it; ++current, ++next)
							*current = *next;
						*pin_it = etmp;
					}
				}
				else {
					if (pin_it != lignecourante.end()) {
						if ( static_cast<long>(pin_it->first) == indpermut) {
							// Only indpermut there
							pin_it->first = (unsigned)k;
							typename Vector::value_type etmp = *pin_it;
							typename Vector::iterator current = pin_it;
							typename Vector::iterator prev = pin_it; --prev;
							for( ; current != kin_it; --current, --prev)
								*current = *prev;
							*kin_it = etmp;
						} // else Nobody
					} // else Nobody
				}
			} // else rien de supérieur à k dans l
			// donc rien à permuter
		}

#if 0
		std::cerr << "E PERMUTE: " << indpermut << " <--> " << k << " of  [";
		for(typename Vector::const_iterator refs =  lignecourante.begin();
		    refs != lignecourante.end() ;
		    ++refs )
			std::cerr << '(' << refs->first << ';' << refs->second << ')';
		std::cerr << "]" << std::endl;
#endif
	}




	template <class _Field>
	template <class Vector, class D> inline void
	GaussDomain<_Field>::eliminate (Vector              &lignecourante,
					const Vector        &lignepivot,
					const unsigned long &indcol,
					const long &indpermut,
					D                   &columns) const
	{

		typedef typename Vector::value_type E;
		typedef typename E::first_type E1;

		unsigned long k = indcol - 1;
		unsigned long nj = lignecourante.size () ;
#if 0
		std::cerr << "BEGIN ELIMINATE, k: " << k << ", nj: " << nj << ", indpermut: " << indpermut << ", indcol: " << indcol << std::endl;
		std::cerr << "lignepivot: [";
		for(typename Vector::const_iterator refs =  lignepivot.begin();
		    refs != lignepivot.end() ;
		    ++refs )
			std::cerr << '(' << refs->first << ';' << refs->second << ')';
		std::cerr << "], lignecour: [";
		for(typename Vector::const_iterator refs =  lignecourante.begin();
		    refs != lignecourante.end() ;
		    ++refs )
			std::cerr << '(' << refs->first << ';' << refs->second << ')';
		std::cerr << ']' << std::endl;
#endif
		if (nj > 0) {
			unsigned long j_head = 0;

			for (; j_head < nj; ++j_head) {
				if (static_cast<long>(lignecourante[(size_t)j_head].first) >= indpermut) break;
#if 0
				std::cerr << "ELIMINATE, j_head: " << j_head << std::endl;
#endif
				}

			if (j_head < nj) {
				if (static_cast<long>(lignecourante[(size_t)j_head].first) == indpermut) {
					// -------------------------------------------
					// Permutation
					if ( indpermut != static_cast<long>(k)) {
						if (lignecourante[0].first == k) {
							// non zero  <--> non zero
							std::swap( lignecourante[0].second, lignecourante[(size_t)j_head].second);
						}
						else {
							// zero <--> non zero
							E tmp = lignecourante[(size_t)j_head];
							--columns[(size_t)tmp.first];
							++columns[(size_t)k];
							tmp.first = (E1)k;

							for (long l = (long)j_head; l > 0; --l)
								lignecourante[(size_t)l] = lignecourante[(size_t)l-1];

							lignecourante[0] = tmp;
						}
						j_head = 0;
					}
					// -------------------------------------------
					// Elimination
					unsigned long npiv = lignepivot.size ();
					Vector construit (nj + npiv);

					// construit : <-- j
					// courante  : <-- m
					// pivot     : <-- l
					unsigned long j = 0;
					unsigned long m = j_head + 1;

					// A[i,k] <-- - A[i,k] / A[k,k]
					Element headcoeff;
					field().divin (field().neg (headcoeff, lignecourante[(size_t)j_head].second),
						  lignepivot[0].second);

					--columns[lignecourante[(size_t)j_head].first];

					// if A[k,j]=0, then A[i,j] <-- A[i,j]
					while (j < j_head) {
						construit[j] = lignecourante[(size_t)j];
						j++;
					}


					unsigned long l = 0;

					for (; l < npiv; l++)
						if (lignepivot[l].first > k) break;

					// for all j such that (j>k) and A[k,j]!=0
					while (l < npiv) {
					unsigned long j_piv;
						j_piv = lignepivot[l].first;

						// if A[k,j]=0, then A[i,j] <-- A[i,j]
						while ((m < nj) && (lignecourante[m].first < j_piv))
							construit[j++] = lignecourante[m++];

						// if A[i,j]!=0, then A[i,j] <-- A[i,j] - A[i,k]*A[k,j]
						if ((m < nj) && (lignecourante[m].first == j_piv)) {
							Element tmp;

							field().axpy (tmp, headcoeff, lignepivot[l].second,
								 lignecourante[m].second);

							if (! field().isZero (tmp)) {
								field().assign (lignecourante[m].second, tmp);
								construit[j++] = lignecourante[m++];
							}
							else
								--columns[lignecourante[m++].first];
						}
						else {
							Element tmp;

							field().mul (tmp, headcoeff, lignepivot[l].second);

							// if (! field().isZero (tmp)) {
							++columns[j_piv];
							construit[j++] = E ((unsigned)j_piv, tmp);
							// }
							// else
							// std::cerr << "NEVER HAPPENED" << std::endl;

						}

						l++;
					}

					// if A[k,j]=0, then A[i,j] <-- A[i,j]
					while (m<nj)
						construit[j++] = lignecourante[m++];

					construit.resize (j);
					lignecourante = construit;
				}
				else {
					// -------------------------------------------
					// j_head < nj but nothing under the pivot
					// Permutation
#if 0
					std::cerr << "----------------------------------------------------------" << std::endl;
					std::cerr << "j_head < nj" << std::endl;
					std::cerr << "j_head: " << j_head << ", nj: " << nj << ", k:" << k
					// << "lignepivot: " << lignepivot
					// << ", lignecour: " << lignecourante
					<< std::endl;
					std::cerr << "----------------------------------------------------------" << std::endl;
#endif
					if (indpermut != static_cast<long>(k)) {
						if (j_head>0) {
							unsigned long l = 0;

							for (; l < nj; ++l)
								if (lignecourante[(size_t)l].first >= k) break;

							if ((l < nj) && (lignecourante[(size_t)l].first == k))  {
								// non zero <--> zero
								E tmp = lignecourante[(size_t)l];
								--columns[k];
								++columns[(size_t)indpermut];
								tmp.first = (E1)indpermut;

								unsigned long bjh = j_head-1;
								for (; l < bjh; ++l)
									lignecourante[(size_t)l] = lignecourante[(size_t)l + 1];

								lignecourante[bjh] = tmp;
							} // else // zero <--> zero
						} // else // zero <--> zero
					}
				}
			}
			else {
				// -------------------------------------------
				// j_head >= nj > 0
#if 0
				std::cerr << "----------------------------------------------------------" << std::endl;
				std::cerr << "j_head >= nj > 0" << std::endl;
				std::cerr << "j_head: " << j_head << ", nj: " << nj << ", k:" << k
				// << "lignepivot: " << lignepivot
				// << ", lignecour: " << lignecourante
				<< std::endl;
				std::cerr << "----------------------------------------------------------" << std::endl;
#endif
				if (indpermut != static_cast<long>(k)) {
					unsigned long l = 0;

					for (; l < nj; ++l)
						if (lignecourante[(size_t)l].first >= k) break;

					if ((l < nj) && (lignecourante[(size_t)l].first == k))  {
						// non zero <--> zero
						E tmp = lignecourante[(size_t)l];
						--columns[k];
						++columns[(size_t)indpermut];
						tmp.first = (E1)indpermut;

						unsigned long bjh = nj - 1;
						for (; l < bjh; ++l)
							lignecourante[(size_t)l] = lignecourante[(size_t)l + 1];

						lignecourante[bjh] = tmp;
					} // else
					// zero <--> zero
				}

			}
		}


#if 0
		std::cerr << "END ELIMINATE, k: " << k << ", nj: " << nj << ", indpermut: " << indpermut << ", indcol: " << indcol << std::endl;
		std::cerr << "lignepivot: [";
		for(typename Vector::const_iterator refs =  lignepivot.begin();
		    refs != lignepivot.end() ;
		    ++refs )
			std::cerr << '(' << refs->first << ';' << refs->second << ')';
		std::cerr << "], lignecour: [";
		for(typename Vector::const_iterator refs =  lignecourante.begin();
		    refs != lignecourante.end() ;
		    ++refs )
			std::cerr << '(' << refs->first << ';' << refs->second << ')';
		std::cerr << ']' << std::endl;
#endif
	}


	template <class _Field>
	template <class Vector, class D> inline void
	GaussDomain<_Field>::eliminate (Element             &headpivot,
					Vector              &lignecourante,
					const Vector        &lignepivot,
					const unsigned long indcol,
					const long indpermut,
					const unsigned long npiv,
					D                   &columns) const
	{

		typedef typename Vector::value_type E;
		typedef typename E::first_type E1;

		unsigned long k = indcol - 1;
		unsigned long nj = lignecourante.size () ;
#if 0
		std::cerr << "BEGIN ELIMINATE, k: " << k << ", nj: " << nj << ", indpermut: " << indpermut << ", indcol: " << indcol << std::endl;
		std::cerr << "lignepivot: [";
		for(typename Vector::const_iterator refs =  lignepivot.begin();
		    refs != lignepivot.end() ;
		    ++refs )
			std::cerr << '(' << refs->first << ';' << refs->second << ')';
		std::cerr << "], lignecour: [";
		for(typename Vector::const_iterator refs =  lignecourante.begin();
		    refs != lignecourante.end() ;
		    ++refs )
			std::cerr << '(' << refs->first << ';' << refs->second << ')';
		std::cerr << ']' << std::endl;
#endif
		if (nj > 0) {
			unsigned long j_head = 0;

			for (; j_head < nj; ++j_head) {
				if (static_cast<long>(lignecourante[(size_t)j_head].first) >= indpermut) break;
#if 0
				std::cerr << "ELIMINATE, j_head: " << j_head << std::endl;
#endif
				}

			if (j_head < nj) {
				if (static_cast<long>(lignecourante[(size_t)j_head].first) == indpermut) {
					// -------------------------------------------
					// Permutation
					if ( indpermut != static_cast<long>(k)) {
						if (lignecourante[0].first == k) {
							// non zero  <--> non zero
							std::swap( lignecourante[0].second, lignecourante[(size_t)j_head].second);
						}
						else {
							// zero <--> non zero
							E tmp = lignecourante[(size_t)j_head];
							--columns[tmp.first];
							++columns[k];
							tmp.first = (E1)k;

							for (long l = (long)j_head; l > 0; --l)
								lignecourante[(size_t)l] = lignecourante[(size_t)l-1];

							lignecourante[0] = tmp;
						}
						j_head = 0;
					}
					// -------------------------------------------
					// Elimination
					Vector construit (nj + npiv);

					// construit : <-- j
					// courante  : <-- m
					// pivot     : <-- l
					unsigned long j = 0;
					unsigned long m = j_head + 1;

					// A[i,k] <-- - A[i,k] / A[k,k]
					Element headcoeff;
					field().div( headpivot, lignecourante[(size_t)j_head].second,
						lignepivot[0].second);
					field().neg(headcoeff, headpivot);
					//                     field().divin (field().neg (headcoeff, lignecourante[(size_t)j_head].second),
					//                               lignepivot[0].second);

					--columns[lignecourante[(size_t)j_head].first];

					// if A[k,j]=0, then A[i,j] <-- A[i,j]
					while (j < j_head) {
						construit[j] = lignecourante[(size_t)j];
						j++;
					}


					unsigned long l = 0;

					for (; l < npiv; l++)
						if (lignepivot[l].first > k) break;

					// for all j such that (j>k) and A[k,j]!=0
					while (l < npiv) {
						unsigned long j_piv;
						j_piv = lignepivot[l].first;

						// if A[k,j]=0, then A[i,j] <-- A[i,j]
						while ((m < nj) && (lignecourante[m].first < j_piv))
							construit[j++] = lignecourante[m++];

						// if A[i,j]!=0, then A[i,j] <-- A[i,j] - A[i,k]*A[k,j]
						if ((m < nj) && (lignecourante[m].first == j_piv)) {
							Element tmp;

							field().axpy (tmp, headcoeff, lignepivot[l].second,
								 lignecourante[m].second);

							if (! field().isZero (tmp)) {
								field().assign (lignecourante[m].second, tmp);
								construit[j++] = lignecourante[m++];
							}
							else
								--columns[lignecourante[m++].first];
						}
						else {
							Element tmp;

							field().mul (tmp, headcoeff, lignepivot[l].second);

							// if (! field().isZero (tmp)) {
							++columns[j_piv];
							construit[j++] = E ((unsigned)j_piv, tmp);
							// }
							// else
							// std::cerr << "NEVER HAPPENED" << std::endl;

						}

						l++;
					}

					// if A[k,j]=0, then A[i,j] <-- A[i,j]
					while (m<nj)
						construit[j++] = lignecourante[m++];

					construit.resize (j);
					lignecourante = construit;
				}
				else {
					// -------------------------------------------
					// j_head < nj but nothing under the pivot
					// Permutation
#if 0
					std::cerr << "----------------------------------------------------------" << std::endl;
					std::cerr << "j_head < nj" << std::endl;
					std::cerr << "j_head: " << j_head << ", nj: " << nj << ", k:" << k
					// << "lignepivot: " << lignepivot
					// << ", lignecour: " << lignecourante
					<< std::endl;
					std::cerr << "----------------------------------------------------------" << std::endl;
#endif
					if (indpermut != static_cast<long>(k)) {
						if (j_head>0) {
							unsigned long l = 0;

							for (; l < nj; ++l)
								if (lignecourante[(size_t)l].first >= k) break;

							if ((l < nj) && (lignecourante[(size_t)l].first == k))  {
								// non zero <--> zero
								E tmp = lignecourante[(size_t)l];
								--columns[k];
								++columns[(size_t)indpermut];
								tmp.first = (E1)indpermut;

								unsigned long bjh = j_head-1;
								for (; l < bjh; ++l)
									lignecourante[(size_t)l] = lignecourante[(size_t)l + 1];

								lignecourante[bjh] = tmp;
							} // else // zero <--> zero
						} // else // zero <--> zero
					}
				}
			}
			else {
				// -------------------------------------------
				// j_head >= nj > 0
#if 0
				std::cerr << "----------------------------------------------------------" << std::endl;
				std::cerr << "j_head >= nj > 0" << std::endl;
				std::cerr << "j_head: " << j_head << ", nj: " << nj << ", k:" << k
				// << "lignepivot: " << lignepivot
				// << ", lignecour: " << lignecourante
				<< std::endl;
				std::cerr << "----------------------------------------------------------" << std::endl;
#endif
				if (indpermut != static_cast<long>(k)) {
					unsigned long l = 0;

					for (; l < nj; ++l)
						if (lignecourante[(size_t)l].first >= k) break;

					if ((l < nj) && (lignecourante[(size_t)l].first == k))  {
						// non zero <--> zero
						E tmp = lignecourante[(size_t)l];
						--columns[k];
						++columns[(size_t)indpermut];
						tmp.first = (E1)indpermut;

						unsigned long bjh = nj - 1;
						for (; l < bjh; ++l)
							lignecourante[(size_t)l] = lignecourante[(size_t)l + 1];

						lignecourante[bjh] = tmp;
					} // else
					// zero <--> zero
				}

			}
		}

#if 0
		std::cerr << "END ELIMINATE, k: " << k << ", nj: " << nj << ", indpermut: " << indpermut << ", indcol: " << indcol << std::endl;
		std::cerr << "lignepivot: [";
		for(typename Vector::const_iterator refs =  lignepivot.begin();
		    refs != lignepivot.end() ;
		    ++refs )
			std::cerr << '(' << refs->first << ';' << refs->second << ')';
		std::cerr << "], lignecour: [";
		for(typename Vector::const_iterator refs =  lignecourante.begin();
		    refs != lignecourante.end() ;
		    ++refs )
			std::cerr << '(' << refs->first << ';' << refs->second << ')';
		std::cerr << ']' << std::endl;
#endif
	}



	template <class _Field>
	template <class Vector> inline void
	GaussDomain<_Field>::eliminate (Vector              &lignecourante,
					const Vector        &lignepivot,
					const unsigned long &indcol,
					const long &indpermut) const
	{
		typedef typename Vector::value_type E;
		typedef typename E::first_type E1;

		unsigned long k = indcol - 1;
		unsigned long nj = lignecourante.size () ;

		if (nj > 0) {
			unsigned long j_head = 0;

			for (; j_head < nj; ++j_head)
				if (static_cast<long>(lignecourante[(size_t)j_head].first) >= indpermut) break;

			if (j_head < nj) {
				if (static_cast<long>(lignecourante[(size_t)j_head].first) == indpermut) {
					// -------------------------------------------
					// Permutation
					if (indpermut != static_cast<long>(k)) {
						if (lignecourante[0].first == k) {
							// non zero  <--> non zero
							std::swap( lignecourante[0].second, lignecourante[(size_t)j_head].second);
						}
						else {
							// zero <--> non zero
							E tmp = lignecourante[(size_t)j_head];
							tmp.first = (E1)k;
							for (long l = (long)j_head; l > 0; --l)
								lignecourante[(size_t)l] = lignecourante[(size_t)l-1];
							lignecourante[0] = tmp;
						}

						j_head = 0;
					}
					// -------------------------------------------
					// Elimination
					unsigned long npiv = lignepivot.size ();
					Vector construit (nj + npiv);
					// construit : <-- j
					// courante  : <-- m
					// pivot     : <-- l
					unsigned long j = 0;
					unsigned long m = j_head + 1;

					// A[i,k] <-- - A[i,k] / A[k,k]

					Element headcoeff;
					field().divin (field().neg (headcoeff, lignecourante[(size_t)j_head].second),
						  lignepivot[0].second);

					// if A[k,j]=0, then A[i,j] <-- A[i,j]
					while (j < j_head) {
						construit[j] = lignecourante[(size_t)j];
						j++;
					}

					unsigned long l = 0;

					for (; l < npiv; l++)
						if (lignepivot[l].first > k) break;

					// for all j such that (j>k) and A[k,j]!=0
					while (l < npiv) {
					unsigned long j_piv;
						j_piv = lignepivot[l].first;

						// if A[k,j]=0, then A[i,j] <-- A[i,j]
						while ((m < nj) && (lignecourante[m].first < j_piv))
							construit[j++] = lignecourante[m++];

						// if A[i,j]!=0, then A[i,j] <-- A[i,j] - A[i,k]*A[k,j]
						if ((m < nj) && (lignecourante[m].first == j_piv)) {
							Element tmp;
							field().axpy (tmp, headcoeff, lignepivot[l].second,
								 lignecourante[m].second);

							if (! field().isZero (tmp)) {
								field().assign (lignecourante[m].second, tmp);
								construit[j++] = lignecourante[m++];
							}
							else
								++m;

						}
						else {
							Element tmp;
							field().mul (tmp, headcoeff, lignepivot[l].second);
							// if (! field().isZero (tmp))
							construit[j++] = E (j_piv, tmp);
							// else
							// std::cerr << "NEVER HAPPENED" << std::endl;
						}
						l++;
					}

					// if A[k,j]=0, then A[i,j] <-- A[i,j]
					while (m < nj)
						construit[j++] = lignecourante[m++];

					construit.resize (j);
					lignecourante = construit;
				}
				else {
					// -------------------------------------------
					// j_head < nj but nothing under the pivot
					// Permutation
					if (indpermut != static_cast<long>(k)) {
						if (j_head > 0) {
							unsigned long l = 0;

							for (; l < nj; ++l)
								if (lignecourante[(size_t)l].first >= k) break;

							if ((l < nj) && (lignecourante[(size_t)l].first == k))  {
								// non zero <--> zero
								E tmp = lignecourante[(size_t)l];
								tmp.first = (E1) indpermut;

								unsigned long bjh = j_head -1;
								for (; l < bjh; l++)
									lignecourante[(size_t)l] = lignecourante[(size_t)l + 1];

								lignecourante[bjh] = tmp;
							} // else // zero <--> zero
						} // else // zero <--> zero
					}
				}
			}
			else {
				// -------------------------------------------
				// -------------------------------------------
				// j_head >= nj > 0
#if 0
				std::cerr << "----------------------------------------------------------" << std::endl;
				std::cerr << "j_head >= nj > 0" << std::endl;
				std::cerr << "j_head: " << j_head << ", nj: " << nj << ", k:" << k
				// << "lignepivot: " << lignepivot
				//  << ", lignecour: " << lignecourante
				<< std::endl;
				std::cerr << "----------------------------------------------------------" << std::endl;
#endif
				if (indpermut != static_cast<long>(k)) {
					unsigned long l = 0;

					for (; l < nj; ++l)
						if (lignecourante[(size_t)l].first >= k) break;

					if ((l < nj) && (lignecourante[(size_t)l].first == k))  {
						// non zero <--> zero
						E tmp = lignecourante[(size_t)l];
						tmp.first = (E1) indpermut;

						unsigned long bjh = nj - 1;
						for (; l < bjh; l++)
							lignecourante[(size_t)l] = lignecourante[(size_t)l + 1];

						lignecourante[bjh] = tmp;
					} // else
					// zero <--> zero
				}
			}
		}
	}


} // namespace LinBox

#endif // __LINBOX_gauss_elim_INL

#if 0
template <class _Field>
template <class Vector>
void GaussDomain<_Field>::permute (Vector              &lignecourante,
				   const unsigned long &indcol,
				   const long &indpermut)
{
	const unsigned long k = indcol - 1;
#if 0
	std::cerr << "B PERMUTE: " << indpermut << " <--> " << k << " of  [";
	for(typename Vector::const_iterator refs =  lignecourante.begin();
	    refs != lignecourante.end() ;
	    ++refs )
		std::cerr << '(' << refs->first << ';' << refs->second << ')';
	std::cerr << "]" << std::endl;
#endif

	// precondition indpermut != k
	unsigned long nj = lignecourante.size () ;
	if (nj > 0) {
		Element tmp; field().init(tmp);
		unsigned long kin = 0;
		for (; kin < nj; ++kin)
			if (static_cast<long>(lignecourante[kin].first) >= k) break;
		if (kin < nj) {
			unsigned long pin = kin;
			for (; pin < nj; ++pin)
				if (static_cast<long>(lignecourante[(size_t)pin].first) >= indpermut) break;
			if ( static_cast<long>(lignecourante[kin].first) == k) {
				if (pin < nj) {
					if ( static_cast<long>(lignecourante[(size_t)pin].first) == indpermut) {
						// Both there
						std::swap( lignecourante[kin].second, lignecourante[(size_t)pin].second);
					}
					else {
						// Only k there
						lignecourante[kin].first = (E1)indpermut;
						typename Vector::value_type etmp = lignecourante[kin];
						--pin;
						for(size_t i=kin; i<pin; ++i)
							lignecourante[(size_t)i] = lignecourante[(size_t)i+1];
						lignecourante[(size_t)pin] = etmp;
					}
				}
				else {
					pin = nj-1;
					// Only k there
					lignecourante[kin].irst = indpermut;
					typename Vector::value_type etmp = lignecourante[kin];
					for(size_t i=kin; i<pin; ++i)
						lignecourante[(size_t)i] = lignecourante[(size_t)i+1];
					lignecourante[(size_t)pin] = etmp;

				}
			}
			else {
				if (pin < nj) {
					if ( static_cast<long>(lignecourante[(size_t)pin].first) == indpermut) {
						// Only indpermut there
						lignecourante[(size_t)pin].first = (E1)k;
						typename Vector::value_type etmp = lignecourante[(size_t)pin];
						for(size_t i = pin; i>kin; --i)
							lignecourante[(size_t)i] = lignecourante[(size_t)i-1];
						lignecourante[kin] = etmp;
					} // else Nobody
				} // else Nobody
			}
		} // else rien de supérieur à k dans l
		// donc rien à permuter
	}
#if 0
	std::cerr << "E PERMUTE: " << indpermut << " <--> " << k << " of  [";
	for(typename Vector::const_iterator refs =  lignecourante.begin();
	    refs != lignecourante.end() ;
	    ++refs )
		std::cerr << '(' << refs->first << ';' << refs->second << ')';
	std::cerr << "]" << std::endl;
#endif
}
#endif



// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

