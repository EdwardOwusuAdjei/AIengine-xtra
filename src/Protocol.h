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
#ifndef _Protocol_H_
#define _Protocol_H_

#include <iostream>
#include <fstream>
#include "FlowForwarder.h"
#include "Multiplexer.h"

class Flow;

class Protocol 
{
public:
    	Protocol():total_malformed_packets_(0),total_validated_packets_(0),
		total_packets_(0),name_(""){};
    	virtual ~Protocol() {};

	virtual void setHeader(unsigned char *raw_packet) = 0;
	virtual void statistics(std::basic_ostream<char>& out) = 0;
	virtual void statistics() = 0;
	virtual const char* getName() = 0;

	virtual void processFlow(Flow *flow) = 0;
	virtual void processPacket(Packet &packet) = 0;

	void virtual setMultiplexer(MultiplexerPtrWeak mux) = 0;
	MultiplexerPtrWeak virtual getMultiplexer() = 0; 

	void virtual setFlowForwarder(FlowForwarderPtrWeak ff) = 0;
	FlowForwarderPtrWeak virtual getFlowForwarder() = 0; 
	
	mutable std::string name_;
	mutable int64_t total_malformed_packets_;
	mutable int64_t total_validated_packets_;
	mutable int64_t total_packets_;
private:
	u_int16_t protocol_id_;
};

typedef std::shared_ptr <Protocol> ProtocolPtr;

#endif
