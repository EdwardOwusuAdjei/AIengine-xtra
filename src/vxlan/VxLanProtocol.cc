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
#include "VxLanProtocol.h"
#include <iomanip>

namespace aiengine {

void VxLanProtocol::processFlow(Flow *flow) {

        int bytes = flow->packet->getLength();
        total_bytes_ += bytes;
        ++total_packets_;

        if (mux_.lock()&&(bytes > 0)) {
		// TODO: Check the VNI and forward the packet
                MultiplexerPtr mux = mux_.lock();

                Packet *packet = flow->packet;
                Packet gpacket;

                gpacket.setPayload(packet->getPayload());
                gpacket.setPrevHeaderSize(header_size);
                gpacket.setPayloadLength(packet->getLength());
                gpacket.setPacketTime(packet->getPacketTime());

                mux->setNextProtocolIdentifier(0);
                mux->forwardPacket(gpacket);
         }
}

void VxLanProtocol::statistics(std::basic_ostream<char>& out){ 

	if (stats_level_ > 0) {
		out << "VxLanProtocol(" << this << ") statistics" << std::endl;
		out << "\t" << "Total packets:          " << std::setw(10) << total_packets_ <<std::endl;
		out << "\t" << "Total bytes:        " << std::setw(14) << total_bytes_ <<std::endl;
		if (stats_level_> 1) {
			out << "\t" << "Total validated packets:" << std::setw(10) << total_validated_packets_ <<std::endl;
			out << "\t" << "Total malformed packets:" << std::setw(10) << total_malformed_packets_ <<std::endl;
			if (stats_level_ > 2) {
				if(mux_.lock())
					mux_.lock()->statistics(out);
                                if (flow_forwarder_.lock())
                                        flow_forwarder_.lock()->statistics(out);
			}
		}
	}
}

} // namespace aiengine
