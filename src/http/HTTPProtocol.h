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
#ifndef _HTTPProtocol_H_
#define _HTTPProtocol_H_

#ifdef __FAVOR_BSD
#undef __FAVOR_BSD
#endif // __FAVOR_BSD

#include "../Multiplexer.h"
#include "../FlowForwarder.h"
#include "../Protocol.h"
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <boost/regex.hpp>
#include "HTTPHost.h"
#include "HTTPUserAgent.h"
#include "HTTPReferer.h"
#include "../Cache.h"
#include <unordered_map>

class HTTPProtocol: public Protocol 
{
public:
    	explicit HTTPProtocol():http_header_(nullptr),total_bytes_(0),
		http_regex_("^(GET|POST|HEAD|PUT|TRACE).*HTTP/1."),
		http_host_("Host: .*?\r\n"),
		http_ua_("User-Agent: .*?\r\n"),
		http_referer_("Referer: .*?\r\n"),
		host_cache_(new Cache<HTTPHost>),
		ua_cache_(new Cache<HTTPUserAgent>)
	{ 
		name_="HTTPProtocol";
	}
    	virtual ~HTTPProtocol() {};
	
	static const u_int16_t id = 0;
	static const int header_size = 0;
	int getHeaderSize() const { return header_size;};

	int64_t getTotalBytes() const { return total_bytes_; };
	int64_t getTotalPackets() const { return total_packets_;};
	int64_t getTotalValidatedPackets() const { return total_validated_packets_;};
	int64_t getTotalMalformedPackets() const { return total_malformed_packets_;};

        const char *getName() { return name_.c_str();};

	void processPacket(Packet& packet){};
	void processFlow(Flow *flow);
	void statistics(std::basic_ostream<char>& out);
	void statistics() { statistics(std::cout);};

        void setMultiplexer(MultiplexerPtrWeak mux) { mux_ = mux; };
        MultiplexerPtrWeak getMultiplexer() { mux_;};

        void setFlowForwarder(FlowForwarderPtrWeak ff) { flow_forwarder_= ff; };
        FlowForwarderPtrWeak getFlowForwarder() { return flow_forwarder_;};

        void setHeader(unsigned char *raw_packet)
        {
                http_header_ = reinterpret_cast <unsigned char*> (raw_packet);
        }


        // Condition for say that a payload is HTTP 
        bool httpChecker(Packet& packet)
        {
		const char * paco = reinterpret_cast<const char*>(packet.getPayload());
		
		if(boost::regex_search(paco, what_, http_regex_)) 
                {
			setHeader(packet.getPayload());
                        ++total_validated_packets_;
                        return true;
                }
                else
                {
                        ++total_malformed_packets_;
                        return false;
                }
        }

	unsigned char *getPayload() { return http_header_; };

        void createHTTPHosts(int number) { host_cache_->create(number);};
        void destroyHTTPHosts(int number) { host_cache_->destroy(number);};
        void createHTTPUserAgents(int number) { ua_cache_->create(number);};
        void destroyHTTPUserAgents(int number) { ua_cache_->destroy(number);};

private:

	void extractHostValue(Flow *flow, const char *header);
	void extractUserAgentValue(Flow *flow, const char *header);

	FlowForwarderPtrWeak flow_forwarder_;
	boost::regex http_regex_,http_host_,http_ua_,http_referer_;
        boost::cmatch what_;
	MultiplexerPtrWeak mux_;
	unsigned char *http_header_;
	int64_t total_bytes_;

	Cache<HTTPHost>::CachePtr host_cache_;
	Cache<HTTPUserAgent>::CachePtr ua_cache_;

	typedef std::map<std::string,std::pair<HTTPHostPtr,int32_t>> HostMapType;
	typedef std::map<std::string,std::pair<HTTPUserAgentPtr,int32_t>> UAMapType;
	UAMapType ua_map_;	
	HostMapType host_map_;	
};

typedef std::shared_ptr<HTTPProtocol> HTTPProtocolPtr;

#endif
