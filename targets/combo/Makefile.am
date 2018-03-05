# a little hacky: the headers are in the parent of the configure subdir
AM_CPPFLAGS = \
-I$(top_srcdir)/include \
-I$(top_srcdir)/lib \
-std=c++11 \
-DDEBUG_LOGS

libpi_combo_la_SOURCES = \
pi_imp.cpp \
pi_tables_imp.cpp \
pi_act_prof_imp.cpp \
pi_counter_imp.cpp \
pi_meter_imp.cpp \
pi_learn_imp.cpp \
api/Device.cpp \
api/Table.cpp \
api/RuleSet.cpp \
devices.cpp 

lib_LTLIBRARIES = libpi_combo.la
