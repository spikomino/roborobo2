DEBUG = 1
MODULES =  TopEDO
CC           = g++ 
CCFLAGS      = -Wall -O3 -std=c++0x
LDFLAGS      = `sdl-config --cflags --libs`
RM           = rm -f 
MAKE         = make 
INCLUDE      = -Iinclude/ext -Iinclude/contrib -Iinclude/core -Iprj/
SRC_C	     = $(wildcard src/core/*.c src/contrib/*.c src/contrib/**/*.c src/ext/*.c)
SRC_CC	     = $(wildcard src/core/*.cpp src/contrib/*.cpp src/contrib/**/*.cpp src/ext/*.cpp)
DIR_NAME     = $(shell basename `pwd`)
DATE_TIME    = `date +'%Y.%m.%d_%Hh%M'`

# if DEBUG is enabled, add -g option to CCFLAGS
ifneq ($(strip $(DEBUG)),)
	CCFLAGS += -g
endif

# if MODULES is empty, compile everything
ifeq ($(strip $(MODULES)),)
	MODULES := ${shell ls -1 prj | grep '/' | cut -d'/' -f 1}
endif

# Preprocessor variables associated with the modules to be compiled
MOD_VARS := -DMODULAR $(foreach m, $(MODULES), -DPRJ_${shell echo $(m) | tr [a-z] [A-Z] | sed 's/\-/\_/g'})
CCFLAGS += $(MOD_VARS)

# Update SRC_C And SRC_CC with modules files 
SRC_C += $(foreach m, $(MODULES), $(wildcard prj/$(m)/src/*.c) $(wildcard prj/$(m)/src/**/*.c))
SRC_CC += $(foreach m, $(MODULES), $(wildcard prj/$(m)/src/*.cpp) $(wildcard prj/$(m)/src/**/*.cpp))

OBJ          = $(SRC_C:.c=.o) $(SRC_CC:.cpp=.o)
TARGET       := roborobo

.PHONY: snapshot

.IGNORE: 

all: verifyMods $(TARGET)

verifyMods:
	@for i in $(MODULES); do \
		if [ ! -d prj/$$i ]; then \
			echo "WARNING : Project $$i does not exist !"; \
		fi; \
	done
	@if [ ! -d "logs" ]; then mkdir logs ; fi;


$(TARGET): $(OBJ)
ifeq ($(strip $(VERBOSE)),)
	@echo "[LD]	" $@
	@$(CC) $(LDFLAGS) -o $@ $^ -lSDL_image -lSDL 
else
	$(CC) $(LDFLAGS) -o $@ $^ -lSDL_image -lSDL 
endif

%.o: %.cpp
ifeq ($(strip $(VERBOSE)),)
	@echo "[CPP]	" $<
	@$(CC) -o $@ -c $< $(CCFLAGS) $(INCLUDE)
else
	$(CC) -o $@ -c $< $(CCFLAGS) $(INCLUDE)
endif

%.o: %.c
ifeq ($(strip $(VERBOSE)),)
	@echo "[C]	" $<
	@$(CC) -o $@ -c $< $(CCFLAGS) $(INCLUDE)
else
	$(CC) -o $@ -c $< $(CCFLAGS) $(INCLUDE)
endif

clean:
ifeq ($(strip $(VERBOSE)),)
	@$(RM) $(OBJ) 
else
	$(RM) $(OBJ) 
endif

distclean:
	@$(MAKE) clean 
	$(RM) $(TARGET) 

snapshot:
	clear
	@echo "Creating snapshot in current directory."
	echo Now: $(DATE_TIME)
	make distclean
	rm -f roborobo-snapshot*.tar
	cd .. ; export COPYFILE_DISABLE=TRUE ; tar --exclude='._*' --exclude='perso' --exclude='.DS_Store' --exclude='datalog_*' --exclude='properties_*' --exclude='roborobo-snapshot*' --exclude='.svn' --exclude='logs/*' -cvf $(DIR_NAME)/roborobo-snapshot-$(DATE_TIME).tar $(DIR_NAME)
	@echo =-=-=-= 
	@echo =-=-=-=
	@echo =-=-=-=
	@echo Snapshot is created as roborobo-snapshot-$(DATE_TIME).tar 
	@echo .
	@echo ALL files have been archived except:
	@echo - datalog_* and properties_* files are NOT in the archive, wherever they were.
	@echo - perso directory and content is NOT in the archive
	@echo - .svn directories and content are NOT in the archive
	@echo - note: dont forget to re-compile as a make distclean was performed prior to building the archive
	@echo =-=-=-=
	@echo =-=-=-=
	@echo =-=-=-=

	

