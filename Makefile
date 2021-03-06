PLUGIN_NAME = ss_plant


HELP_DIR=/home/adam/RTXI/module_help
PLDS_DIR=$(HELP_DIR)/plds_adam/plds_adamX/plds_adamX

HEADERS = ss_plant.h\
	$(PLDS_DIR)/plds_adam_funs.hpp

SOURCES = ss_plant.cpp\
          moc_ss_plant.cpp\
	  $(HELP_DIR)/StAC_rtxi/dataFuns.cpp\
	  $(PLDS_DIR)/plds_adam_funs.cpp

LIBS = 

CXXFLAGS += -I$(HELP_DIR)
CXXFLAGS += -I$(PLDS_DIR)



# dynCtrlEst
#include Makefile.include_plds

# armadillo
include Makefile.include_arm

### Do not edit below this line ###

include $(shell rtxi_plugin_config --pkgdata-dir)/Makefile.plugin_compile
