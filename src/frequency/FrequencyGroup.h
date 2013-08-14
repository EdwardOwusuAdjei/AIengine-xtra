/*
 * AIEngine a deep packet inspector reverse engineering engine.
 *
 * Copyright (C) 2013  Luis Campo Giralte
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
 * Written by Luis Campo Giralte <luis.camp0.2009@gmail.com> 2013
 *
 */
#ifndef _FrequencyGroup_H_
#define _FrequencyGroup_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <utility>
#include <cstring>
#include "Frequencies.h"
#include "../flow/FlowManager.h"

using namespace std;

template <class A_Type> class FrequencyGroup 
{
public:
    	explicit FrequencyGroup(): name_(""),total_process_flows_(0),total_computed_freqs_(0) {};
    	virtual ~FrequencyGroup() {};

	const char* getName(){ return name_.c_str();} 
	void setName(char *name) { name_ = name;}

	void agregateFlows(FlowManagerPtr flow_t, std::function <A_Type (FlowPtr&)> condition);
	void compute();

	friend ostream& operator<<(ostream& os, const FrequencyGroup& fg)
	{
		os << "Frequency Group(" << fg.name_ <<") total frequencies groups:" << fg.group_map_.size() << std::endl;
		os << "\tTotal process flows:" << fg.total_process_flows_<< std::endl;
		os << "\tTotal computed frequencies:" << fg.total_computed_freqs_<< std::endl;
		for (auto it = fg.group_map_.begin(); it!=fg.group_map_.end();++it)
		{
			Frequencies *freq_ptr = std::get<0>(it->second).get();
			int items = std::get<1>(it->second);	
			
			os << "\tGroup by:" << it->first <<  " items:" << items << " dispersion:" << freq_ptr->getDispersion() <<std::endl;
			os << "\t" << freq_ptr->getFrequenciesString() << std::endl;
		}
		os << std::endl; 
	}	

	void agregateFlowsBySourcePort(FlowManagerPtr flow_t);
	void agregateFlowsByDestinationPort(FlowManagerPtr flow_t);
	void agregateFlowsBySourceAddress(FlowManagerPtr flow_t); 
	void agregateFlowsByDestinationAddress(FlowManagerPtr flow_t); 

	int32_t getTotalProcessFlows() { return total_process_flows_;}
	int32_t getTotalComputedFrequencies() { return total_computed_freqs_;}

private:
	std::string name_;
	int32_t total_process_flows_;
	int32_t total_computed_freqs_;
	std::map <A_Type,std::pair<FrequenciesPtr,int>> group_map_;
};

#include "FrequencyGroup_Impl.h"
#endif
