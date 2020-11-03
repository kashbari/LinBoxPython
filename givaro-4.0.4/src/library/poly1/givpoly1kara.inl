// ==========================================================================
// $Source: /var/lib/cvs/Givaro/src/library/poly1/givpoly1kara.inl,v $
// Copyright(c)'1994-2011 by The Givaro group
// This file is part of Givaro.
// Givaro is governed by the CeCILL-B license under French law
// and abiding by the rules of distribution of free software.
// see the COPYRIGHT file for more details.
// Authors: J-G Dumas
// $Id: givpoly1kara.inl,v 1.3 2011-11-08 10:38:00 jgdumas Exp $
// ==========================================================================
#ifndef __GIVARO_poly1_kara_INL
#define __GIVARO_poly1_kara_INL

namespace Givaro {

#ifndef KARA_THRESHOLD
#define KARA_THRESHOLD 50
#endif

#ifndef SQR_THRESHOLD
#define SQR_THRESHOLD 50
#endif

#ifndef GIVMIN
#define GIVMIN(a,b) ((a)<(b)?(a):(b))
#endif

// forces standard multiplication
template <class Domain>
inline typename Poly1Dom<Domain,Dense>::Rep& Poly1Dom<Domain,Dense>::stdmul( Rep& R, const Rep& P, const Rep& Q ) const
{
	const size_t sP = P.size();
	const size_t sQ = Q.size();
	if ((sQ ==0) || (sP ==0)) { R.resize(0); return R; }
	const size_t sR = sP+sQ-1;
	if (sR != R.size() ) R.resize(sR);

 	stdmul(R, R.begin(), R.end(),
            P, P.begin(), P.end(),
            Q, Q.begin(), Q.end());

    return setdegree(R);
}

// forces FIRST recursive level with Karatsuba multiplication
template <class Domain>
inline typename Poly1Dom<Domain,Dense>::Rep& Poly1Dom<Domain,Dense>::karamul( Rep& R, const Rep& P, const Rep& Q ) const
{
	const size_t sP = P.size();
	const size_t sQ = Q.size();
	if ((sQ ==0) || (sP ==0)) { R.resize(0); return R; }
	const size_t sR = sP+sQ-1;
	if (sR != R.size()) R.resize(sR);

 	karamul(R, R.begin(), R.end(),
            P, P.begin(), P.end(),
            Q, Q.begin(), Q.end());

    return setdegree(R);
}

// Generic mul with choices between standard and Karatsuba multiplication
// Multiplies between the iterator bounds.
template <class Domain>
inline typename Poly1Dom<Domain,Dense>::Rep& Poly1Dom<Domain,Dense>::mul(
    Rep& R, const RepIterator Rbeg, const RepIterator Rend,
    const Rep& P, const RepConstIterator Pbeg, const RepConstIterator Pend,
    const Rep& Q, const RepConstIterator Qbeg, const RepConstIterator Qend ) const {

    if ( ( (Pend-Pbeg)> KARA_THRESHOLD ) &&
         ( (Qend-Qbeg)> KARA_THRESHOLD) )
        return karamul(R, Rbeg, Rend,
                       P, Pbeg, Pend,
                       Q, Qbeg, Qend);
    else
        return stdmul(R, Rbeg, Rend,
                      P, Pbeg, Pend,
                      Q, Qbeg, Qend);
}


// Generic sqr with choices between standard and recursive multiplication
// Multiplies between the iterator bounds.
template <class Domain>
inline typename Poly1Dom<Domain,Dense>::Rep& Poly1Dom<Domain,Dense>::sqr(
    Rep& R, const RepIterator Rbeg, const RepIterator Rend,
    const Rep& P, const RepConstIterator Pbeg, const RepConstIterator Pend) const {

    Type_t two; _domain.init(two);
    _domain.add(two, _domain.one, _domain.one);

    if ( (Pend-Pbeg)> SQR_THRESHOLD )
        return sqrrec(R, Rbeg, Rend,
                      P, Pbeg, Pend,
                      two);
    else
        return stdsqr(R, Rbeg, Rend,
                      P, Pbeg, Pend,
                      two);
}




// Standard multiplication between iterator bounds
template <class Domain>
inline typename Poly1Dom<Domain,Dense>::Rep& Poly1Dom<Domain,Dense>::stdmul(
    Rep& R, const RepIterator Rbeg, const RepIterator Rend,
    const Rep& P, const RepConstIterator Pbeg, const RepConstIterator Pend,
    const Rep& Q, const RepConstIterator Qbeg, const RepConstIterator Qend ) const {

	RepConstIterator ai=Pbeg,bi=Qbeg;
	RepIterator ri=Rbeg, rig=Rbeg;
	if (_domain.isZero(*ai))
		for(;bi!=Qend;++bi,++ri)
			_domain.assign(*ri,_domain.zero);
	else
		for(;bi!=Qend;++bi,++ri)
			if (_domain.isZero(*bi))
				_domain.assign(*ri, _domain.zero);
			else
				_domain.mul(*ri,*ai,*bi);

	for(;ri!=Rend;++ri)
		_domain.assign(*ri,_domain.zero);
	for(++ai,++rig;ai!=Pend;++ai,++rig)
		if (! _domain.isZero(*ai))
			for(ri=rig,bi=Qbeg;bi!=Qend;++bi,++ri)
				_domain.axpyin(*ri,*ai,*bi);
	return R;
}

template <class Domain>
inline typename Poly1Dom<Domain,Dense>::Rep& Poly1Dom<Domain,Dense>::karamul( Rep& R, const RepIterator Rbeg, const RepIterator Rend, const Rep& P, const RepConstIterator Pbeg, const RepConstIterator Pend, const Rep& Q, const RepConstIterator Qbeg, const RepConstIterator Qend ) const
{
    // Initialize R to zero
    for(RepIterator ri=Rbeg; ri!= Rend; ++ri) _domain.assign(*ri,_domain.zero);


    const size_t halfP = (size_t) ((Pend-Pbeg)>>1);
    const size_t halfQ = (size_t) ((Qend-Qbeg)>>1);
    const size_t half = GIVMIN(halfP, halfQ);
    const size_t halfR = half<<1;

    const RepConstIterator Pmid=Pbeg+(ssize_t)half;		// cut P in halves
    const RepConstIterator Qmid=Qbeg+(ssize_t)half;		// cut Q in halves
    const RepIterator Rmid=Rbeg+(ssize_t)halfR;			// cut R in halves

    mul(R, Rbeg, Rmid, 				// Recursive dynamic choice
        P, Pbeg, Pmid,
        Q, Qbeg, Qmid);				// PlQl in first storage part of R

    mul(R, Rmid, Rend,				// Recursive dynamic choice
        P, Pmid, Pend,
        Q, Qmid, Qend);				// PhQh in second storage part of R

    Rep PHPL;
    for(RepConstIterator PHi=Pmid; PHi!=Pend; ++PHi)
        PHPL.push_back(*PHi);
    subin(PHPL, PHPL.begin(), P, Pbeg, Pmid);	// Ph - Pl
    setdegree(PHPL);

    Rep QHQL;
    for(RepConstIterator QHi=Qmid; QHi!=Qend; ++QHi)
        QHQL.push_back(*QHi);
    subin(QHQL, QHQL.begin(), Q, Qbeg, Qmid);	// Qh - Ql
    setdegree(QHQL);

    Rep M;
    mul(M, 					// Recursive dynamic choice
        PHPL,
        QHQL);					// (Ph-Pl)(Qh-Ql)
    setdegree(M);

    subin(M, M.begin(), M.end(), R, Rbeg, Rmid);// -= PlQl
    setdegree(M);

    subin(M, M.begin(), M.end(), R, Rmid, Rend);// -= PhQh
    setdegree(M);


    RepIterator ri=Rbeg+(ssize_t)half;
    RepConstIterator mi=M.begin();		// update R with mid product
    for( ; mi != M.end(); ++ri, ++mi) _domain.subin(*ri, *mi);

    return R;
}


template <class Domain>
inline typename Poly1Dom<Domain,Dense>::Rep& Poly1Dom<Domain,Dense>::sqrrec( Rep& R, const RepIterator Rbeg, const RepIterator Rend, const Rep& P, const RepConstIterator Pbeg, const RepConstIterator Pend, const Type_t& two) const
{
    // Initialize R to zero
    for(RepIterator ri=Rbeg; ri!= Rend; ++ri) _domain.assign(*ri,_domain.zero);


    const size_t half = (size_t) ((Pend-Pbeg)>>1);
    const size_t halfR = (half<<1);

    const RepConstIterator Pmid=Pbeg+(ssize_t)half;  // cut P in halves
    const RepIterator Rmid=Rbeg+(ssize_t)halfR;	// cut R in halves

    sqr(R, Rbeg, Rmid-1, 			// Recursive dynamic choice
        P, Pbeg, Pmid);				// Pl^2 in first storage part of R

    sqr(R, Rmid, Rend,				// Recursive dynamic choice
        P, Pmid, Pend);				// Ph^2 in second storage part of R

    Rep M(P.size());
    mul(M, M.begin(), M.end(),                  // Recursive dynamic choice
        P, Pbeg, Pmid,
        P, Pmid, Pend);
    setdegree(M);
    this->mulin(M,two);

    RepIterator ri=Rbeg+(ssize_t)half;
    RepConstIterator mi=M.begin();		// update R with mid product
    for( ; mi != M.end(); ++ri, ++mi) _domain.addin(*ri, *mi);

    return R;
}

// Standard square
template <class Domain>
inline typename Poly1Dom<Domain,Dense>::Rep& Poly1Dom<Domain,Dense>::stdsqr(
    Rep& R, const RepIterator Rbeg, const RepIterator Rend,
    const Rep& P, const RepConstIterator Pbeg, const RepConstIterator Pend,
    const Type_t& two) const {

    _domain.mul(*Rbeg, *Pbeg, *Pbeg);

    RepIterator rit(Rbeg);
    RepConstIterator pit=Pbeg;
    for(++rit,++pit; rit != Rend; ++pit, ++rit) {

        _domain.assign(*rit,_domain.zero);

        RepConstIterator backpit=pit, forpit=pit;
        for(--backpit ; forpit != Pend; --backpit, ++forpit) {
            _domain.axpyin(*rit, *backpit, *forpit);
            if (backpit == Pbeg) break;
        }


        _domain.mulin(*rit, two);

        ++rit;

        _domain.assign(*rit,_domain.zero);

        backpit=pit, forpit=pit;
        for(--backpit,++forpit; forpit != Pend; --backpit, ++forpit)
        {
            _domain.axpyin(*rit, *backpit, *forpit);
            if (backpit == Pbeg) break;
        }
        _domain.mulin(*rit, two);
        _domain.axpyin(*rit, *pit, *pit);

    }



//     for(size_t i=0; i<dR; ++i) {
//             // even coefficients
//         _domain.assign(R[i],_domain.zero);
//         size_t iot(i>>1);
//         size_t firstj( i>dP ? i-dP : 0);
//         for(size_t j=firstj; j<iot; ++j)
//             _domain.axpyin(R[i], P[j], P[i-j]);
//         _domain.mulin(R[i], two);
//         _domain.axpyin(R[i],P[iot],P[iot]);

//             // odd coefficients
//         ++i; ++iot;
//         _domain.assign(R[i],_domain.zero);
//         firstj = i>dP ? i-dP : 0;
//         for(size_t j=firstj; j<iot; ++j) {
//             _domain.axpyin(R[i], P[j], P[i-j]);
//         }
//         _domain.mulin(R[i], two);

//     }
//     _domain.mul(R[dR],P[dP],P[dP]);


	return R;
}



}
#endif // __GIVARO_poly1_kara_INL
