/*
 * Copyright (c) 2020, Rutgers Discovery Informatics Institute, Rutgers University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided
 * that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this list of conditions and
 * the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 * the following disclaimer in the documentation and/or other materials provided with the distribution.
 * - Neither the name of the NSF Cloud and Autonomic Computing Center, Rutgers University, nor the names of its
 * contributors may be used to endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 *  Pradeep Subedi (2020)  RDI2 Rutgers University
 *  pradeep.subedi@rutgers.edu
 */

#include "MapWrap.hh"
#include "CppWrapper.h"

extern "C" {

	WrapperMap * map_new() {
		MapWrap *t = new MapWrap();
		return (WrapperMap *)t;
	}

	void map_subscribe(const WrapperMap *test, const char *names, const char *filter, const char *subscriber_addr) {
		MapWrap *t = (MapWrap*)test;
		t->mp_insert(names, filter, subscriber_addr);
	}

	void map_unsubscribe(const WrapperMap *test, const char *names, const char *filter, const char *subscriber_addr) {
		MapWrap *t = (MapWrap*)test;
		t->mp_delete(names, filter, subscriber_addr);
	}

	void map_remove(const WrapperMap *test, const char *subscriber_addr) {
		MapWrap *t = (MapWrap*)test;
		t->mp_remove(subscriber_addr);
	}

	vector map_get_value(const WrapperMap *test, const char *names, const char *filter){
		MapWrap *t = (MapWrap*)test;
		return t->get_value(names, filter);
	}

	vector map_get_filters(const WrapperMap *test){
		MapWrap *t = (MapWrap*)test;
		return t->get_filters();
	}

	void map_delete(WrapperMap *test) {
		MapWrap *t = (MapWrap *)test;
		t->delete_all();
		delete t;
	}

	void insert_handler(WrapperMap *test, const char *names, const char *filter, void *func_ptr, void *func_args){
		MapWrap *t = (MapWrap*)test;
		t->insert_pointers(names, filter, func_ptr, func_args);
	}
    void delete_handler(WrapperMap *test, const char *names, const char *filter){
    	MapWrap *t = (MapWrap*)test;
    	t->delete_filter(names, filter);
    }

}