# Makefile for mac/sr/cfg/cfgUtil

CC = gcc
CFLAGS += -g -I.. -I../../../include 

GENERATED=../cfgParamName.c \
		../../../export/wniCfgAp.bin ../../../export/wniCfgSta.bin \
		../../../export/wniCfgAp.h ../../../export/wniCfgSta.h

# Generate locally executable cfgGen
all: cfgGen

#all: cfgGen cfg checkoutCfg updateCfg
# Generate locally executable cfgGen
#   use it to generate new config files in this directory
#   check out the config files in the other directory
#   move the generated config files to their checked in locations

# User must still manually checkout cfg.txt
#   and check it and generated files in on completion
#   (p4 submit will not take a list of files)

cfgall: cfg checkoutCfg updateCfg

cfg: cfgGen cfg.txt
	./cfgGen

checkoutCfg:
	p4 edit $(GENERATED)

updateCfg:
	mv -f cfgParamName.c ..
	mv -f wniCfgSta.bin ../../../export
	mv -f wniCfgAp.bin ../../../export
	mv -f wniCfgSta.h ../../../export
	mv -f wniCfgAp.h ../../../export

cmpCfg:
	-diff ../cfgParamName.c cfgParamName.c
	-cmp -l ../../../export/wniCfgSta.bin wniCfgSta.bin
	-cmp -l ../../../export/wniCfgAp.bin wniCfgAp.bin
	-diff ../../../export/wniCfgSta.h wniCfgSta.h
	-diff ../../../export/wniCfgAp.h wniCfgAp.h

revertCfg:
	p4 revert $(GENERATED)

.PHONY: cfgall cfg checkoutCfg updateCfg cmpCfg  revertCfg
