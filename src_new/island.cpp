/*****************************************************************************
 *   Copyright (C) 2004-2009 The PaGMO development team,                     *
 *   Advanced Concepts Team (ACT), European Space Agency (ESA)               *
 *   http://apps.sourceforge.net/mediawiki/pagmo                             *
 *   http://apps.sourceforge.net/mediawiki/pagmo/index.php?title=Developers  *
 *   http://apps.sourceforge.net/mediawiki/pagmo/index.php?title=Credits     *
 *   act@esa.int                                                             *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the                           *
 *   Free Software Foundation, Inc.,                                         *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.               *
 *****************************************************************************/

// 04/01/2009: Initial version by Francesco Biscani.

#include <boost/numeric/conversion/cast.hpp>
#include <sstream>
#include <string>

#include "algorithm/base.h"
#include "problem/base.h"
#include "exceptions.h"
#include "island.h"
#include "population.h"
#include "types.h"

namespace pagmo
{
/// Constructor from problem::base, algorithm::base and number of individuals.
/**
 * Will store a copy of the problem and of the algorithm internally, will initialise internal population to n individuals
 * and evolution time to zero. Will fail if n is negative. Island will be associated to no archipelago.
 */
island::island(const problem::base &p, const algorithm::base &a, int n):m_pop(p,n),m_algo(a.clone()),m_archi(0),m_evo_time(0) {}

/// Copy constructor.
/**
 * Will perform a deep copy of all the elements of island isl, which will be synchronised before any operation takes place.
 */
island::island(const island &isl)
{
	// Do it like this so that we can synchronise isl before poking into its internals.
	operator=(isl);
}

/// Assignment operator.
/**
 * Performs a deep copy of all the elements of isl into this island. Both island will be synchronised before assignment.
 */
island &island::operator=(const island &isl)
{
	if (this != &isl) {
		// Make sure both islands are in a known state.
		join();
		isl.join();
		// Copy over content.
		m_pop = isl.m_pop;
		m_algo = isl.m_algo->clone();
		m_archi = isl.m_archi;
		m_evo_time = isl.m_evo_time;
	}
	return *this;
}

/// Destructor.
/**
 * Will call island::join() before returning.
 */
island::~island()
{
	join();
}

/// Explicitly synchronise the island.
/**
 * After this method returns, any pending evolution has been completed.
 */
void island::join() const
{
	lock_type lock(m_evo_mutex);
}

/// Return terse human readable representation of the island.
/**
 * Will return a formatted string containing the human readable forms of the problem and of the algorithm.
 */
std::string island::human_readable_terse() const
{
	join();
	std::ostringstream oss;
	oss << m_pop.problem() << '\n';
	oss << *m_algo << '\n';
	return oss.str();
}

/// Return complete human readable representation of the island.
/**
 * Will return a formatted string containing the output of human_readable_terse(),
 * whether or not the island belongs to an archipelago, the total evolution time and the list of individuals.
 */
std::string island::human_readable() const
{
	join();
	std::ostringstream oss;
	oss << human_readable_terse();
	oss << "Belongs to archipelago: " << (m_archi ? "true" : "false") << '\n' << '\n';
	if (m_pop.size()) {
		oss << "List of individuals:\n";
		for (population::size_type i = 0; i < m_pop.size(); ++i) {
			oss << '#' << i << ":\n";
			oss << "\tDecision vector:\t" << m_pop.get_individual(i).get<0>() << '\n';
			oss << "\tVelocity vector:\t" << m_pop.get_individual(i).get<1>() << '\n';
			oss << "\tFitness vector:\t\t" << m_pop.get_individual(i).get<2>() << '\n';
			oss << "\tBest fitness vector:\t" << m_pop.get_individual(i).get<3>() << '\n';
		}
	} else {
		oss << "No individuals.\n";
	}
	if (m_pop.champion().get<0>().size()) {
		oss << "Champion:\n";
		oss << "\tDecision vector:\t" << m_pop.champion().get<0>() << '\n';
		oss << "\tFitness vector:\t\t" << m_pop.champion().get<1>() << '\n';
	} else {
		oss << "No champion yet.\n";
	}
	return oss.str();
}

/// Overload stream operator for pagmo::island.
/**
 * Equivalent to printing island::human_readable() to stream.
 */
std::ostream &operator<<(std::ostream &s, const island &isl)
{
	s << isl.human_readable();
	return s;
}

}
