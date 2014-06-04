#!/usr/bin/env python
#
#  AIEngine.
#
# Copyright (C) 2013  Luis Campo Giralte
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
# Boston, MA  02110-1301, USA.
#
# Written by Luis Campo Giralte <luis.camp0.2009@gmail.com> 2013
#
""" Example for detect unknown activity on the network and detect it """
__author__ = "Luis Campo Giralte"
__copyright__ = "Copyright (C) 2013 by Luis Campo Giralte"
__revision__ = "$Id$"
__version__ = "0.1"
import sys
import os
sys.path.append("../src/")
import pyaiengine

def unknown_callback(flow):

    print("Detecting unknown traffic on:",str(flow))

if __name__ == '__main__':
    
    # Load an instance of a Network Stack on a Lan network
    st1 = pyaiengine.StackLan()

    # Create a instace of a PacketDispatcher
    pdis = pyaiengine.PacketDispatcher()

    # Plug the stack on the PacketDispatcher
    pdis.setStack(st1)

    st1.setTotalUDPFlows(16384)
    st1.setTotalTCPFlows(16384)

    """ Generate two instances of the FrequencyGroup and LearnerEngine """
    learn = pyaiengine.LearnerEngine()
    freq = pyaiengine.FrequencyGroup()

    """ Tell the stack that should store the payloads """ 
    st1.enableFrequencyEngine(True)
       
    """ Open the pcapfile and process """ 
    pdis.open("unknown_traffic.pcap")
    pdis.run()
    pdis.close()

    """ Asuming that the unknown traffic is TCP """
    ft = st1.getTCPFlowManager()

    """ Use the method most suitable for your case """
    freq.addFlowsByDestinationPort(ft)
    freq.compute()

    flow_list = freq.getReferenceFlows()
    learn.agregateFlows(flow_list)
    learn.compute()

    """ Get the generated regex """
    r_candidate = learn.getRegex()
  
    print("Generated Regex:",r_candidate)
    rm = pyaiengine.RegexManager()
    r = pyaiengine.Regex("Unknown attack/traffic",r_candidate)
    r.setCallback(unknown_callback)
    rm.addRegex(r)    

    """ We create another clean StackLan """
    st2 = pyaiengine.StackLan()
    
    st2.setTCPRegexManager(rm)
    st2.enableNIDSEngine(True)    

    st2.setTotalUDPFlows(16384)
    st2.setTotalTCPFlows(16384)
  
    """ Plug a new stack """ 
    pdis.setStack(st2)
 
    pdis.open("eth0")

    try:
        pdis.run()
    except:
        e = sys.exc_info()[0]
        print("Interrupt during capturing packets:",e)
     
    pdis.close()
 
    sys.exit(0)