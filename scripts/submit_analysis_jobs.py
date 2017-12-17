#!/usr/bin/env python
"""Submit l1analysis histogram jobs"""
import argparse
import os

QUEUE = "8nh"

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
    cmd += " -o def.log \'l1jetanalysis.exe def "
    cmd += ARGS.default
    cmd += "; cp l1analysis_def.root '`pwd`"
    os.system(cmd)
if(ARGS.new):
    cmd = BASECMD
    cmd += " -o new.log \'l1jetanalysis.exe new "
    cmd += ARGS.new
    cmd += "; cp l1analysis_new_cond.root '`pwd`"
    os.system(cmd)
