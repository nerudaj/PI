# P4 Runtime support for Combo-CG

## Introduction

Combo-CG is a network card developed by CESNET and Netcope Technologies. Files
in this folder implements the support for this card within the P4 Runtime. Two
modes of compilation are available - testing and release.

## Testing

This is the default mode. There is a set of automated test that are performed
using the PI CLI and are performed without the Combo-CG card, using a dummy
implementation of libp4dev that you can find in the `dummy` folder.

To run the tests, configure the repo without any suplementary flags and
run the combo_test.sh in the root of the repo.

## Release

To compile in the release mode, you have to change linkage in the `/CLI/Makefile.am`.
Starting at the line 60, change this:
```
pi_CLI_combo_LDADD = $(common_libs) \
-L$(top_builddir)/targets/combo/dummy -lp4dev-d \
$(top_builddir)/targets/combo/libpi_combo.la 
```

to this:
```
pi_CLI_combo_LDADD = $(common_libs) \
-lp4dev \
$(top_builddir)/targets/combo/libpi_combo.la 
```

## Demo with the controller

You can also tinker with the demo controller+SDN app implementation. For
this make sure you have protobuf and grpc installed and run the
combo_demo.sh script. It will properly reconfigure the repo and compile
dependencies.

After success, if you are using dummy libp4dev, you have to go to
`/targets/combo/dummy` and `make install` it.

Then go to `/proto/demo_grpc` and launch the demo according to the instructions
found there.

If you want to run production version of the demo, change `/proto/demo_grpc/Makefile.am`,
starting at line 51 from this:
```
pi_grpc_server_LDADD = \
$(COMMON_SERVER_LIBS) \
-L../../targets/combo/dummy -lp4dev-d \
$(top_builddir)/../targets/combo/libpi_combo.la \
-lthrift -lruntimestubs -lsimpleswitch_thrift
```

to this:

```
pi_grpc_server_LDADD = \
$(COMMON_SERVER_LIBS) \
-lp4dev \
$(top_builddir)/../targets/combo/libpi_combo.la \
-lthrift -lruntimestubs -lsimpleswitch_thrift
```