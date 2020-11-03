/* rmint/arith.h - Arithmetic functions for rmint

Copyright Université Joseph Fourier - Grenoble
Contributors :
    Alexis BREUST (alexis.breust@gmail.com 2014)
    Christophe CHABOT (christophechabotcc@gmail.com 2011)


This software is a computer program whose purpose is to provide an fixed precision arithmetic library.

This software is governed by the CeCILL-B license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL-B
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy, 
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software, 
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL-B license and that you accept its terms.
*/


#ifndef RMINT_BASIC_ARITH_EXP_H
#define RMINT_BASIC_ARITH_EXP_H

#include "rmbrmint.h"
#include "rmbreduc.h"

// --------------------------------------------------------------
// ----------------------- DEFINTIONS ---------------------------

namespace RecInt
{
    template <size_t K> void exp(rmint<K, MGI>&, const rmint<K, MGI>&, const ruint<K>&);
    template <size_t K, typename T> __RECINT_IS_UNSIGNED(T, void) exp(rmint<K, MGI>&, const rmint<K, MGI>&, const T&);
}


// --------------------------------------------------------------
// --------------------- Implementation -------------------------

namespace RecInt
{
    // a = b^c mod a.p
    template <size_t K>
    inline void exp(rmint<K, MGI>& a, const rmint<K, MGI>& b, const ruint<K>& c) {
        exp_mod(a.Value, b.Value, c, a.p);
    }

    // a = b^c mod a.p
    template <size_t K, typename T>
    inline __RECINT_IS_UNSIGNED(T, void) exp(rmint<K, MGI>& a, const rmint<K, MGI>& b, const T& c) {
        exp_mod(a.Value, b.Value, c, a.p);
    }
}

#endif

