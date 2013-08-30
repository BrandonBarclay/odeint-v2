/*
 [auto_generated]
 boost/numeric/odeint/external/mpi/mpi_vector_state.hpp

 [begin_description]
 Copying a container from/to an mpi_state splits/joins it.
 [end_description]

 Copyright 2009-2011 Karsten Ahnert
 Copyright 2009-2011 Mario Mulansky

 Distributed under the Boost Software License, Version 1.0.
 (See accompanying file LICENSE_1_0.txt or
 copy at http://www.boost.org/LICENSE_1_0.txt)
 */


#ifndef BOOST_NUMERIC_ODEINT_EXTERNAL_MPI_MPI_VECTOR_STATE_HPP_INCLUDED
#define BOOST_NUMERIC_ODEINT_EXTERNAL_MPI_MPI_VECTOR_STATE_HPP_INCLUDED

#include <vector>
#include <algorithm>
#include <boost/mpi.hpp>
#include <boost/numeric/odeint/util/copy.hpp>
#include <boost/numeric/odeint/util/split_adaptor.hpp>
#include <boost/numeric/odeint/algebra/algebra_dispatcher.hpp>
#include "mpi_state.hpp"

namespace boost {
namespace numeric {
namespace odeint {


/** \brief Copy data from some container on node 0 to the slaves.
 * Source must be a model of Random Access Range. */
template< class Source , class InnerState >
struct copy_impl< Source, mpi_state< InnerState >,
    typename boost::enable_if< boost::has_range_const_iterator<Source> >::type >
{
    typedef typename boost::range_iterator<const Source>::type iterator;

    static void copy( const Source &from, mpi_state< InnerState > &to )
    {
        std::vector< InnerState > pieces;
        if(to.world.rank() == 0) {
            const size_t num = static_cast<size_t>(to.world.size());
            pieces.resize(num);
            for(size_t i = 0 ; i < num ; i++) {
                iterator_range<iterator> part = detail::make_split_range(from, i, num);
                boost::numeric::odeint::resize(pieces[i], part);
                boost::numeric::odeint::copy(part, pieces[i]);
            }
        }
        // send to nodes
        boost::mpi::scatter(to.world, pieces, to.data, 0);
    }
};

/** \brief Copy data from an mpi_state to some container on node 0.
 * Target must be a model Single Pass Range. */
template< class Target, class InnerState >
struct copy_impl< mpi_state< InnerState >, Target,
    typename boost::enable_if< boost::has_range_iterator<Target> >::type >
{
    typedef typename boost::range_iterator<Target>::type iterator;

    static void copy( const mpi_state< InnerState > &from , Target &to )
    {
        std::vector< InnerState > pieces;
        // send data to root
        boost::mpi::gather(from.world, from.data, pieces, 0);
        if(from.world.rank() == 0) {
            // check target size
            size_t total_size = 0;
            for(size_t i = 0 ; i < pieces.size() ; i++)
                total_size += boost::size(pieces[i]);
            BOOST_ASSERT( total_size <= boost::size(to) );
            // copy parts
            iterator out = boost::begin(to);
            for(size_t i = 0 ; i < pieces.size() ; i++)
                out = boost::copy(pieces[i], out);
        }
    }
};


}
}
}


#endif

