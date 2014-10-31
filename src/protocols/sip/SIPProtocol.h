/*
 * AIEngine a deep packet inspector reverse engineering engine.
 *
 * Copyright (C) 2013-2014  Luis Campo Giralte
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
#pragma GCC diagnostic ignored "-Wwrite-strings"
#ifndef SRC_PROTOCOLS_SIP_SIPPROTOCOL_H_
#define SRC_PROTOCOLS_SIP_SIPPROTOCOL_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBLOG4CXX
#include "log4cxx/logger.h"
#endif
#include "Protocol.h"
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "SIPUri.h"
#include "SIPTo.h"
#include "SIPFrom.h"
#include "Cache.h"
#include <unordered_map>
#include "names/DomainNameManager.h"
#include "regex/Regex.h"
#include "flow/FlowManager.h"

namespace aiengine {

class SIPProtocol: public Protocol 
{
public:
    	explicit SIPProtocol():Protocol(SIPProtocol::default_name),stats_level_(0),
                sip_regex_(new Regex("Main SIP expression","^(REGISTER|INVITE|OPTIONS).*SIP/2.")),
                sip_from_(new Regex("From expression","From: .*?\r\n")),
                sip_to_(new Regex("To expression","To: .*?\r\n")),
		sip_header_(nullptr),total_bytes_(0),
		total_requests_(0),
		uri_cache_(new Cache<SIPUri>("Uri cache")),
		from_cache_(new Cache<SIPFrom>("From cache")),
		to_cache_(new Cache<SIPTo>("To cache")),
		to_map_(),from_map_(),uri_map_(),
		flow_mng_() {}	

    	virtual ~SIPProtocol() {}

	static constexpr char *default_name = "SIPProtocol";
	static const uint16_t id = 0;
	static const int header_size = 0;
	int getHeaderSize() const { return header_size;}

	int64_t getTotalBytes() const { return total_bytes_; }
	int64_t getTotalPackets() const { return total_packets_;}
	int64_t getTotalValidatedPackets() const { return total_validated_packets_;}
	int64_t getTotalMalformedPackets() const { return total_malformed_packets_;}

	void processPacket(Packet& packet) { /* Nothing to process at packet level*/ }
	void processFlow(Flow *flow);

	void setStatisticsLevel(int level) { stats_level_ = level;}
	void statistics(std::basic_ostream<char>& out);
	void statistics() { statistics(std::cout);}

	void releaseCache(); // Three caches will be clear 

        void setHeader(unsigned char *raw_packet) {
        
                sip_header_ = reinterpret_cast <unsigned char*> (raw_packet);
        }

        // Condition for say that a payload is SIP
        bool sipChecker(Packet& packet) {
        
		const char * header = reinterpret_cast<const char*>(packet.getPayload());

		if (sip_regex_->evaluate(header)) {

			setHeader(packet.getPayload());
                        ++total_validated_packets_;
                        return true;
                } else {
                        ++total_malformed_packets_;
                        return false;
                }
        }

	unsigned char *getPayload() { return sip_header_; }

        void createSIPUris(int number) { uri_cache_->create(number);}
        void destroySIPUris(int number) { uri_cache_->destroy(number);}
        void createSIPFroms(int number) { from_cache_->create(number);}
        void destroySIPFroms(int number) { from_cache_->destroy(number);}
        void createSIPTos(int number) { to_cache_->create(number);}
        void destroySIPTos(int number) { to_cache_->destroy(number);}

	void setFlowManager(FlowManagerPtrWeak flow_mng) { flow_mng_ = flow_mng; }

private:

	void attach_uri_to_flow(Flow *flow, std::string &host);
	void attach_from_to_flow(Flow *flow, std::string &host);
	void attach_to_to_flow(Flow *flow, std::string &ua);
	void extract_uri_value(Flow *flow, const char *header);
	void extract_from_value(Flow *flow, const char *header);
	void extract_to_value(Flow *flow, const char *header);

	int stats_level_;
	SharedPointer<Regex> sip_regex_,sip_from_,sip_to_;
	unsigned char *sip_header_;
	int64_t total_bytes_;
	int32_t total_requests_;

	Cache<SIPUri>::CachePtr uri_cache_;
	Cache<SIPFrom>::CachePtr from_cache_;
	Cache<SIPTo>::CachePtr to_cache_;

	typedef std::pair<SharedPointer<SIPUri>,int32_t> UriHits;
	typedef std::pair<SharedPointer<SIPFrom>,int32_t> FromHits;
	typedef std::pair<SharedPointer<SIPTo>,int32_t> ToHits;

	typedef std::map<std::string,UriHits> UriMapType;
	typedef std::map<std::string,FromHits> FromMapType;
	typedef std::map<std::string,ToHits> ToMapType;
	FromMapType from_map_;	
	ToMapType to_map_;	
	UriMapType uri_map_;	

	FlowManagerPtrWeak flow_mng_;
#ifdef HAVE_LIBLOG4CXX
	static log4cxx::LoggerPtr logger;
#endif
};

typedef std::shared_ptr<SIPProtocol> SIPProtocolPtr;

} // namespace aiengine

#endif  // SRC_PROTOCOLS_SIP_SIPPROTOCOL_H_