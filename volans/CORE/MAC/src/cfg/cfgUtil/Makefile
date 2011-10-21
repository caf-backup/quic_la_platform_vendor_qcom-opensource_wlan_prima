# Makefile for mac/sr/cfg/cfgUtil

CC = gcc
CFLAGS += -g -I.. -I../../../include 

GENERATED=../cfgParamName.c ../../../inc/cfg.dat \
		../../../inc/wniCfgAp.bin ../../../inc/wniCfgSta.bin \
		../../../inc/wniCfgAp.h ../../../inc/wniCfgSta.h

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
	mv -f cfg.dat ../../../inc
	mv -f wniCfgSta.bin ../../../inc
	mv -f wniCfgAp.bin ../../../inc
	mv -f wniCfgSta.h ../../../inc
	mv -f wniCfgAp.h ../../../inc

cmpCfg:
	-diff ../cfgParamName.c cfgParamName.c
	-diff ../../../inc/cfg.dat cfg.dat
	-cmp -l ../../../inc/wniCfgSta.bin wniCfgSta.bin
	-cmp -l ../../../inc/wniCfgAp.bin wniCfgAp.bin
	-diff ../../../inc/wniCfgSta.h wniCfgSta.h
	-diff ../../../inc/wniCfgAp.h wniCfgAp.h

revertCfg:
	p4 revert $(GENERATED)

.PHONY: cfgall cfg checkoutCfg updateCfg cmpCfg  revertCfg
