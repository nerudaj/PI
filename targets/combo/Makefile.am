# a little hacky: the headers are in the parent of the configure subdir
AM_CPPFLAGS += \
-I$(top_srcdir)/../../include

if WITH_PCAP_FIX
AM_CPPFLAGS += -DWITH_PCAP_FIX
endif

libpi_combo_la_SOURCES = \
pi_imp.cpp \
pi_tables_imp.cpp \
pi_act_prof_imp.cpp \
pi_counter_imp.cpp \
pi_meter_imp.cpp \
pi_learn_imp.cpp

libpi_combo_la_LIBADD = 
lib_LTLIBRARIES = libpi_combo.la
