/*
 * AIEngine a deep packet inspector reverse engineering engine.
 *
 * Copyright (C) 2013-2016  Luis Campo Giralte
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 * Written by Luis Campo Giralte <luis.camp0.2009@gmail.com> 
 *
 */
#include "IPBloomSet.h"
#include <iomanip> // setw

#ifdef HAVE_BLOOMFILTER

namespace aiengine {

void IPBloomSet::addIPAddress(const std::string &ip) {

	bloom_.insert(ip);
	++total_ips_;
}

bool IPBloomSet::lookupIPAddress(const std::string &ip) {

	if (bloom_.probably_contains(ip)) {
		++total_ips_on_set_;
		return true;
	} else {
		++total_ips_not_on_set_;
		return false;
	}
}

std::ostream& operator<< (std::ostream& out, const IPBloomSet& is) {

	out << "IPBloomSet " << is.getName() << std::endl;
	//out << "\tFalse positive rate:    " << std::setw(10) << is.getFalsePositiveRate() <<std::endl;
	out << "\tTotal IP address:       " << std::setw(10) << is.total_ips_ <<std::endl;
	out << "\tTotal lookups in:       " << std::setw(10) << is.total_ips_on_set_ <<std::endl;
	out << "\tTotal lookups out:      " << std::setw(10) << is.total_ips_not_on_set_ <<std::endl;
	return out;
}

} // namespace aiengine

#endif // HAVE_BLOOMFILTER
