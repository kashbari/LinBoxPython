// ==========================================================================
// Copyright(c)'1994-2015 by The Givaro group
// This file is part of Givaro.
// Givaro is governed by the CeCILL-B license under French law
// and abiding by the rules of distribution of free software.
// see the COPYRIGHT file for more details.
// Authors: J.G. Dumas
// Time-stamp: <27 Sep 16 17:21:35 Jean-Guillaume.Dumas@imag.fr>
// ==========================================================================
//
//  Modified by Pascal Giorgi on 2002/02/13  (pascal.giorgi@ens-lyon.fr)

/*! @file givzpz16table1.h
 * @ingroup zpz
 * @brief  Arithmetic on Z/pZ, with tabulation of operations.
 */

#ifndef __GIVARO_modular_log16_H
#define __GIVARO_modular_log16_H

#include "givaro/givinteger.h"
#include "givaro/givbasictype.h"
#include "givaro/giverror.h"
#include "givaro/givarray0.h"
#include "givaro/givranditer.h"
#include "givaro/modular-general.h"

namespace Givaro
{
    struct Log16;

    /*! @brief This class implement the standard arithmetic with Modulo Elements.
     * - The representation of an integer a in Zpz is the value a % p
     * - p max is 16381
     * - p si supposed to be prime
     * .
     */

    template<>
    class Modular<Log16, Log16>
    {
    public:
        // ----- Exported Types and constantes
        typedef Modular<Log16> Self_t;
        typedef uint16_t Residu_t;                    // - type to store residue
        enum { size_rep = sizeof(Residu_t) };      // - size of the storage type

        // ----- Representation of Element of the domain Modular:
        typedef int16_t Power_t;
        typedef Power_t Rep;
        typedef int16_t Element;
        typedef Element* Element_ptr ;
        typedef const Element* ConstElement_ptr;


        // ----- Representation of vector of the Element
        typedef Residu_t* Array;
        typedef const Residu_t* constArray;

        // ----- Constructor /destor
        inline Modular( Residu_t p = 2 );
        inline Modular( const Modular<Log16>& F);
        inline ~Modular();

        bool operator==( const Modular<Log16>& BC) const { return _p == BC._p;}
        bool operator!=( const Modular<Log16>& BC) const { return _p != BC._p;}

        Modular<Log16>& operator=( const Modular<Log16>& F);

        // ----- Access to the modulus
        Residu_t residu() const;
        Residu_t size() const { return _p;}

        inline Residu_t characteristic() const { return _p; }
        inline Residu_t cardinality() const { return _p; }
        template<class T> inline T& characteristic(T& p) const { return p = _p; }
        template<class T> inline T& cardinality(T& p) const { return p = _p; }

        // ----- Convert from Element to int
        int16_t& convert( int16_t& x , const Rep a) const {
            return x = Rep((a >= _p)?0:_tab_rep2value[a]);
        }
        uint16_t& convert( uint16_t& x , const Rep a) const {
            return x = Residu_t((a >= _p)?0:_tab_rep2value[a]);
        }
        uint32_t & convert( uint32_t& x , const Rep a) const {
            return x = uint32_t((a >= _p)?0:_tab_rep2value[a]);
        }
        int32_t& convert( int32_t& x , const Rep a)  const {
            return x = (int32_t)((a >= _p)?0:_tab_rep2value[a]);
        }
        uint64_t & convert( uint64_t& x , const Rep a) const {
            return x = ((a >= _p)?0:_tab_rep2value[a]);
        }
        int64_t& convert( int64_t& x , const Rep a)  const {
            return x = (int64_t)((a >= _p)?0:_tab_rep2value[a]);
        }

        double& convert( double& x , const Rep a)  const {
            return x = (double)((a >= _p)?0:_tab_rep2value[a]);
        }
        Integer& convert(Integer& i, const Rep a) const {
            return i = (Integer)((a >= _p)?0:_tab_rep2value[a]);
        }


        // initialized by a degree of the generator.
        Rep& init( Rep& r ) const;
        Rep& init( Rep& r, const int64_t a) const;
        Rep& init( Rep& a, const int32_t i) const ;
        Rep& init( Rep& r, const uint64_t a) const;
        Rep& init( Rep& a, const uint32_t i) const ;
        Rep& init( Rep& a, const Integer& i) const ;
        Rep& init( Rep& a, const double i) const;
        Rep& init( Rep& a, const float i) const;

        // Specials
        Rep& init( Rep& a, const int16_t i) const ;
        Rep& init( Rep& r, const uint16_t a) const;

        // -- Assignment :  r = a
        Rep& assign (Rep& r, const Rep a) const;
        void assign ( const size_t sz, Array r, constArray a ) const;

        // ----- Misc methods
        bool iszero( const Rep a ) const;
        bool isone ( const Rep a ) const;
        bool ismone ( const Rep a ) const;
        bool isZero( const Rep a ) const;
        bool isOne ( const Rep a ) const;
        bool isUnit ( const Rep a ) const;
        bool isMOne ( const Rep a ) const;
        size_t length ( const Rep a ) const;


        // ----- Equality between two Elements
        bool areEqual( const Element& a, const Element& b) const {return a==b;}


        // ----- Operations with reduction: r <- a op b mod p, r <- op a mod p
        Rep& mul (Rep& r, const Rep a, const Rep b) const;
        Rep& div (Rep& r, const Rep a, const Rep b) const;
        Rep& add (Rep& r, const Rep a, const Rep b) const;
        Rep& sub (Rep& r, const Rep a, const Rep b) const;
        Rep& neg (Rep& r, const Rep a) const;
        Rep& inv (Rep& r, const Rep a) const;

        Rep& mulin (Rep& r, const Rep a) const;
        Rep& divin (Rep& r, const Rep a) const;
        Rep& addin (Rep& r, const Rep a) const;
        Rep& subin (Rep& r, const Rep a) const;
        Rep& negin (Rep& r) const;
        Rep& invin (Rep& r) const;


        // ----- Operations with reduction: r <- a op b mod p, r <- op a mod p
        void mul (const size_t sz, Array r, constArray a, constArray b) const;
        void mul (const size_t sz, Array r, constArray a, Rep b) const;

        void div (const size_t sz, Array r, constArray a, constArray b) const;
        void div (const size_t sz, Array r, constArray a, Rep b) const;

        void add (const size_t sz, Array r, constArray a, constArray b) const;
        void add (const size_t sz, Array r, constArray a, Rep b) const;

        void sub (const size_t sz, Array r, constArray a, constArray b) const;
        void sub (const size_t sz, Array r, constArray a, Rep b) const;

        void neg (const size_t sz, Array r, constArray a) const;
        void inv (const size_t sz, Array r, constArray a) const;

        // -- axpy: r <- a * x + y mod p
        Rep& axpy   (Rep& r, const Rep a, const Rep b, const Rep c) const;
        Rep& axpyin (Rep& r, const Rep a, const Rep b) const;
        void axpy
        (const size_t sz, Array r, constArray a, constArray x, constArray c) const;
        void axpyin
        (const size_t sz, Array r, constArray a, constArray x) const;

        // -- axmy: r <- a * x - y mod p
        Rep& axmy   (Rep& r, const Rep a, const Rep b, const Rep c) const;
        void axmy
        (const size_t sz, Array r, constArray a, constArray x, constArray c) const;
        // -- axmyin: r <- a * b - r  mod p
        Rep& axmyin (Rep& r, const Rep a, const Rep b) const;
        // void axmyin (const size_t sz, Array r, constArray a, constArray x) const;

        // -- maxpy: r <- c - a * b mod p
        Rep& maxpy   (Rep& r, const Rep a, const Rep b, const Rep c) const;
        // -- maxpyin: r <- r - a * b mod p
        Rep& maxpyin (Rep& r, const Rep a, const Rep b) const;
        void maxpyin (const size_t sz, Array r, constArray a, constArray x) const;


        // <- \sum_i a[i], return 1 if a.size() ==0,
        Rep& reduceadd ( Rep& r, const size_t sz, constArray a ) const;

        // <- \prod_i a[i], return 1 if a.size() ==0,
        Rep& reducemul ( Rep& r, const size_t sz, constArray a ) const;

        // <- \sum_i a[i] * b[i]
        Rep& dotprod ( Rep& r, const size_t sz, constArray a, constArray b ) const;
        Rep& dotprod ( Rep& r, const int32_t bound, const size_t sz, constArray a, constArray b ) const;

        // ----- a -> r: uint16_t to double
        void i2d ( const size_t sz, double* r, constArray a ) const;

        // ----- a -> r % p: double to uint16_t % p
        void d2i ( const size_t sz, Array r, const double* a ) const;

        // ----- Random generators
        typedef ModularRandIter<Self_t> RandIter;
        typedef GeneralRingNonZeroRandIter<Self_t> NonZeroRandIter;
        template< class Random > Element& random(Random& g, Element& r) const
        { return init(r, g()); }
        template< class Random > Element& nonzerorandom(Random& g, Element& a) const
        { while (isZero(init(a, g())))
                ;
            return a; }


        // --- IO methods
        std::istream& read ( std::istream& s );
        std::ostream& write( std::ostream& s ) const;
        std::istream& read ( std::istream& s, Rep& a ) const;
        std::ostream& write( std::ostream& s, const Rep a ) const;

    protected:
        // -- data representation of the domain:
        Residu_t  _p;  // the modulo
        Residu_t  _pmone;  // _p -1
        Power_t * _tab_value2rep;    // table for convertion
        Residu_t* _tab_rep2value;    // table for convertion
        Power_t* _tab_mul;      // table for mul
        Power_t* _tab_div;      // table for div
        Power_t* _tab_neg;      // table for neg
        Power_t* _tab_addone;   // table for ei+1
        Power_t* _tab_subone;   // table for -(ei+1)
        Power_t* _tab_mone;   // table for ei+1
        Power_t* _tab_pone;   // table for -(ei+1)
        int* numRefs;

    public:
        // ----- Constantes
        const Element zero;
        const Element one;
        const Element mOne;

    public:

        static inline Residu_t maxCardinality() { return 16381; }
        static inline Residu_t minCardinality() { return 2; }

    };

} // namespace Givaro

#include "givaro/modular-log16.inl"

#endif // __GIVARO_modular_log16_H

