// ==========================================================================
// Copyright(c)'1994-2015 by The Givaro group
// This file is part of Givaro.
// Givaro is governed by the CeCILL-B license under French law
// and abiding by the rules of distribution of free software.
// see the COPYRIGHT file for more details.
// Authors: T. Gautier
//          A. Breust (adapted)
// ==========================================================================

#ifndef __GIVARO_zpz16std_INL
#define __GIVARO_zpz16std_INL

#include "modular-defines.h"

namespace Givaro {

    // -------------
    // ----- Modular

    template<>
    inline Modular<int16_t, int16_t>::Residu_t
    Modular<int16_t, int16_t>::maxCardinality() { return 255u; } // 2^8 - 1

    template<>
    inline Modular<int16_t, uint16_t>::Residu_t
    Modular<int16_t, uint16_t>::maxCardinality() { return 255u; }

    template<>
    inline Modular<int16_t, uint32_t>::Residu_t
    Modular<int16_t, uint32_t>::maxCardinality() { return 32757u; } // 2^15 - 1

    template<>
    inline Modular<int16_t, int32_t>::Residu_t
    Modular<int16_t, int32_t>::maxCardinality() { return 32757u; }

    // ------------------------
    // ----- Classic arithmetic

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element& Modular<int16_t, COMP>::mul
    (Element& r, const Element& a, const Element& b) const
    {
        return  __GIVARO_MODULAR_INTEGER_MUL(r,_p,a,b);
    }

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element& Modular<int16_t, COMP>::sub
    (Element& r, const Element& a, const Element& b) const
    {
        return __GIVARO_MODULAR_INTEGER_SUB(r,_p,a,b);
    }

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element& Modular<int16_t, COMP>::add
    (Element& r, const Element& a, const Element& b) const
    {
        __GIVARO_MODULAR_INTEGER_ADD(r,_p,a,b);
        return r;
    }

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element& Modular<int16_t, COMP>::neg
    (Element& r, const Element& a) const
    {
        return __GIVARO_MODULAR_INTEGER_NEG(r,_p,a);
    }

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element& Modular<int16_t, COMP>::inv
    (Element& r, const Element& a) const
    {
        invext(r, a, int16_t(_p));
        return (r < 0)? r = static_cast<Element>(r + _p) : r;
    }

    template<typename COMP>
    inline bool Modular<int16_t, COMP>::isUnit(const Element& a) const 
    { 
        Element u,d; 
        invext(u,d,a,int16_t(_p)); 
        return isOne(d) || isMOne(d); 
    }

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element& Modular<int16_t, COMP>::div
    (Element& r, const Element& a, const Element& b) const
    {
        return mulin( inv(r,b), a );
    }

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element& Modular<int16_t, COMP>::mulin
    (Element& r, const Element& a) const
    {
        return __GIVARO_MODULAR_INTEGER_MULIN(r,_p,a);
    }

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element& Modular<int16_t, COMP>::divin
    (Element& r, const Element& a) const
    {
        Element ia;
        return mulin(r, inv(ia, a));
    }

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element& Modular<int16_t, COMP>::addin
    (Element& r, const Element& a) const
    {
        __GIVARO_MODULAR_INTEGER_ADDIN(r,_p,a);
        return r;
    }

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element& Modular<int16_t, COMP>::subin
    (Element& r, const Element& a) const
    {
        __GIVARO_MODULAR_INTEGER_SUBIN(r,_p,a);
        return r;
    }

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element& Modular<int16_t, COMP>::negin
    (Element& r) const
    {
        return __GIVARO_MODULAR_INTEGER_NEGIN(r,_p);
    }

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element& Modular<int16_t, COMP>::invin
    (Element& r) const
    {
        return inv(r, r);
    }

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element& Modular<int16_t, COMP>::axpy
    (Element& r, const Element& a, const Element& b, const Element& c) const
    {
        return __GIVARO_MODULAR_INTEGER_MULADD(r,_p,a,b,c);
    }

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element&  Modular<int16_t, COMP>::axpyin
    (Element& r, const Element& a, const Element& b) const
    {
        return __GIVARO_MODULAR_INTEGER_MULADDIN(r,_p,a,b);
    }

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element& Modular<int16_t, COMP>::maxpy
    (Element& r, const Element& a, const Element& b, const Element& c) const
    {
        int16_t tmp;
        __GIVARO_MODULAR_INTEGER_MUL(tmp,_p,a,b);
        __GIVARO_MODULAR_INTEGER_SUB(r,_p,c,tmp);
        return r;
    }

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element&  Modular<int16_t, COMP>::axmy
    (Element& r, const Element& a, const Element& b, const Element& c) const
    {
        return __GIVARO_MODULAR_INTEGER_MULSUB(r,_p,a,b,c);
    }

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element&  Modular<int16_t, COMP>::maxpyin
    (Element& r, const Element& a, const Element& b) const
    {
        __GIVARO_MODULAR_INTEGER_SUBMULIN(r,_p,a,b);
        return r;
    }

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element&  Modular<int16_t, COMP>::axmyin
    (Element& r, const Element& a, const Element& b) const
    {
        return __GIVARO_MODULAR_INTEGER_MULSUB(r,_p,a,b,r);
    }

    // ----------------------------------
    // ----- Classic arithmetic on arrays

    template<typename COMP>
    inline void Modular<int16_t, COMP>::mul
    (const size_t sz, Array r, constArray a, constArray b) const
    {
        for ( size_t i=sz ; --i ; ) {
            int16_t tmp;
            __GIVARO_MODULAR_INTEGER_MUL(tmp,_p,a[i],b[i]);
            r[i] = (Element)tmp;
        }
    }

    template<typename COMP>
    inline void Modular<int16_t, COMP>::mul
    (const size_t sz, Array r, constArray a, Element b) const
    {
        for ( size_t i=sz ; --i ; ) {
            int16_t tmp;
            __GIVARO_MODULAR_INTEGER_MUL(tmp,_p,a[i],b);
            r[i] = (Element)tmp;
        }
    }

    template<typename COMP>
    inline void Modular<int16_t, COMP>::div
    (const size_t sz, Array r, constArray a, constArray b) const
    {
        for ( size_t i=sz ; --i ; ) {
            div( r[i], a[i], b[i]);
        }
    }

    template<typename COMP>
    inline void Modular<int16_t, COMP>::div
    (const size_t sz, Array r, constArray a, Element b) const
    {
        Element ib;
        inv(ib, b);
        mul(sz, r, a, ib);
    }

    template<typename COMP>
    inline void Modular<int16_t, COMP>::add
    (const size_t sz, Array r, constArray a, constArray b) const
    {
        for ( size_t i=sz ; --i ; ) {
            int16_t tmp;
            __GIVARO_MODULAR_INTEGER_ADD(tmp,_p,a[i],b[i]);
            r[i] = (Element)tmp;
        }
    }

    template<typename COMP>
    inline void Modular<int16_t, COMP>::add
    (const size_t sz, Array r, constArray a, Element b) const
    {
        for ( size_t i=sz ; --i ; ) {
            int16_t tmp;
            __GIVARO_MODULAR_INTEGER_ADD(tmp,_p,a[i],b);
            r[i] = (Element)tmp;
        }
    }

    template<typename COMP>
    inline void Modular<int16_t, COMP>::sub
    (const size_t sz, Array r, constArray a, constArray b) const
    {
        for ( size_t i=sz ; --i ; ) {
            int16_t tmp;
            __GIVARO_MODULAR_INTEGER_SUB(tmp,_p,a[i],b[i]);
            r[i] = (Element)tmp;
        }
    }

    template<typename COMP>
    inline void Modular<int16_t, COMP>::sub
    (const size_t sz, Array r, constArray a, Element b) const
    {
        for ( size_t i=sz ; --i ; ) {
            int16_t tmp;
            __GIVARO_MODULAR_INTEGER_SUB(tmp,_p,a[i],b);
            r[i] = (Element)tmp;
        }
    }

    template<typename COMP>
    inline void Modular<int16_t, COMP>::neg
    (const size_t sz, Array r, constArray a) const
    {
        for ( size_t i=sz ; --i ; ) {
            int16_t tmp;
            __GIVARO_MODULAR_INTEGER_NEG(tmp,_p,a[i]);
            r[i] = (Element)tmp;
        }
    }

    template<typename COMP>
    inline void Modular<int16_t, COMP>::axpy
    (const size_t sz, Array r, constArray a, constArray x, constArray y) const
    {
        for ( size_t i=sz ; --i ; ) {
            int16_t tmp;
            __GIVARO_MODULAR_INTEGER_MULADD(tmp,_p,a[i],x[i],y[i]);
            r[i] = (Element)tmp;
        }
    }

    template<typename COMP>
    inline void Modular<int16_t, COMP>::axpyin
    (const size_t sz, Array r, constArray a, constArray x) const
    {
        for ( size_t i=sz ; --i ; ) {
            int16_t tmp = (int16_t)r[i];
            __GIVARO_MODULAR_INTEGER_MULADDIN(tmp,_p,a[i],x[i]);
            r[i] = (Element)tmp;
        }
    }

    template<typename COMP>
    inline void Modular<int16_t, COMP>::axmy
    (const size_t sz, Array r, constArray a, constArray x, constArray y) const
    {
        for ( size_t i=sz; i--; ) {
            int16_t tmp;
            __GIVARO_MODULAR_INTEGER_MULSUB(tmp,_p,a[i],x[i],y[i]);
            r[i] = (Element)tmp;
        }
    }

    template<typename COMP>
    inline void Modular<int16_t, COMP>::maxpyin
    (const size_t sz, Array r, constArray a, constArray x) const
    {
        for ( size_t i=sz ; --i ; ) {
            int16_t tmp = (int16_t)r[i];
            __GIVARO_MODULAR_INTEGER_SUBMULIN(tmp,_p,a[i],x[i]);
            r[i] = (Element)tmp;
        }
    }

    // --------------------
    // ----- Initialisation
    
    template<typename COMP> inline typename Modular<int16_t, COMP>::Element&
    Modular<int16_t, COMP>::init(Element& x) const
    {
        return x = zero;
    }
    
    template<typename COMP> inline typename Modular<int16_t, COMP>::Element&
    Modular<int16_t, COMP>::init (Element& x, const float y) const
    {
    x = static_cast<Element>(std::fmod(y, float(_p)));
    if (x < 0) x = static_cast<Element>(x + _p);
    return x;
}
    
    template<typename COMP> inline typename Modular<int16_t, COMP>::Element&
    Modular<int16_t, COMP>::init (Element& x, const double y) const
    {
    x = static_cast<Element>(std::fmod(y, double(_p)));
    if (x < 0) x = static_cast<Element>(x + _p);
    return x;
}
    
    template<typename COMP> inline typename Modular<int16_t, COMP>::Element&
    Modular<int16_t, COMP>::init (Element& x, const int32_t y) const
    {
    x = static_cast<Element>(y % int32_t(_p));
    if (x < 0) x = static_cast<Element>(x + _p);
    return x;
}
    
    template<typename COMP> inline typename Modular<int16_t, COMP>::Element&
    Modular<int16_t, COMP>::init (Element& x, const uint32_t y) const
    {
    x = static_cast<Element>(y % uint32_t(_p));
    return x;
}
    
    template<typename COMP> inline typename Modular<int16_t, COMP>::Element&
    Modular<int16_t, COMP>::init (Element& x, const int64_t y) const
    {
    x = static_cast<Element>(y % int64_t(_p));
    if (x < 0) x = static_cast<Element>(x + _p);
    return x;
}
    
    template<typename COMP> inline typename Modular<int16_t, COMP>::Element&
    Modular<int16_t, COMP>::init (Element& x, const uint64_t y) const
    {
    x = static_cast<Element>(y % uint64_t(_p));
    return x;
}
    
    template<typename COMP> inline typename Modular<int16_t, COMP>::Element&
    Modular<int16_t, COMP>::init (Element& x, const Integer& y) const
    {
    x = static_cast<Element>(y % _p);
    if (x < 0) x = static_cast<Element>(x + _p);
    return x;
}

    template<typename COMP>
    inline void Modular<int16_t, COMP>::init
    ( const size_t sz, Array r, constArray a ) const
    {
    for ( size_t i=sz ; --i ; )
        r[i] = a[i];
}

    template<typename COMP> inline  typename Modular<int16_t, COMP>::Element&
    Modular<int16_t, COMP>::assign ( Element& r, const Element& a ) const
    {
    return r = a;
}

    template<typename COMP>
    inline void Modular<int16_t, COMP>::assign
    ( const size_t sz, Array r, constArray a ) const
    {
    for ( size_t i=sz ; --i ; )
        r[i] = a[i];
}

    // ------------
    // ----- Reduce

    template<typename COMP> inline typename Modular<int16_t, COMP>::Element&
    Modular<int16_t, COMP>::reduce(Element& r, const Element& a) const
    {
    r = a % static_cast<Element>(_p);
    if (r < 0) r = static_cast<Element>(r + _p);
    return r;
}

    template<typename COMP> inline typename Modular<int16_t, COMP>::Element&
    Modular<int16_t, COMP>::reduce(Element& r) const
    {
    r = static_cast<Element>(r % _p);
    if (r < 0) r = static_cast<Element>(r + _p);
    return r;
}

    // ------------
    // ----- Arrays

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element& Modular<int16_t, COMP>::dotprod
    ( Element& r, const int bound, const size_t sz, constArray a, constArray b ) const
    {
    unsigned int stride = 1;
    if ((unsigned long)bound < GIVARO_MAXUINT16)
        stride = (unsigned int) (GIVARO_MAXULONG/((unsigned long)bound * (unsigned long)bound));
    unsigned long dot = (unsigned long)zero;
    if ((sz <10) && (sz <stride)) {
    for(  size_t i= sz; i--; )
        dot += (unsigned long)a[i] * (unsigned long)b[i];
    if (dot > _p) r = (Element)(dot % (uint16_t)_p);
    else r = (Element)dot;
    return r;
}
    unsigned int i_begin=0;
    stride &= (unsigned int)~0x1;
    if (stride ==0) {
    for(  size_t i = sz; i--; ) {
    dot += (unsigned long)a[i] * (unsigned long)b[i];
    if (dot>_p) dot %= _p;
}
    r = (Element)dot;
    return r;
}
    do {
    size_t min_sz = ((sz-i_begin) < stride ? (sz-i_begin) : stride);
    if ((min_sz & 0x1) !=0) {
    min_sz--;
    i_begin++;
    dot += (unsigned long)a++[min_sz] * (unsigned long)b++[min_sz];
}
    if (min_sz > 1)
        for(  size_t i= min_sz; i>0; --i, --i, ++a, ++a, ++b, ++b )
            {
    dot += (unsigned long)a[0] * (unsigned long)b[0];
    dot += (unsigned long)a[1] * (unsigned long)b[1];
}
    if (dot>_p) dot %= (uint16_t)_p;
    i_begin += (unsigned int) min_sz;
} while (i_begin <sz);

    return r = (Element)dot;
}

    template<typename COMP>
    inline typename Modular<int16_t, COMP>::Element& Modular<int16_t, COMP>::dotprod
    ( Element& r, const size_t sz, constArray a, constArray b ) const
    {
    return Modular<int16_t, COMP>::dotprod(r, _p, sz, a, b);
}


    //  a -> r: int16_t to double
    template<typename COMP>
    inline void
    Modular<int16_t, COMP>::i2d ( const size_t sz, double* r, constArray a ) const
    {
    for (size_t i=0; i<sz; ++i) r[i] = a[i];
}

    //  a -> r: double to int16_t
    template<typename COMP>
    inline void
    Modular<int16_t, COMP>::d2i ( const size_t sz, Array r, const double* a ) const
    {
    union d_2_l {
    double d;
    int32_t r[2];
};
    //  static const double offset = 4503599627370496.0; // 2^52
    double offset = 4503599627370496.0; // 2^52
    for (size_t i=0; i<sz; ++i)
        {
    d_2_l tmp;
    // - normalization: put fractional part at the end of the representation
    tmp.d = a[i] + offset;
    r[i] = (Element) tmp.r[1];
    if (r[i] <_p)
        r[i] = Element(r[i]%_p);
}
    //    r[i] = (tmp.r[1] <_p ? tmp.r[1] : tmp.r[1]-_p);
    //    r[i] = (r[i] <_p ? r[i] : r[i]%_p);
    //    r[i] = (tmp.r[1] <_p ? tmp.r[1] : tmp.r[1]%_p);
}



    // -- Input: (z, <_p>)
    template<typename COMP>
    inline std::istream& Modular<int16_t, COMP>::read (std::istream& s)
    {
    char ch;
    s >> std::ws >> ch;
    if (ch != '(')
        std::cerr << "GivBadFormat(Modular<int16_t, COMP>::read: syntax error: no '('))" << std::endl;

    s >> std::ws >> ch;
    if (ch != 'z')
        std::cerr << "GivBadFormat(Modular<int16_t, COMP>::read: bad domain object))" << std::endl;

    s >> std::ws >> ch;
    if (ch != ',')
        std::cerr << "GivBadFormat(Modular<int16_t, COMP>::read: syntax error: no ',')) " << std::endl;

    s >> std::ws >> _p;

    s >> std::ws >> ch;
    if (ch != ')')
        std::cerr << "GivBadFormat(Modular<int16_t, COMP>::read: syntax error: no ')')) " << std::endl;

    return s;
}

    template<>
    inline std::ostream& Modular<int16_t, int16_t>::write (std::ostream& s) const
    {
    return s << "Modular<int16_t, uint16_t> modulo " << residu();
}

    template<>
    inline std::ostream& Modular<int16_t, uint16_t>::write (std::ostream& s) const
    {
    return s << "Modular<int16_t, uint16_t> modulo " << residu();
}

    template<>
    inline std::ostream& Modular<int16_t, int32_t>::write (std::ostream& s) const
    {
    return s << "Modular<int16_t, uint32_t> modulo " << residu();
}

    template<>
    inline std::ostream& Modular<int16_t, uint32_t>::write (std::ostream& s) const
    {
    return s << "Modular<int16_t, uint32_t> modulo " << residu();
}

    template<typename COMP>
    inline std::istream& Modular<int16_t, COMP>::read (std::istream& s, Element& a) const
    {
    Integer tmp;
    s >> tmp;
    init(a, tmp);
    return s;
}

    template<typename COMP>
    inline std::ostream& Modular<int16_t, COMP>::write (std::ostream& s, const Element a) const
    {
    return s << a;
}

} // namespace Givaro

#endif // __GIVARO_zpz16std_INL
    // vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
