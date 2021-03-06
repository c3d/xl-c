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

SOURCES     =				\
	main.c				\
	tree.c				\
	blob.c				\
	text.c				\
	delimited_text.c		\
	name.c				\
	number.c			\
	block.c				\
	pfix.c				\
	infix.c				\
	array.c				\
	position.c			\
	error.c				\
	scanner.c			\
	syntax.c			\
	parser.c			\
	renderer.c			\
	utf8.c				\
	recorder/recorder.c		\
	recorder/recorder_ring.c

PRODUCTS=xl.exe

CONFIG= struct_sigaction		\
	libpthread

INCLUDES=recorder .

MIQ=make-it-quick/
include $(MIQ)rules.mk

.tests: xl_tests
xl_tests:
	cd tests; ./alltests

# Get the rules.mk file if missing
$(MIQ)rules.mk:
	git submodule update --init --recursive
