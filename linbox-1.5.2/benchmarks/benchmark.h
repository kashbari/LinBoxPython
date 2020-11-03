/* Copyright (C) 2011 LinBox
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

/*! @file   benchmarks/benchmark.h
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

#ifndef __LINBOX_benchmarks_benchmark_H
#define __LINBOX_benchmarks_benchmark_H
// here classes TimeWatcher, PlotStyle, PlotData are declared

#include "linbox/linbox-config.h"
#include <stdlib.h>
#include "linbox/integer.h"
#include "linbox/util/debug.h"
#include "tests/test-common.h"
#include "linbox/util/timer.h"
#include <string>
#include <fstream>
#include <iomanip> // setprecision

#include <sys/utsname.h>
#include <ctime>
#ifdef __LINBOX_HAVE_TINYXML2
#include <tinyxml2.h>
#endif

#include "benchmark-utils.h"
#include "benchmark-metadata.h"


//
// TimeWatcher
//

namespace LinBox {
	/*! Helper.
	 * This helper has several functions :
	 *   - Records the timings
	 *   - predict the execution time for the next experiment
	 *   - helps producing enough experiments (but not too much and not too time consuming) for producing a valid measure.
	 *   .
	 *   @warning if the timings are too short, this may not be accurate.
	 *
	 *   See member function help for more information.
	 */
	class TimeWatcher  {
	private :
		dvector_t  *    Points_; //!< Points data. If <code>Points_[i] = x</code>, then <code>Values_[i]=f(x)<code>.
		dvector_t  *    Values_; //!< Time data. See \p Points_ .

		size_t       MaxRepet_; //!< Maximum number of repetitions of timings
		size_t       MinRepet_; //!< Minimum number of repetitions of timings
		double         MaxTime_; //!< Maximum time to be spent on repetitions (after MinRepet_ iters have been done)
		double       AbortTime_; //!< Time to abort a series of computation.
		bool           aborted_; //!< abort any subsequent computation


	public:
		/*! constructor.
		 * Inits the time watcher with a pair of points/values
		 * @param pts vector of points
		 * @param vals vector of times
		 */
		TimeWatcher (dvector_t & pts, dvector_t & vals) ;

		//! Null Constructor. The pointers are intialised to NULL
		TimeWatcher () ;

		void init(dvector_t & pts, dvector_t & vals);

		//! returns the vector of abscissa (points)
		dvector_t & refX() ;

		//! returns the vector of ordiantes (values)
		dvector_t & refY() ;

		/** Prediction for the next experiment time.
		 * It is assumed that \c predict(0)=0. If Curent_<3, a linear,
		 * then quadratic fit is done. Other wise, a cubic fit is
		 * performed.
		 * @param x the next evaluation point.
		 * @return f(x) where f tries to fit the points : \f$ f(\mathtt{Data\_}[0][0..\mathtt{Current\_}-1]) \approx  refY()[0..\mathtt{Current\_}-1]\f$
		 */
		double predict(double x) ;

		/*! @brief Watches a timer and a number and repet and signals if over.
		 *
		 * We want at least 2 repetions but not more than maxtime spent on timing.
		 *
		 * @param repet number of previous repetitions. Should be 0 on the first time \c keepon is called.
		 * @param tim timer to watch
		 * @param maxtime maximum time (in seconds) until \c watchon tells stop.
		 * @return \c true if we conditions are not met to stop, \c false otherwise.
		 * @pre \c tim should have been started previously !
		 *
		 */
		bool keepon(size_t & repet, double tim, bool usePrediction = false) ;

		//! size
		size_t size() const;

		//! clear the pointers (not the settings)
		void clear();

	} ; // TimeWatcher
} // LinBox

//
// DataSeries
//

namespace LinBox {
	/** @brief this structure holds a bunch of timings.
	 * It collects the points, the time spent at each point and a measure
	 * (for instance mflops).
	 * @todo Times and Values could be dmatrix_t (and mergeable)
	 */
	struct DataSeries {
		svector_t   PointLabels ; //!< points abscisa, values for the x axis. Used in legend for the X axis.
		dvector_t   Points      ; //!< points abscisa, values for the x axis. Used in TimeWatcher (for instance, if PointLabels are the names of sparse matrices, Points would be their number of non zeros, or 1,2,3,... or whatever relevant for predicting time)
		dvector_t   Times       ; //!< actual computation times.
		dvector_t   Values      ; //!< actual data to be plotted (for instance mflops)
		svector_t   UID         ; //!< unique id of a point.

		//! Constructor
		DataSeries() ;

		~DataSeries() ;

#if 0
		/** @brief resize
		 * @param n new size
		 * @pre the size before was n-1
		 */
		void resize(const size_t & n);
#endif

		//! Size of the series of measurements.
		size_t size() const;

		//! add some new data.
		void push_back(const std::string & nam, const double & val, const double & x = NAN, const double &y = NAN);

		// use it but change time
		// void useMetadata(const MetaData & m) ;
	}; // DataSeries

} // LinBox

/* ********************** */
/*    Plot structures     */
/* ********************** */

//
// PlotStyle
//

namespace LinBox {

	/*! @brief Represents a table of values to plot (2D).
	 * list of values are reprensented by vectors.  the table is a vector
	 * of these vectors.
	 *
	 * @warning NaN, inf are used as missing data. More genenally
	 * we could store data in strings.
	 * @todo Allow for 'speed up against col X' style
	 * @todo make depend on PlotData
	 */
	//!@todo setUsingSeries(const svector_t &)
	class PlotStyle {
	public:
		//! What format the plot should be in?
		struct Term {
			//! Term type
			enum Type {
				png  = 100, //!< png. Portable Network Graphics file.
				pdf  = 101, //!< pdf. Portable Document Format actually, this is eps converted to pdf.
				eps  = 102, //!< eps. Encapsulated PostScript. Cool for inclusion in LaTex files. This is the default.
				epstex  = 107, //!< epslatex. Encapsulated PostScript. Cool for inclusion in LaTex files with latex formulas.
				svg  = 103, //!< sgv. Scalable Vector Graphics.
				tex  = 104, //!< tex. Simple tabular.
				html = 105, //!< html. HTML table.
				other= 106 //!< don't know yet...
			} ;
		};

		// enum NoType { other = 0 } ;
		//! What style of graphic : histogram ? graph ?
		struct Plot {
			//! Plot type
			enum Type {
				histo = 200, //! histogram plot. This is the default. x ticks are evenly spaced, whatever there value and are labelled with their value.
				graph = 201, //! standard plot. Plots y_i=f(x) with x in the first colum and y_i in ith column. x-ticks are well spaced. This will not work if the X are not numbers (but strings).
				other = 202  //! other (ie user supplied).
			} ;
		};

		struct Line {
			enum Type {
				lines      = 300,  //! lines.
				histogram  = 301,  //! histogram (boxes).
				linespoints= 302,  //! lines with points. (default)
				points     = 303,  //! only the points.
				other      = 304   //! rien.
			} ;
		};

		struct Options {
			enum Type {
				oblique = 400,
				other   = 401
			};
		};

		/*! @brief Constructor.
		 * By default, creates an histogram representing the data in an eps plot.
		 */
		PlotStyle() ;

		/*! @brief sets the titles in the graph.
		 * @param titre Title of the graph
		 * @param titre_y Title of the y-axis (series)
		 * @param titre_x Title of the x-axis (data points)
		 */
		void setTitle ( const std::string  &  titre
				, const std::string  & titre_y
				, const std::string  & titre_x);

		/*! @brief Gets the title of the graph.
		 * @return a gnuplot command to set the title of the graph.
		 */
		std::string  getTitle() const ;

		/*! @brief Gets the title of points abscissa.
		 * @return a gnuplot command to set the title of the abscissa.
		 */
		std::string getTitleX() const ;

		/*! @brief Gets the title of the series.
		 * @return a gnuplot command to set the title of the ordinate (series).
		 */
		std::string getTitleY() const ;

		/*! @brief get the title string.
		 * @param index can be (0,1,2)
		 */
		std::string getRawTitle(int index=0) const ;

		/*! @brief Sets the output format.
		 * @sa TermType
		 * @param term type
		 */
		void setTerm( enum Term::Type term) ;


		/*! @brief Gets the output format.
		 * @return string for setting the expected output format in gnuplot.
		 * @warning noenhanced allows underscores while enhanced does subscripts.
		 * if we add a (no) enhanced option, we'll have to add a safeFormat(std::string) that replaces \c _ with <code>\_</code> .
		 * This is tricky and can be done at "post production" stage :-)
		 */
		std::string getTerm() const ;

		/*! @brief Gets the graph output extension.
		 * By default, this is ".eps".
		 * @return a string for this extension, including the sepatating dot.
		 *
		 */
		std::string getExt() const ;

#if 0
		/*! @brief gets the style of the graph.
		 * This is very user-tweakable !!
		 * @return the style for gnuplot.
		 */
		const std::string & getStyle()
		{
			return "#style\n"+_styleopts_ ;
		}

		/*! @brief sets the style of the graph.
		 * This is very user-tweakable !!
		 * @param style the style for gnuplot as a gnuplot command.
		 */
		void setStyle(const std::string & style)
		{
			_styleopts_ = style ;
		}
#endif

		/*! @brief sets the legend position.
		 * @param keypos the arguments to key (where the legend should be put)
		 * can be :
		 * <code>
		 *      set key {on|off} {default}
		 *              {{inside | outside} | {lmargin | rmargin | tmargin | bmargin} | {at <position>}}
		 *              {left | right | center} {top | bottom | center}
		 *              {vertical | horizontal} {Left | Right}
		 *              {{no}reverse} {{no}invert}
		 *              {samplen <sample_length>} {spacing <vertical_spacing>}
		 *              {width <width_increment>}
		 *              {height <height_increment>}
		 *              {{no}autotitle {columnheader}}
		 *              {title "<text>"} {{no}enhanced}
		 *              {{no}box { {linestyle | ls <line_style>} | {linetype | lt <line_type>} {linewidth | lw <line_width>}}}
		 * </code>
		 */
		void setKeyPos(const std::string & keypos) ;


		/*! @brief Gets the legend position.
		 * by default, it is "under".
		 */
		std::string getKeyPos() const ;

		/*! @brief sets the  position of the labels on the X absciss.
		 * @param opt
		 * @param more more stuff
		 */
		void setXtics ( enum Options::Type opt, const std::string & more="") ;

		/*! @brief Gets the legend position.
		 * by default, it is 45° inclined (use in on long tics legends).
		 */
		const std::string & getXtics() const ;

		/*! @brief Gets the name of the output graph.
		 * @param basnam the raw name for the output.
		 * @return basnam+extenstion.
		 */
		std::string getOutput(const std::string & basnam) const ;

		/*! @brief Sets the type of plot.
		 * @param type the type.
		 * @sa PlotType
		 *
		 */
		void setPlotType(enum Plot::Type type) ;

		/*! @brief Sets the way dots are linked.
		 * @sa LineType
		 * @param type type
		 */
		void setLineType( enum Line::Type type) ;

		/*! @brief Gets the type of plot.
		 * default is histogram, or if graph is supplied, then the default is linespoints.
		 * Can be totally customized.
		 * @return a string for gnuplot to set the plot type.
		 * @sa PlotType
		 *
		 */
		std::string getPlotType(const std::string & extraargs ="") ;

		/*! @brief adds some style line to the graph.
		 * This is very user-tweakable !!
		 * @param style a style line for gnuplot as a gnuplot command.
		 */
		void addPlotType(const std::string & style) ;

		/*! @brief tells which columns to use.
		 * @param col a column to use.
		 * @param moreargs more stuff
		 */
		void setUsingSeries(size_t col, const std::string &  moreargs= "") ;

		/*! @brief adds a column to use
		 * @param col a  column to use.
		 * @param moreargs more stuff
		 * @pre \p _usingcols_ is not empty, ie \c setUsingSeries has already been called.
		 */
		void addUsingSeries(size_t col, const std::string &  moreargs= "") ;

		/*! @brief tells which columns to use.
		 * @param cols a list of column to use.
		 * @param moreargs more stuff
		 */
		void setUsingSeries(std::list<size_t> cols, const std::string & moreargs= "") ;

		/*! @brief adds a set of columns to use.
		 * @param cols a list of column to use.
		 * @param moreargs more stuff
		 * @pre \p _usingcols_ is not empty, ie \c setUsingSeries has already been called.
		 */
		void addUsingSeries(std::list<size_t> cols, const std::string & moreargs= "") ;

		/*! @brief tells which columns to use.
		 * @param cols all colums between \c cols.first and \c cols.second (included)
		 * will be used.
		 * @param moreargs more stuff
		 *
		 */
		void setUsingSeries(std::pair<size_t,size_t> cols, const std::string & moreargs= "") ;

		/*! @brief adds contiguous columns to use.
		 * @param cols all colums between \c cols.first and \c
		 * cols.second will be used.
		 * @param moreargs more stuff
		 * @pre \p _usingcols_ is not empty, ie \c setUsingSeries has
		 * already been called.
		 *
		 */
		void addUsingSeries(std::pair<size_t,size_t> cols, const std::string & moreargs= "") ;

		const std::string & getUsingSeries() const ;

	private :
		// int                                 _precision_ ;   //!< precision of the output. by default 2.
		/* Legend. */
		std::string                         _legend_pos_;   //!< legend position
		/*  titles */
		std::string                         _title_     ;   //!< name of the graph
		std::string                         _title_x_   ;   //!< title for the points
		std::string                         _title_y_   ;   //!< title for the series
		std::string                         _xtics_     ;   //!< format for the x tics.
		/*  units */
		// std::string                      _unit_      ;
		/*  terminal output */
		enum Term::Type                     _term_      ; //!< output data format.
		// std::string                      _termopts_  ;
		/*  plotting style */
		enum Plot::Type                     _plot_type_ ; //!< histogram/graph style
		// std::string                         _plot_extra_; //!< extra specification for the plot style. default empty.
		enum Line::Type                     _line_type_ ; //!< style for the representation of points
		std::string                         _styleopts_ ; //!< gp style command.
		/*  columns to use */
		std::string                         _usingcols_ ; //!< columns to be used (gp command)


	} ; // PlotStyle

} // LinBox

//
// PlotData
//

namespace LinBox {

		struct Point {
			//! x
			struct Labels {
				typedef std::string type;
			};

			//! y
			struct Values {
				typedef double type;
			} ;

			//! numerical value for x
			struct Points {
				typedef double type;
			};

			//! y time
			struct Times  {
				typedef double type;
			};
		} ;

	/*! @brief The raw data to plot.
	 * Represents  the labels for the points (X axis) and the values for
	 * each series of measures (Y axis).
	 *
	 * Members that set/get are named as follows :
	 * - getX(nom,j) return the j'th element of series named nom
	 * - getX(i,j) return the j'th element of series number i
	 * - getCurrentSeriesX(j) return the j'th element of current series
	 * - getCurrentEntryX() return the current element of current series (ie the last one)
	 * - .
	 *
	 * Members are also named as follows :
	 * - getXXX is a const member
	 * - selectXXX is non const and may create/update stuff
	 * - refXXX returns a reference
	 * - setXXX sets something
	 *
	 * @internal The internal representation is a
	 * vector of vector, each series of point being a vector of double.
	 *
	 * @tparam Xkind the X axis is parametrised by \p Xkind (string, int, double...)
	 * The Y axis is always represented by double.
	 * @todo put the legend (title, x, y) in there
	 */
	class PlotData {
	private :
		std::vector<DataSeries >      _tableau_  ;   //!< data. \c _tableau_[i] represents a series of measurements. A data series is augmented only via the \c push_back method. A series may be accessed by its name, its number or it is the current working series.
		svector_t                  _series_label_ ;   //!< label for each series of measures. Used in the legend of the plots/tables of points.
		mutable size_t            _curr_series_  ;   //!< index of the current series of measurements.
		mutable TimeWatcher        _time_watch_  ;   //!< time predictor, helper. See \c TimeWatcher.
		MetaData                     _plot_data_ ;   //!< information abouth the benchmark
		MetaDataSeries               _meta_data_ ;   //!< information about each point
	private:

#ifdef __LINBOX_HAVE_TINYXML2
		//! @internal data part of the XML output
		tinyxml2::XMLElement * saveData(tinyxml2::XMLDocument & doc) ;
#endif

		/** Finds the index of a series by its name.
		 * @param nom name of the series
		 * @return its index
		 */
		size_t getIndex(const std::string & nom) const ;

		/** Finds the index of a series by its name.
		 * @param nom name of the series
		 * @return its index
		 */
		size_t selectIndex(const std::string & nom) ;

	public :

		/*! Returns the ith series of measurements.
		 * @param i ith series to be returned
		 */
		const DataSeries & getSeries(const size_t  &i) const ;

		/*! Returns the ith series of measurements.
		 * @param i ith series to be returned
		 */
		const DataSeries & selectSeries(const size_t  &i) ;

		/*! Returns the series of measurements after its name.
		 * @param nom name of series to be returned
		 */
		const DataSeries & selectSeries(const std::string &name) ;

		/*! Returns the current series of measurements.
		*/
		const DataSeries & getCurrentSeries() const;

		/*! Returns the ith series of measurements.
		 * @param i ith series to be returned
		 */
		DataSeries & refSeries(const size_t  &i) ;

		/*! Returns the ith series of measurements.
		 * @param i ith series to be returned
		 */
		DataSeries & refSeries(const std::string  & nom) ;

		/*! Returns the current series of measurements.
		*/
		DataSeries & refCurrentSeries() ;

		/*! Inits a plot with series of data.
		 * @param nb_pts number of points in each series.
		 * @param nb_srs number of series of points. Default is 1.
		 */
		PlotData() ;
		/*! destructor.
		*/
		~PlotData()  ;

		/*! copy constructor.
		 * @param PD a PlotData to copy.
		 */
		PlotData(const PlotData & PD);

		/** @brief initialize to empty
		*/
		void clear() ;

		/*! merges another plot data to the current one.
		 * (just adds to the end, does not merge series by name yet)
		 */
		void merge(const PlotData &PD) ;

		/*! @brief get the number of series.
		 * @return number of series.
		 */
		size_t size() const ;

		/** @brief gets the current series number.
		*/
		size_t getCurrentSeriesNumber() const ;

		/*! @brief Sets the name of a series.
		 * @param i index of the series
		 * @param nom name of the series
		 */
		void setSeriesName(const size_t & i, const std::string & nom) ;

		// ref  Series

		template<class Pt>
		std::vector<typename Pt::type> & refSeries(const size_t & i,  Pt kind) ;

		template<class Pt>
		std::vector<typename Pt::type> & refSeries(const std::string & name,  Pt kind)
		{
			return refSeries(getIndex(name ), kind );
		}

		// get Series

		template<class Pt>
		const std::vector<typename Pt::type> & getSeries(const size_t & i,  Pt kind) const ;

		template<class Pt>
		const std::vector<typename Pt::type> & getSeries(const std::string & name,  Pt kind) const
		{
			return getSeries(getIndex(name),kind);
		}

		template<class Pt>
		const std::vector<typename Pt::type> & getCurrentSeries( Pt kind) const
		{
			return getSeries(_curr_series_, kind);
		}

		template<class Pt>
		const typename Pt::type & getSeriesEntry(const size_t & i, const size_t & j, Pt kind) const
		{
			linbox_check(j<getSeriesSize(i));
			return (getSeries(i,kind)[j]) ;
		}

		template<class Pt>
		const typename Pt::type & getSeriesEntry(const std::string & name, const size_t & j, Pt kind) const
		{
			return getSeriesEntry(getIndex(name),j,kind);
		}

		template<class Pt>
		const typename Pt::type & getSeriesEntry(const size_t & i,  Pt kind) const
		{
			return ( getSeries(i,kind).back() ) ;
		}

		template<class Pt>
		const typename Pt::type & getSeriesEntry(const std::string & name,  Pt kind) const
		{
			return getSeriesEntry(getIndex(name),kind);
		}

		template<class Pt>
		const typename Pt::type & getCurrentSeriesEntry(const size_t & j, Pt kind) const
		{
			return getSeriesEntry(_curr_series_, j, kind);
		}

		template<class Pt>
		const typename Pt::type & getCurrentSeriesEntry( Pt kind) const
		{
			return getSeriesEntry(_curr_series_, kind);
		}


		// set Series

		/** Gets the name of a series.
		 * @param i index of the series
		 */
		const std::string &  getSeriesName(const size_t & i) const ;

		/** Gets the name of the current series.
		*/
		const std::string &  getCurrentSeriesName() const ;

		/*! @brief Sets the name of the current series.
		 * @param nom name of the series
		 */
		void setCurrentSeriesName(const std::string & nom) ;

		/** Inits the watch on a series
		 * @param i index of a series
		 */
		void initWatch ( const size_t & i) ;

		/** Inits the watch to current series
		*/
		void initCurrentSeriesWatch () ;

		/** Creates a new series.
		 * It is created after the last series.
		 * \c getCurrentSeries() points to it.
		 * @param nom name of the new series
		 */
		void newSeries(const std::string & nom = "") ;

		/** Finish a series of measurements.
		 * Nothing is done for the moment.
		 */
		void finishSeries() ;

		/** @brief size of a series.
		 * @param i index of the series
		 */
		size_t getSeriesSize(const size_t & i) const ;

		/** @brief size of the current series.
		*/
		size_t getCurrentSeriesSize() const ;

		/*! goes to the next series of points
		*/
		bool selectNextSeries() ;

		/** selects the first series
		 */
		void selectFirstSeries() ;

		/*! @brief Sets the name of a point.
		 * @param i series number
		 * @param j index for the the point
		 * @param nom name of the point
		 */
		template<class T>
		void setSeriesPointLabel(const size_t & i, const size_t & j, const T & nom)
		{
			std::string nom_s = fortifyString(toString(nom));
			linbox_check(j<getSeriesSize(i) );
			refSeries(i).PointLabels[j] = nom_s ;

			return;
		}

		/*! @brief Sets the name of a point.
		 * @param j index for the the point
		 * @param nom name of the point
		 */
		void setCurrentSeriesPointLabel(const size_t & j, const std::string & nom) ;



		/*! @brief gets the name of a series.
		 * Defaults to \c "series.i"
		 * @param i its index.
		 * @return its name.
		 */
		std::string getSeriesLabel(const size_t & i) const ;

		/*! @brief gets the name of a series.
		 * Defaults to \c "series.i"
		 * @param i its index.
		 * @return its name.
		 */
		std::string  getCurrentSeriesLabel() const;

		/*! @brief gets all the names in the series.
		 * @return a vector of names.
		 */
		const svector_t  & getSeriesLabels() const ;





		/*! @brief sets a new entry.
		 * @param i index of the series
		 * @param j index of the point
		 * @param nam name of the point (eg size of the matrix, name of a sparse matrix,...)
		 * @param val value to be inserted (eg mflops, sec,...).
		 * @param xval x value of the point (eg size of the matrix, of a sparse matrix,...)
		 * @param yval time for this computation (seconds)
		 */
		void setSeriesEntry(const size_t &i, const std::string & nam, const double & val
				    , const double & xval = NAN, const double & yval = NAN) ;

		/*! @brief sets a new entry.
		 * @param name name of the series
		 * @param j index of the point
		 * @param nam name of the point (eg size of the matrix, name of a sparse matrix,...)
		 * @param val value to be inserted (eg mflops, sec,...).
		 * @param xval x value of the point (eg size of the matrix, of a sparse matrix,...)
		 * @param yval time for this computation (seconds)
		 */
		void setSeriesEntry(const std::string & nom, const std::string & nam, const double & val
			      , const double & xval = NAN, const double & yval = NAN) ;

		/*! @brief sets a new entry.
		 * @param j index of the point
		 * @param nam name of the point (eg size of the matrix, name of a sparse matrix,...)
		 * @param val value to be inserted (eg mflops, sec,...).
		 */
		template<class T>
		void setCurrentSeriesEntry(const T & nam, const double & val
					   , const double & xval = NAN, const double & yval = NAN)
		{
			std::string nam_s = fortifyString(toString(nam));
			return setSeriesEntry(_curr_series_,nam_s,val,xval,yval) ;
		}



		/*! gets a reference to the array of data.
		 * @return a reference to the member \c _tableau_ representing the data.
		 */
		const std::vector<DataSeries > & getTable() const ;

		/*! gets a reference to the array of data.
		 * @return a reference to the member \c _tableau_ representing the data.
		 */
		std::vector<DataSeries > & refTable() ;


		/** @brief Continue for another time measure ?
		 * @see TimeWatcher::keepon
		 * @param repet  current number of repetitions for this new measure
		 * @param tim    time previously spent on the measures.
		 * @return true if one more measure can be done
		 */
		bool keepon(size_t & repet, double tim, bool usePrediction=false) ;

		void load( const std::string & filename) ;

		/** @brief saves the data in XML format.
		 * @param filename file name
		 * @param title titles of the data
		 * @param xtitle legend of the X axis
		 * @param ytitle legend of the Y axis.
		 */
		void save( const std::string & filename
			   , const std::string & title = ""
			   , const std::string & xtitle = ""
			   , const std::string & ytitle = "") ;

		void addCurrentEntryMetaData(const MetaData & m)
		{
			_meta_data_.push_back(getCurrentEntryId(),m);
		}

		/** returns the unique ID of the current series last entry
		 */
		const std::string & getCurrentEntryId() const
		{
			return (getCurrentSeries().UID).back();
		}

		/** @brief returns the unique ID of the current series j'th entry.
		 * @param j index of the entry.
		 */
		const std::string & getCurrentSeriesId(const size_t & j) const
		{
			return   (getCurrentSeries().UID[j]);
		}

		/** @brief returns the unique ID of the i'th series j'th entry.
		 * @param i index of the series.
		 * @param j index of the entry.
		 */
		const std::string & getId(const size_t & i, const size_t & j)
		{
			return   (selectSeries(i).UID[j]);
		}

		/** @brief returns the unique ID of the i'th series j'th entry.
		 * @param i index of the series.
		 * @param j index of the entry.
		 */
		const std::string & getId(const std::string & name, const size_t & j)
		{
			return   (selectSeries(name).UID[j]);
		}

	}; // PlotData

} // LinBox

//
// PlotGraph
//

namespace LinBox {

	/*! @brief The graph (2D).
	 * This class joins a PlotStyle and a PlotData to build up a graph.  A
	 * filename should be provided as well, indicating where the output
	 * graph and scripts will be generated.
	 *
	 * @warning the filename will get a random suffix before the extension
	 * so as not to overwrite files "par inadvertance".
	 * @warning don't name anything else than a folder "data" in your working directory. You've been warned.
	 * @todo make depend on PlotStyle (that owns data)
	 */
	//!@todo use getUsingSeries in latex/html/csv/xml
	class PlotGraph {
	private :
		PlotData              & _data_ ;   //!< reference to the data points
		PlotStyle            & _style_ ;   //!< reference to a plotting style
		std::string         _filename_ ;   //!< name for the output file (without extension). a random \c _XXXXXX suffix will be added to make it unique.
		std::string        _printname_ ;   //!< name for the output file (without extension) to be printed. a random \c _XXXXXX suffix makes it unique.
		dmatrix_t         _merge_data_ ;
		svector_t       _merge_points_ ;

	private :

		/*! @internal
		 * @brief Appends random suffix.
		 * Appends to \p _filename_ a random string constituted of an
		 * underscore followed by 8 random alnum chars.
		 * @return the concatenation of \c _filename_ and this suffix.
		 */
		void _randomName();

		//! @internal returns the file name (without extension and uid)
		const std::string & getFileName()  ;

		//! @bug this supposes the two series have unique measurements for one point.
		void mergeTwoSeries( svector_t & merge_points
				     , dmatrix_t & merge_data
				     , const svector_t & pts
				     , const dvector_t & dat
				     , const size_t & idx) const ;

		//! merge all series of points into a vector of absissa points  and a vector of vector of data points
		void mergeSeries();

		void print_csv();

		void print_dat();

		void print_xml();

		void print_html() ;

		/*! @brief Prints data in a latex tabular.
		*/
		void print_latex();

		/*!@brief Plots the data with gnuplot.
		 * Produces data in a .dat file, creates a .gp gnuplot script and
		 * outputs a graph calling gnuplot.
		 * @warning If gnuplot is not available, fall back to the latex method.
		 */
		void print_gnuplot(bool only_data=false);

	public :

		/*! @brief Sets a new data structure.
		 * @param data a reference to a PlotData class.
		 */
		void setData( PlotData & data );

		/*! @brief Gets the data.
		 * @param[in,out] data a reference to a PlotData class.
		 */
		PlotData & refData( PlotData & data);

		/*! @brief Sets a new style structure.
		 * @param style a reference to a PlotStyle class.
		 */
		void setStyle( PlotStyle & style ) ;

		/*! @brief Gets the style.
		 * @param[in,out] style a reference to a PlotStyle class.
		 */
		PlotStyle & refStyle( PlotStyle & style) ;

		PlotStyle & refStyle( )
		{ return _style_ ; }


		// not implemented yet
		void sortSeries()  ;

		// not implemented yet
		void unique() ;

		/*! @brief Constructor for the PlotGraph class.
		 * Plots a series of data according to a style.
		 * @param data data to be plot, will be processed by the style
		 * @param style sets parameters to gnuplot to achieve a nice
		 * plot.
		 */
		PlotGraph( PlotData & data, PlotStyle & style );

		/*! @brief sets the ouput file name.
		 * All output is put in a "data" subfolder.
		 * @warning Since no file is overwritten, this
		 * directory can rapidly get very populated.
		 */
		void setOutFilename( const std::string & filename ) ;

		const std::string & getUsingSeries() ;

		/*! @brief Gets the plot command line.
		 * @param File the name of/path to the data file (with extension)
		 * @return a gnuplot "plot" command stream.
		 */
		std::string getPlotCommand(const std::string & File) ;

		void print( Tag::Printer pt = Tag::Printer::xml) ;

		void save() ;

		void load(const std::string & filename) ;

	}; // PlotGraph

} // LinBox

//
// Least Squares
//

#ifdef __LINBOX_HAVE_LAPACK
extern "C" {
#if 1 // from lapack (not clapack)

	void dgels_(char *trans, int *m, int *n, int *nrhs, double *a, int *lda,
		    double *b, int *ldb, double *work, int *lwork, int *info);

	void dgelsy_(int *m, int *n, int *nrhs, double *a, int *lda,
		     double *b, int *ldb, int *JPVT, double *RCOND, int *RANK,
		     double *work, int *lwork, int *info);

	void dgelss_(int *m, int *n, int *nrhs, double *a, int *lda,
		     double *b, int *ldb, double *s, double *RCOND, int *RANK,
		     double *work, int *lwork, int *info);
#endif

#if 0
	int clapack_dgels (const enum CBLAS_ORDER 	Order,
			   const enum CBLAS_TRANSPOSE 	TA,
			   int M,
			   int N,
			   int NRHS,
			   double * 	A,
			   int lda,
			   double * 	B,
			   const int 	ldb
			  );
#endif
}
#endif // __LINBOX_HAVE_LAPACK

//
// Curve fitting
//

namespace LinBox {

	//! fit X[n-1,n],Y[n-1,n] and return evaluation at x.
	double fit2(const dvector_t & X, const dvector_t & Y, int n, double x);

#ifdef __LINBOX_HAVE_LAPACK
	//! fits with a degree 3 polynomial and return evaluation at x.
	double fit_lapack3(const dvector_t &X, const dvector_t &Z, double x);
#endif // __LINBOX_HAVE_LAPACK


	//! fit X[n-2,n],Y[n-2,n] and return evaluation at x.
	double fit3(const dvector_t & X, const dvector_t & Y,int n, double x);

} // LinBox


#ifdef LinBoxSrcOnly
#include "benchmarks/benchmark.C"
#endif

#include "benchmarks/benchmark.inl"

#endif // __LINBOX_benchmarks_benchmark_H

// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
