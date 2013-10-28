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
#include "UDPProtocol.h"
#include <iomanip> // setw

namespace aiengine {

void UDPProtocol::statistics(std::basic_ostream<char>& out) {

	if (stats_level_ > 0) {
		out << "UDPProtocol(" << this << ") statistics" << std::dec << std::endl;
		out << "\t" << "Total packets:          " << std::setw(10) << total_packets_ <<std::endl;
		out << "\t" << "Total bytes:            " << std::setw(10) << total_bytes_ <<std::endl;
		if (stats_level_ > 1){ 
			out << "\t" << "Total validated packets:" << std::setw(10) << total_validated_packets_ <<std::endl;
			out << "\t" << "Total malformed packets:" << std::setw(10) << total_malformed_packets_ <<std::endl;
			if (stats_level_ > 2) {
				if(mux_.lock())
					mux_.lock()->statistics(out);
				if(flow_forwarder_.lock())
					flow_forwarder_.lock()->statistics(out);
				if (stats_level_ > 3){ 
					if(flow_table_)
						flow_table_->statistics(out);
					if(flow_cache_)
						flow_cache_->statistics(out);
				 }
			}
		}
	}
}

SharedPointer<Flow> UDPProtocol::getFlow() { 

	unsigned long h1;
	unsigned long h2;
	SharedPointer<Flow> flow;
	MultiplexerPtrWeak downmux = mux_.lock()->getDownMultiplexer();	
	MultiplexerPtr ipmux = downmux.lock();

	if (flow_table_) {
                h1 = ipmux->address.getHash(getSrcPort(),17,getDstPort());
                h2 = ipmux->address.getHash(getDstPort(),17,getSrcPort());

		flow = flow_table_->findFlow(h1,h2);
		if (!flow){
			if (flow_cache_){
				flow = flow_cache_->acquireFlow().lock();
				if (flow) {
					flow->setId(h1);
					flow->setFiveTuple(ipmux->address.getSourceAddress(),
						getSrcPort(),17,
						ipmux->address.getDestinationAddress(),
						getDstPort());
					flow_table_->addFlow(flow);			
				}
			}
		}
	}
	return flow; 
}

void UDPProtocol::processPacket(Packet& packet) {

	SharedPointer<Flow> flow = getFlow();
	int bytes;

	++total_packets_;

	if(flow) {
		bytes = (getLength() - getHeaderLength());

		total_bytes_ += bytes;
		flow->total_bytes += bytes;
		++flow->total_packets;

		if(flow_forwarder_.lock()&&(bytes>0)) {
			FlowForwarderPtr ff = flow_forwarder_.lock();

                        // Modify the packet for the next level
                        packet.setPayload(&packet.getPayload()[getHeaderLength()]);
                        packet.setPrevHeaderSize(getHeaderLength());
                        packet.setPayloadLength(packet.getLength() - getHeaderLength());

                        packet.setDestinationPort(getDstPort());
                        packet.setSourcePort(getSrcPort());

                        flow->packet = const_cast<Packet*>(&packet);
                        ff->forwardFlow(flow.get());
		}	
	}
}

} // namespace aiengine
