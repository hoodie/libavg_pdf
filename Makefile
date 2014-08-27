PLUGIN_NAME = popplernode
PLUGIN_SOURCES = \
	PopplerNode.cpp

AVG_PATH=/home/hendrik/code/hub

ADDITIONAL_PKGCONFIG =
ADDITIONAL_LINKOPT =

all: testpath $(PLUGIN_NAME).so

testpath:
ifndef AVG_PATH
	@echo "Please define AVG_PATH"
	@exit 1
endif
INCLUDE = -I.
INCLUDE += -I$(AVG_PATH)/libavg/src
CONFIG = $(shell pkg-config gdk-pixbuf-2.0 --cflags)
CONFIG += $(shell pkg-config sdl --cflags)
CONFIG += $(shell pkg-config freetype2 --cflags)
CONFIG += $(shell pkg-config pango --cflags)
CONFIG += $(shell python2-config --includes)
CONFIG += $(ADDITIONAL_PKGCONFIG)
CONFIG += -DAVG_PLUGIN

CONFIG += $(shell pkg-config poppler poppler-glib --cflags)
LIBS   += $(shell pkg-config poppler poppler-glib --libs)

CXXOPT = -Wall -Wno-invalid-offsetof -O3 -fPIC -DPOPPLERNODE

ifneq (,$(findstring Linux,$(shell uname)))
	AVG_LIB_PATH = $(shell python -c "import os;import libavg;print os.path.dirname(libavg.__file__)" 2>/dev/null)
	CONFIG += $(shell pkg-config libxml-2.0 --cflags)
	LINKOPT = -fPIC -shared -shared-libgcc
	LIBS += -lx264 -lpthread -lturbojpeg -lboost_system $(AVG_LIB_PATH)/avg.so
	CXXFLAGS += -gstabs
else
ifneq (,$(findstring Darwin,$(shell uname)))
	@echo "Not yet supported platform, please adapt the linux build"
	@exit 1
	LINKOPT = -bundle -flat_namespace -undefined suppress
	CONFIG += -I/usr/include/libxml2
	LIBS += -lxml2 -L$(AVG_PATH)/lib -lturbojpeg
	CXXFLAGS += -g
else
	@echo "Unsupported platform"
	@exit 1
endif
endif

CXX = c++ $(CXXOPT) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDE) $(CONFIG)

LINKOPT += $(ADDITIONAL_LINKOPT)

PLUGIN_OBJECTS = $(PLUGIN_SOURCES:.cpp=.o)

$(PLUGIN_NAME).so: $(PLUGIN_OBJECTS)
	$(CXX) $(LINKOPT) -o $@ $^ $(LIBS)

$(PLUGIN_OBJECTS): %.o: %.cpp
	$(CXX) -c $< -o $@

%.d: %.cpp
	@echo "Generating dependencies for $<"
	@set -e; rm -f $@; \
	$(CXX) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

check:
	@cd ../test; python test.py

install: $(PLUGIN_NAME).so deploypath
	@set -e; \
	if [ ! -d "$(AVG_DEPLOY_PATH)" ]; \
	then \
		echo "Can't find a valid path for libavg python package"; \
		echo "If you're on OSX, make sure your libavg autoconf prefix is the default one."; \
		exit 1; \
	else \
		if [ ! -d "$(AVG_DEPLOY_PATH)/plugins" ]; \
		then \
			echo "Creating plugins directory"; \
			mkdir -p $(AVG_DEPLOY_PATH)/plugins; \
		fi; \
		echo "Installing plugin $(PLUGIN_NAME)"; \
		install -m 644 $(PLUGIN_NAME).so $(AVG_DEPLOY_PATH)/plugins/$(PLUGIN_NAME).so; \
	fi

deploypath:
AVG_DEPLOY_PATH = $(shell python -c "import os;import libavg;print os.path.dirname(libavg.__file__)" 2>/dev/null)

clean:
	rm -f *.o *.d *.d.* $(PLUGIN_NAME).so

# Automatic dependencies inclusion
-include $(PLUGIN_SOURCES:.cpp=.d)
