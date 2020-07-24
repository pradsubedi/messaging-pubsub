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

#include "vector.h"

typedef void WrapperMap;

#ifdef __cplusplus
extern "C" {
#endif
	WrapperMap * map_new();
	void map_subscribe( const WrapperMap *t, const char *names, const char *filter, const char *subscriber_addr);
	vector map_get_value(const WrapperMap *t, const char *names, const char *filter);
	void map_unsubscribe(const WrapperMap *t, const char *names, const char *filter, const char *subscriber_addr);
	void map_remove(const WrapperMap *t, const char *subscriber_addr);
	void map_delete(WrapperMap *t);
	void insert_handler(WrapperMap *test, const char *names, const char *filter, void *func_ptr,  void *func_args);
	void delete_handler(WrapperMap *test, const char *names, const char *filter);

#ifdef __cplusplus
}
#endif