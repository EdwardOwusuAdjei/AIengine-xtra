#!/usr/bin/env python

""" Example for integrating pyaiengine with cassandra """

__author__ = "Luis Campo Giralte"
__copyright__ = "Copyright (C) 2013-2016 by Luis Campo Giralte"
__revision__ = "$Id$"
__version__ = "0.1"
import sys
import os
sys.path.append("../src/")
import pyaiengine
import pycassa
import json

class cassandraAdaptor(pyaiengine.DatabaseAdaptor):
    """ This class inheritance of DatabaseAdaptor that contains 
        the following methods:
        - insert, called on the first insertion of the network flow
        - update, called depending on the sample selected.
        - remove, called when the flow is destroy.
    """
    def __init__(self):
        self.__c = None 
        self.__pool = None

    def connect(self,connection_str):
        self.__pool = pycassa.ConnectionPool(keyspace='demo', server_list=['127.0.0.1:9160'], prefill=False)
        self.__c = pycassa.ColumnFamily(self.__pool, 'flows')	

    def update(self,key,data):
        obj = json.loads(data)   
        
        bytes = obj["bytes"]
        l7 = obj["layer7"]
        l7info = obj.get("httphost",0)
        if (l7info == 0):
            l7info = obj.get("sslphost",0)
            if ( l7info > 0): 
                d["layer7info"] = l7info
        else: 
            d["layer7info"] = l7info
        # Create a dict with all the values of the cassandra table 
        d = {'bytes':bytes,'layer7':l7}
        
        self.__c.insert(key,d)
    
    def insert(self,key):
        self.__c.insert(key,{'bytes':0})

    def remove(self,key):
        # We dont remove anything on this example 
        pass

if __name__ == '__main__':

    # Load an instance of a Mobile Network
    st = pyaiengine.StackMobile()

    st.tcp_flows = 327680
    st.udp_flows = 163840
 
    """
     Create a cassandraAdaptor object. 
    This is just and example you can create your own adaptor for
    any database, or file, or whatever you decide.
    """
    db = cassandraAdaptor()
    # connect to the cassandra database instance 
    db.connect("localhost")
 
    """ 
    Set the database adaptor just for UDP traffic
        and with a packet sampling of 512 packets, so every 512 packets
        the method "update" will be called.
        Fix this value depending on your software/hardware requirments.
    """
    st.set_udp_database_adaptor(db,16)
    st.set_tcp_database_adaptor(db,16)

    with pyaiengine.PacketDispatcher("eth0") as pd:
        """ Plug the stack on the PacketDispatcher """
        pd.stack = st 
        pd.run()

    sys.exit(0)

