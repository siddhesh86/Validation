#!/usr/bin/env python
"""Submit rate histogram jobs"""
import argparse
import os

QUEUE = "1nd"

PARSER = argparse.ArgumentParser()
PARSER.add_argument('-d', '--default')
PARSER.add_argument('-n', '--new')
PARSER.add_argument('-q', '--queue')
ARGS = PARSER.parse_args()
if(ARGS.queue):
    QUEUE = ARGS.queue

BASECMD = "bsub -q " + QUEUE

if(ARGS.default):
    cmd = BASECMD
    cmd += " -o def.log \'rates.exe def "
    cmd += ARGS.default
    cmd += "; cp rates_def.root '`pwd`"
    os.system(cmd)
if(ARGS.new):
    cmd = BASECMD
    cmd += " -o new.log \'rates.exe new "
    cmd += ARGS.new
    cmd += "; cp rates_new_cond.root '`pwd`"
    os.system(cmd)
