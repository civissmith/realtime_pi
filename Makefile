################################################################################
# @Title: Makefile
#
# @Author: Phil Smith
#
# @Date: Thu, 10-Sep-15 04:04PM
#
# @Project: Multithreading Raspberry Pi
#
# @Purpose: Builds the scheduler demo.
#
#
################################################################################

# Make all the default target
all::

# Use the regular GCC compiler
CC := gcc

# Define the flags needed at compile time
flags := -ggdb

# Define the libraries needed at link time (in order)
libraries := pthread wiringPi rt

# This creates the '-lFOO -lBAR for each library'
lib_flags := $(addprefix -l, $(libraries))

# Define the output binary
targets := pisched

# Define the required source file
source := sched.c

# Any compile target that is NOT a file should be declared PHONY
.PHONY: all clean

all::  $(targets)

clean::
	$(RM) $(targets)


# Rule to build the pisched executable
pisched: sched.c
	$(CC) $(flags) $< -o $@ $(lib_flags)

# Issuing 'make pisched_d' will enable the debugging output, but this
# target will not be built by 'all'
pisched_d: sched.c
	$(CC) $(flags) -D__DEBUG__ $< -o $@ $(lib_flags)
