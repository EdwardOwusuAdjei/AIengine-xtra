AUTOMAKE_OPTIONS = gnu

noinst_HEADERS =	test_vlan.h

TEST = test_vlan

check_PROGRAMS = test_vlan

test_vlan_SOURCES = 	../../Multiplexer.cc ../../AnomalyManager.cc \
			../ethernet/EthernetProtocol.cc \
			VLanProtocol.cc test_vlan.cc

test_vlan_CPPFLAGS = -I../.. $(BOOST_CPPFLAGS) -DSTAND_ALONE
test_vlan_LDADD   = $(BOOST_LDFLAGS) -lboost_unit_test_framework

CLEANFILES = *.o *.lo *.so

MAINTAINERCLEANFILES = \
        $(srcdir)/Makefile \
        $(srcdir)/Makefile.in \
        `rm -rf $(srcdir)/build `

