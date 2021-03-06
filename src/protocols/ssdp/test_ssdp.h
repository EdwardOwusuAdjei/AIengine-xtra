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
#ifndef _test_ssdp_H_
#define _test_ssdp_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBLOG4CXX
#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#endif

#include <string>
//#include "../test/dns_test_packets.h"
#include "Protocol.h"
#include "Multiplexer.h"
#include "../ethernet/EthernetProtocol.h"
#include "../ip/IPProtocol.h"
#include "../udp/UDPProtocol.h"
#include "SSDPProtocol.h"

using namespace aiengine;

struct StackSSDPtest
{
        //Protocols
        EthernetProtocolPtr eth;
        IPProtocolPtr ip;
        UDPProtocolPtr udp;
        SSDPProtocolPtr ssdp;

        // Multiplexers
        MultiplexerPtr mux_eth;
        MultiplexerPtr mux_ip;
        MultiplexerPtr mux_udp;

        // FlowManager and FlowCache
        FlowManagerPtr flow_mng;
        FlowCachePtr flow_cache;

        // FlowForwarders
        FlowForwarderPtr ff_udp;
        FlowForwarderPtr ff_ssdp;

        StackSSDPtest()
        {
#ifdef HAVE_LIBLOG4CXX
                log4cxx::BasicConfigurator::configure();
#endif
                // Allocate all the Protocol objects
                udp = UDPProtocolPtr(new UDPProtocol());
                ip = IPProtocolPtr(new IPProtocol());
                eth = EthernetProtocolPtr(new EthernetProtocol());
                ssdp = SSDPProtocolPtr(new SSDPProtocol());

                // Allocate the Multiplexers
                mux_eth = MultiplexerPtr(new Multiplexer());
                mux_ip = MultiplexerPtr(new Multiplexer());
                mux_udp = MultiplexerPtr(new Multiplexer());

                // Allocate the flow caches and tables
                flow_mng = FlowManagerPtr(new FlowManager());
                flow_cache = FlowCachePtr(new FlowCache());

                ff_udp = FlowForwarderPtr(new FlowForwarder());
                ff_ssdp = FlowForwarderPtr(new FlowForwarder());

                //configure the eth
                eth->setMultiplexer(mux_eth);
                mux_eth->setProtocol(static_cast<ProtocolPtr>(eth));
                mux_eth->setProtocolIdentifier(0);
                mux_eth->setHeaderSize(eth->getHeaderSize());
                mux_eth->addChecker(std::bind(&EthernetProtocol::ethernetChecker,eth,std::placeholders::_1));

                // configure the ip
                ip->setMultiplexer(mux_ip);
                mux_ip->setProtocol(static_cast<ProtocolPtr>(ip));
                mux_ip->setProtocolIdentifier(ETHERTYPE_IP);
                mux_ip->setHeaderSize(ip->getHeaderSize());
                mux_ip->addChecker(std::bind(&IPProtocol::ipChecker,ip,std::placeholders::_1));
                mux_ip->addPacketFunction(std::bind(&IPProtocol::processPacket,ip,std::placeholders::_1));

                //configure the udp
                udp->setMultiplexer(mux_udp);
                mux_udp->setProtocol(static_cast<ProtocolPtr>(udp));
                ff_udp->setProtocol(static_cast<ProtocolPtr>(udp));
                mux_udp->setProtocolIdentifier(IPPROTO_UDP);
                mux_udp->setHeaderSize(udp->getHeaderSize());
                mux_udp->addChecker(std::bind(&UDPProtocol::udpChecker,udp,std::placeholders::_1));
                mux_udp->addPacketFunction(std::bind(&UDPProtocol::processPacket,udp,std::placeholders::_1));

                // configure the dns 
                ssdp->setFlowForwarder(ff_ssdp);
                ff_ssdp->setProtocol(static_cast<ProtocolPtr>(ssdp));
                ff_ssdp->addChecker(std::bind(&SSDPProtocol::ssdpChecker,ssdp,std::placeholders::_1));
                ff_ssdp->addFlowFunction(std::bind(&SSDPProtocol::processFlow,ssdp,std::placeholders::_1));

                // configure the multiplexers
                mux_eth->addUpMultiplexer(mux_ip,ETHERTYPE_IP);
                mux_ip->addDownMultiplexer(mux_eth);
                mux_ip->addUpMultiplexer(mux_udp,IPPROTO_UDP);
                mux_udp->addDownMultiplexer(mux_ip);

                // Connect the FlowManager and FlowCache
                flow_cache->createFlows(2);

		ssdp->setFlowManager(flow_mng);
		ssdp->createSSDPInfos(2);

                udp->setFlowCache(flow_cache);
                udp->setFlowManager(flow_mng);

                // Configure the FlowForwarders
                udp->setFlowForwarder(ff_udp);

                ff_udp->addUpFlowForwarder(ff_ssdp);

        }

	void show() {
		udp->setStatisticsLevel(5);
		udp->statistics();
		ssdp->setStatisticsLevel(5);
		ssdp->statistics();
	}

	void showFlows() {

		flow_mng->showFlows();

	}

        ~StackSSDPtest()
        {
        }
};

#endif
