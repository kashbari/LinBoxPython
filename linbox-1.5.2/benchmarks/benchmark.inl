/* Copyright (C) 2013 LinBox
 * Written by Brice Boyer (briceboyer) <boyer.brice@gmail.com>
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========
 */

/*! @file   benchmarks/benchmark.inl
 * @ingroup benchmarks
 * @brief   Common header to ease benchmarking routines.
 * We provide a class to easily populate and plot files that represent
 * benchmarks.
 *
 * We use <a href="http://www.gnuplot.info/">gnuplot</a> for the plotting part
 * or LaTeX to provide tables.  A minimum knowledge of \c Gnuplot is a plus but
 * the \c benchmark-* files should provide enough examples for creating
 * standard (not too fancy) plots.
 *
 * We fall back to plain latex tabulars when gnuplot is not available. We plot
 * graphs in files whose name is extended by a random 8-char string to avoid
 * écraser files.
 *
 */

#ifndef __LINBOX_benchmarks_benchmark_INL
#define __LINBOX_benchmarks_benchmark_INL

#include <stdlib.h>
#include "linbox/integer.h"
#include "linbox/util/debug.h"
#include "tests/test-common.h"
#include "linbox/util/timer.h"
#include <string>
#include <fstream>




//
// Chrono
//

namespace LinBox {

	template<class MyTimer>
	Chrono<MyTimer>::Chrono()  :
		_chrono_(),
		_times_(0),
		_total_(0)
	{
		_chrono_.clear();
	}

	template<class MyTimer>
	Chrono<MyTimer>::~Chrono() {}

	template<class MyTimer>
	void Chrono<MyTimer>::clear()
	{
		_chrono_.clear();
		_times_.resize(0);
		_total_ = 0;
	}

	template<class MyTimer>
	void Chrono<MyTimer>::start()
	{
		_chrono_.start();
	}

	template<class MyTimer>
	void Chrono<MyTimer>::stop()
	{
		_chrono_.stop();
		double split = _chrono_.userElapsedTime();
		_times_.push_back( split );
		_total_ += split ;
		_chrono_.clear();
	}

	template<class MyTimer>
	double Chrono<MyTimer>::time() const
	{
		return _total_ ;
	}

	template<class MyTimer>
	dvector_t Chrono<MyTimer>::times() const
	{
		return _times_ ;
	}


} // LinBox

//
// PlotData
//

namespace LinBox {

	// set Series

		template<>
		svector_t & PlotData::refSeries(const size_t & i, Point::Labels)
		{
			linbox_check(i<size());
			return (refSeries(i).PointLabels) ;
		}

		template<>
		dvector_t & PlotData::refSeries(const size_t & i, Point::Values)
		{
			return (refSeries(i).Values) ;
		}

		template<>
		dvector_t & PlotData::refSeries(const size_t & i, Point::Points)
		{
			return (refSeries(i).Points) ;
		}

		template<>
		dvector_t & PlotData::refSeries(const size_t & i, Point::Times)
		{
			return (refSeries(i).Times) ;
		}

	// get Series
		template<>
		const svector_t & PlotData::getSeries(const size_t & i, Point::Labels) const
		{
			linbox_check(i<size());
			return (getSeries(i).PointLabels) ;
		}

		template<>
		const dvector_t & PlotData::getSeries(const size_t & i, Point::Values) const
		{
			return (getSeries(i).Values) ;
		}

		template<>
		const dvector_t & PlotData::getSeries(const size_t & i, Point::Points) const
		{
			return (getSeries(i).Points) ;
		}

		template<>
		const dvector_t & PlotData::getSeries(const size_t & i, Point::Times) const
		{
			return (getSeries(i).Times) ;
		}

} // LinBox

#endif // __LINBOX_benchmarks_benchmark_INL

// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
