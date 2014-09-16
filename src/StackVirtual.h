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
#ifndef SRC_STACKVIRTUAL_H_
#define SRC_STACKVIRTUAL_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <chrono>
#include <string>
#ifdef HAVE_LIBLOG4CXX
#include "log4cxx/logger.h"
#endif
#include "Multiplexer.h"
#include "FlowForwarder.h"
#include "./ethernet/EthernetProtocol.h"
#include "./vlan/VLanProtocol.h"
#include "./mpls/MPLSProtocol.h"
#include "./ip/IPProtocol.h"
#include "./udp/UDPProtocol.h"
#include "./tcp/TCPProtocol.h"
#include "./tcpgeneric/TCPGenericProtocol.h"
#include "./udpgeneric/UDPGenericProtocol.h"
#include "./icmp/ICMPProtocol.h"
#include "./vxlan/VxLanProtocol.h"
#include "./http/HTTPProtocol.h"
#include "./ssl/SSLProtocol.h"
#include "./dns/DNSProtocol.h"
#include "./flow/FlowManager.h"
#include "./flow/FlowCache.h"
#include "./frequency/FrequencyProtocol.h"
#include "NetworkStack.h"
#include "DatabaseAdaptor.h"

namespace aiengine {

class StackVirtual: public NetworkStack
{
public:
	explicit StackVirtual();
        virtual ~StackVirtual() {}

        const char* getName() { return name_.c_str();} 
        void setName(char *name) { name_ = name;}

        void setLinkLayerMultiplexer(MultiplexerPtrWeak mux) { }
        MultiplexerPtrWeak getLinkLayerMultiplexer() { return mux_eth_;}
	
	void showFlows(std::basic_ostream<char>& out);
	void showFlows() { showFlows(std::cout);}

	void setTotalTCPFlows(int value);
	void setTotalUDPFlows(int value);

	void enableNIDSEngine(bool enable);
	void enableFrequencyEngine(bool enable);
	void enableLinkLayerTagging(std::string type);

	void setFlowsTimeout(int timeout);

#ifdef PYTHON_BINDING
        FlowManager &getTCPFlowManager() { return *flow_table_tcp_vir_.get();}
        FlowManager &getUDPFlowManager() { return *flow_table_udp_vir_.get();}
	
	void setTCPIPSetManager(IPSetManager& ipset_mng) { tcp_vir_->setIPSetManager(ipset_mng);}
        void setUDPIPSetManager(IPSetManager& ipset_mng) { udp_vir_->setIPSetManager(ipset_mng);}
#else
        void setTCPIPSetManager(SharedPointer<IPSetManager> ipset_mng) { tcp_vir_->setIPSetManager(ipset_mng);}
        void setUDPIPSetManager(SharedPointer<IPSetManager> ipset_mng) { udp_vir_->setIPSetManager(ipset_mng);}

        FlowManagerPtrWeak getTCPFlowManager() { return flow_table_tcp_vir_;}
        FlowManagerPtrWeak getUDPFlowManager() { return flow_table_udp_vir_;}
#endif

private:
	std::string name_;
#ifdef HAVE_LIBLOG4CXX
	static log4cxx::LoggerPtr logger;
#endif
        //Protocols
	EthernetProtocolPtr eth_;
	VLanProtocolPtr vlan_;
	MPLSProtocolPtr mpls_;
	IPProtocolPtr ip_;
        UDPProtocolPtr udp_;
	VxLanProtocolPtr vxlan_;
	EthernetProtocolPtr eth_vir_;
	IPProtocolPtr ip_vir_;
	UDPProtocolPtr udp_vir_;
        TCPProtocolPtr tcp_vir_;
        ICMPProtocolPtr icmp_;
        HTTPProtocolPtr http_;
        SSLProtocolPtr ssl_;
        DNSProtocolPtr dns_;
	TCPGenericProtocolPtr tcp_generic_;
	UDPGenericProtocolPtr udp_generic_;
	FrequencyProtocolPtr freqs_tcp_;
	FrequencyProtocolPtr freqs_udp_;

        // Multiplexers
        MultiplexerPtr mux_eth_;
        MultiplexerPtr mux_vlan_;
        MultiplexerPtr mux_mpls_;
        MultiplexerPtr mux_ip_;
        MultiplexerPtr mux_udp_;
        MultiplexerPtr mux_vxlan_;
        MultiplexerPtr mux_eth_vir_;
        MultiplexerPtr mux_ip_vir_;
        MultiplexerPtr mux_udp_vir_;
        MultiplexerPtr mux_tcp_vir_;
        MultiplexerPtr mux_icmp_;

        // FlowManager and FlowCache
	FlowManagerPtr flow_table_udp_;
        FlowCachePtr flow_cache_udp_;

        FlowManagerPtr flow_table_udp_vir_;
        FlowManagerPtr flow_table_tcp_vir_;
        FlowCachePtr flow_cache_udp_vir_;
        FlowCachePtr flow_cache_tcp_vir_;

        // FlowForwarders
	FlowForwarderPtr ff_vxlan_;
	FlowForwarderPtr ff_udp_;

        FlowForwarderPtr ff_tcp_vir_;
        FlowForwarderPtr ff_udp_vir_;
        FlowForwarderPtr ff_http_;
        FlowForwarderPtr ff_ssl_;
	FlowForwarderPtr ff_dns_;
	FlowForwarderPtr ff_tcp_generic_;
	FlowForwarderPtr ff_udp_generic_;
	FlowForwarderPtr ff_tcp_freqs_;
	FlowForwarderPtr ff_udp_freqs_;
};

typedef std::shared_ptr<StackVirtual> StackVirtualPtr;

} // namespace aiengine

#endif  // SRC_STACKVIRTUAL_H_