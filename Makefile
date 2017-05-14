#******************************************************************************
# Makefile<xl>                                      XL - An extensible language
#******************************************************************************
#
#  File Description:
#
#    Makefile for the C implementation of the XL compiler
#
#
#
#
#
#
#
#
#******************************************************************************
# (C) 2017 Christophe de Dinechin <christophe@dinechin.org>
#  This software is licensed under the GNU General Public License v3
#  See LICENSE file for details.
#******************************************************************************

SOURCES     =		\
	main.c		\
	tree.c		\
	blob.c		\
	text.c		\
	name.c		\
	number.c	\
	block.c		\
	pfix.c		\
	infix.c

PRODUCTS=xl.exe

BUILD=build/
include $(BUILD)rules.mk

test: xl_tests
xl_tests:
	cd tests; ./alltests

# Get the rules.mk file if missing
$(BUILD)rules.mk:
	git submodule update --init --recursive
