# Copyright (c) 2021 Syntiant Corp.  All rights reserved.
# Contact at http://www.syntiant.com
# 
# This software is available to you under a choice of one of two licenses.
# You may choose to be licensed under the terms of the GNU General Public
# License (GPL) Version 2, available from the file LICENSE in the main
# directory of this source tree, or the OpenIB.org BSD license below.  Any
# code involving Linux software will require selection of the GNU General
# Public License (GPL) Version 2.
# 
# OPENIB.ORG BSD LICENSE
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

CFLAGS= -g3 -O3 -Wall -Werror -Wc++-compat -fpic -I. -D_FILE_OFFSET_BITS=64 
		

ILIB_LIBRARY=synilib
ILIB_STATIC_LIBRARY=lib$(ILIB_LIBRARY).a
ILIB_OBJS := syntiant_ndp120_tiny.o syntiant_tiny_cspi.o

$(ILIB_STATIC_LIBRARY): $(ILIB_OBJS)
	$(AR) rcs $@ $^

all: $(ILIB_STATIC_LIBRARY)  

-include $(OBJS:.o=.d)

%.o: %.c
	$(CC) -MM -MT $*.o $(CFLAGS) $(CPPFLAGS) $*.c > $*.d
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $*.c -o $*.o

clean:
	$(RM) -f $(ILIB_STATIC_LIBRARY) \
		$(ILIB_OBJS) *.d 
