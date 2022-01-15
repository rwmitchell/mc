# Makefile

CC_DEBUG_FLAGS    =-g3 -DDEBUG_ALL
CC_CHECK_FLAGS    = --analyzer-output text --analyze -I$(SRC)
CC_RELEASE_FLAGS  = 

RLS  = release
DBG  = debug
PTH := $(RLS)
RUN  = all

list:

column = sed 's/ / 	/g' | tr ' |' '\n\n'


DIR = $(shell basename $(CURDIR))
BLD = ./Build
INC = $(BLD)/include
BAS = $(BLD)/$(PTH)/$(DIR)
DEP = $(BAS)/.dep

DST = $(BAS)/bin
OBJ = $(BAS)/obj
LIB = $(BAS)/lib

# Override this on the cmdline with: make prefix=/some/where/else
prefix = $(BLD)

SRC = Source
NST = $(prefix)/bin

MYINC = -I$(BLD)/include -I$(SRC)
# MYLIB = -L$(BLD)/lib -lmylib

.PHONY: check
check: CFLAGS = $(CC_CHECK_FLAGS)
check: .analyze

.PHONY: check_all
check_all: CFLAGS = $(CC_CHECK_FLAGS)
check_all: make_check_all

.PHONY: debug
debug: CFLAGS += $(CC_DEBUG_FLAGS) $(MYINC)
debug: PTH    := $(DBG)
debug: make_it

.PHONY: release
release: CFLAGS += $(CC_RELEASE_FLAGS) $(MYINC)
release: PTH    := $(RLS)
release: make_it


# Additional object files used with other programs
OBJ_FILES = \
	$(OBJ)/io.o       \
	$(OBJ)/loadfile.o \
	$(OBJ)/strings.o  \
	$(OBJ)/utils.o    \
	$(OBJ)/malloc.o   \
	$(OBJ)/helpd.o    \


NEEDSXTRA  =         \
	$(DST)/mc          \

# All C programs
DST_PROGS =          \
	$(DST)/mc          \

# All Scripts (basename, no extensions ie: foo, not foo.pl)
DST_SCRPT =          \


DIRS =    \
	$(DEP)  \
	$(OBJ)  \
	$(BAS)  \
	$(DST)  \
	$(NST)  \


$(NST)/%: $(DST)/%
	install -m ugo+rx $< $@

$(DST)/mc  : $(DST)/% : $(OBJ)/%.o $(OBJ_FILES)
	$(CC) -o $@ $^ $(MYLIB) -lm


NOMATH := $(filter-out $(NEEDSXTRA), $(NOMATH))

# $(filter-out $(NEEDSMATH), $(DST_PROGS)):	$(DST)/%:	$(OBJ)/%.o
$(NOMATH):	$(DST)/% : $(OBJ)/%.o $(OBJ_FILES)
	$(CC) -o $@ $^ $(MYLIB)

$(OBJ)/%.o	:	$(SRC)/%.c $(DEP)/%.d
	@ echo "|Making_OBJ:$@ $^ " | $(column)
	$(CC) $(CFLAGS) -c $< -o $@

#$(OBJ)/%.o:	$(SRC)/%.c
#	$(CC) -c $(CFLAGS) -I$(SRC) -o $@ $^

$(NEEDSMATH):	$(DST)/% : $(OBJ)/%.o
	$(CC) -o $@ $< $(OBJ_FILES) -lm $(MYLIB)

NST_PROGS = $(subst $(DST), $(NST), $(DST_PROGS))
NST_SCRPT = $(subst $(DST), $(NST), $(DST_SCRPT))

.PHONY: install real_install help

list:
	@echo all allsu install
	@echo $(DST_PROGS)
	@echo $(DST_SCRPT)
#@echo $(NST_PROGS)
#@echo $(NST_SCRPT)

all: \
	$(DIRS)       \
	$(OBJ_FILES)  \
	$(DST_PROGS)  \
	$(DST_SCRPT)  \
	tags types    \
	show_install  \

allsu: \
	$(NEEDSSUID)  \

install: real_install
	@true

installsu: real_installsu
	@true

real_install:        \
	$(NST)        \
	$(NST_SCRPT)  \
	$(NST_PROGS)  \

real_installsu:       \
	$(NST)/sucmd  \

$(DIRS):
	mkdir -p $@

show_install:
	@echo ""
	@echo "These programs need to be installed:"
	@make -sn install

help:
	@make -sn
	@echo "These programs are made:"
	@echo $(DST_PROGS) | tr ' ' '\n'
	@echo $(DST_SCRPT) | tr ' ' '\n'
	@echo
	@echo "Try: make install"

help_install:
	@echo "These programs are installed:"
	@echo
	@echo $(NST_PROGS) | tr ' ' '\n'
	@echo $(NST_SCRPT) | tr ' ' '\n'
	@echo

clean:
	$(RM) $(DEP)/*.d $(OBJ)/*.o $(DST_PROGS) $(DST_SCRPT)
	rmdir $(OBJ) $(DST)

foo:
	@ echo "OBJ  " $(OBJ)
	@ echo "SRC  " $(SRC)
	@ echo "SUID " $(NEEDSSUID)
	@ echo "obj  " $(OBJ_FILES)
	@ echo "HOST " $(HOST)
	@ echo "NSTS " $(NST_SCRPT) | $(column)

.analyze: $(wildcard $(SRC)/*.c)
	gcc $(CFLAGS) $?
	@ touch .analyze

make_check_all:
	@ rm .analyze  || true
	@ make CFLAGS="$(CFLAGS)" check


make_it:
	make PTH=$(PTH) CFLAGS="$(CFLAGS)" $(RUN)

#We don't need to clean up when we're making these targets
NODEPS:=clean tags svn install
#Find all the C++ files in the $(SRC)/ directory
SOURCES:=$(shell find $(SRC)  -name "*.c")
#These are the dependency files, which make will clean up after it creates them
DEPFILES:=$(patsubst %.c,%.d,$(patsubst $(SRC)/%,$(DEP)/%, $(SOURCES)))

#Don't create dependencies when we're cleaning, for instance
ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
    #Chances are, these files don't exist.  GMake will create them and
    #clean up automatically afterwards
    -include $(DEPFILES)
endif

#This is the rule for creating the dependency files
$(DEP)/%.d: $(SRC)/%.c $(DEP)
	@echo "START DEP: $@"
	@echo $(CC) $(CFLAGS) -MG -MM -MT '$(patsubst $(SRC)/%,$(OBJ)/%, $(patsubst %.c,%.o,$<))' $(MYINC) $<
	$(CC) $(CFLAGS) -MG -MM -MT '$(patsubst $(SRC)/%,$(OBJ)/%, $(patsubst %.c,%.o,$<))' $(MYINC) $< > $@
	@echo "END   DEP: $@"
# End of - Dependency code added here

# Make a highlight file for types.  Requires Exuberant ctags and awk

# Make a highlight file for types.  Requires Universal ctags and awk
types: $(SRC)/.types.vim
$(SRC)/.types.vim: $(SRC)/*.[ch]
	ctags --kinds-c=gstu -o- \
		$(SRC)/*.[ch] \
		$(INC)/*.h \
		| grep -v "^__anon" \
		| awk 'BEGIN{printf("syntax keyword Type\t")} \
		{printf("%s ", $$1)}END{print ""}' > $@
	ctags --kinds-c=d -o- \
		$(SRC)/*.h \
		$(INC)/*.h \
		| grep -v "^__anon" \
		| awk 'BEGIN{printf("syntax keyword Debug\t")}\
		{printf("%s ", $$1)}END{print ""}' >> $@
# End types

tags: $(SRC)/*.[ch]
	ctags --fields=+l --langmap=c:.c.h \
		$(SRC)/* \
		$(INC)/*

