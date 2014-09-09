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
#ifndef SRC_PACKET_H_
#define SRC_PACKET_H_

#include <iostream>
#include "PacketAnomaly.h"

namespace aiengine {

class Packet 
{
public:
    	explicit Packet(unsigned char *packet,int length, int prev_header_size,
		PacketAnomaly pa, time_t packet_time):
		length_(length),packet_(packet),prev_header_size_(prev_header_size),
		source_port_(0),dest_port_(0),pa_(pa),packet_time_(packet_time) {}
	
	explicit Packet(unsigned char *packet, int length, int prev_header_size,
		PacketAnomaly pa): Packet(packet,length,prev_header_size,pa,0) {}

	explicit Packet(unsigned char *packet, int length, int prev_header_size):
		Packet(packet,length,prev_header_size,PacketAnomaly::NONE,0) {}

	explicit Packet(unsigned char *packet, int length):
		Packet(packet,length,0,PacketAnomaly::NONE,0) {}

    	explicit Packet():Packet(nullptr,0,0,PacketAnomaly::NONE,0) {}

    	virtual ~Packet() {}

	void setPacketTime(time_t packet_time) { packet_time_ = packet_time; }
	time_t getPacketTime() const { return packet_time_; }

	void setPayload(unsigned char *packet) { packet_ = packet; }
	void setPayloadLength(int length) { length_ = length;}
	void setPrevHeaderSize(int size) { prev_header_size_ = size;}

	void setDestinationPort(u_int16_t port) { dest_port_ = port;}
	void setSourcePort(u_int16_t port) { source_port_ = port;}

	void setPacketAnomaly(const PacketAnomaly &pa) { pa_ = pa; }
	PacketAnomaly getPacketAnomaly() const { return pa_;} 

	u_int16_t getDestinationPort() { return dest_port_;}
	u_int16_t getSourcePort() { return source_port_;}

	unsigned char *getPayload() { return packet_;}
	int getLength()  { return length_;}
	int getPrevHeaderSize()  { return prev_header_size_;}

	friend std::ostream& operator<<(std::ostream& os, const Packet& p) {
	
		os << "Begin packet(" << &p << ") length:" << p.length_ << " prev header size:" << p.prev_header_size_;
		os << " anomaly:" << PacketAnomalyToString.at(p.pa_) << " time:" << p.packet_time_ << std::endl;
		for (int i = 0;i< p.length_;++i) {
			os << std::hex << (int)p.packet_[i] << " ";
		}
		os << std::endl << "End packet" << std::endl; 
		return os;
	}	

private:
	int length_;
	unsigned char *packet_;
	int prev_header_size_;
	u_int16_t source_port_;
	u_int16_t dest_port_;
	PacketAnomaly pa_;
	time_t packet_time_;
};

typedef std::shared_ptr<Packet> PacketPtr;

} // namespace aiengine

#endif  // SRC_PACKET_H_
