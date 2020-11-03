// ==========================================================================
// $Source
// Copyright(c)'1994-2009 by The Givaro group
// This file is part of Givaro.
// Givaro is governed by the CeCILL-B license under French law
// and abiding by the rules of distribution of free software.
// see the COPYRIGHT file for more details.
// Authors: T. Gautier
// $Id
// ==========================================================================
namespace Givaro {
#pragma message "#warning this file will probably not compile"



template<>
inline void VectorDom<Modular<int16_t>,Dense>::dot
  ( Type_t& res, const Rep& op1, const Rep& op2) const
{
  size_t sz = dim(op1);
  const Modular<int16_t>& domain = subdomain();
  domain.dotprod( res, sz, op1.baseptr(), op2.baseptr() );
}


} // Givaro
