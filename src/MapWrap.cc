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

#include <stdio.h>
#include <string.h>
#include "vector.h"
#include "MapWrap.hh"

MapWrap::MapWrap(){
	std::map <std::string, std::map <std::string, vector> > mymap;
	this->cMap = mymap;
}

void MapWrap::mp_insert(const char *names, const char *filter, const char *subscriber_addr){

	std::string c_names(names);
	std::string c_filter(filter);
	std::map<std::string, vector> inner_map = cMap[c_names];
	std::map<std::string, vector>::iterator it;
	it=inner_map.find(c_filter);
	if(it==inner_map.end()){
		VECTOR_INIT(v);
		VECTOR_ADD(v, subscriber_addr);
		inner_map[c_filter] = v;
	}else{
		//
		vector v;
		v = it->second;
		int flg = 0;
		for (int i = 0; i < VECTOR_TOTAL(v); i++){
			char *curr_subs = VECTOR_GET(v, char*, i);
			if(strcmp(curr_subs, subscriber_addr) == 0){
				flg = 1;
				break;
			}
		}
		if(flg==0){
			VECTOR_ADD(inner_map[c_filter], subscriber_addr);
		}
	}
	cMap[c_names] = inner_map;
	
}


vector MapWrap::get_value(const char *names, const char *filter){

	std::string c_names(names);
	std::string c_filter(filter);
	
	std::map<std::string, vector> inner_map = cMap[c_names];
	VECTOR_INIT(v);
	v = inner_map[c_filter];
	return v;

}

vector MapWrap::get_filters(){

	VECTOR_INIT(v);
	std::map <std::string, std::map<std::string, vector>>::iterator it_out = cMap.begin();
	while (it_out != cMap.end()){
		int i = 0;
		char *out_str = new char[(it_out->first).length()+1];
		strcpy(out_str, (it_out->first).c_str());
		VECTOR_ADD(v, out_str);

		std::map<std::string, vector> inner_map = it_out->second;
		std::map<std::string, vector>::iterator it_in = inner_map.begin();
		while(it_in != inner_map.end()){
			if((i%2) != 0){
				VECTOR_ADD(v, out_str);
				i = 0;
			}
			char *in_str = new char[(it_in->first).length()+1];
			strcpy(in_str, (it_in->first).c_str());
			VECTOR_ADD(v, in_str);
			i++;
			it_in++;

		}
		if(i==0)
			VECTOR_DELETE(v, VECTOR_TOTAL(v)-1);
		
		it_out++;
	}
	return v;

}

void MapWrap::mp_delete(const char *names, const char *filter, const char *subscriber_addr){

	std::string c_names(names);
	std::string c_filter(filter);

	
	std::map<std::string, vector> inner_map = cMap[c_names];
	std::map<std::string, vector>::iterator it;
	it=inner_map.find(c_filter);
	vector v;
	if(it!=inner_map.end()){
		v = it->second;
		int del_id =0;
		for (int i = 0; i < VECTOR_TOTAL(v); i++){
			char *curr_subs = VECTOR_GET(v, char*, i);
			if(strcmp(curr_subs, subscriber_addr) == 0){
				del_id = i;
				break;
			}
		}
		VECTOR_DELETE(v, del_id);
		inner_map[c_filter] = v;
		cMap[c_names] = inner_map;
	}

	
}

void MapWrap::mp_remove(const char *subscriber_addr){
	std::map <std::string, std::map<std::string, vector>>::iterator it_out = cMap.begin();
	while (it_out != cMap.end()){
		std::map<std::string, vector> inner_map = it_out->second;
		std::map<std::string, vector>::iterator it_in = inner_map.begin();
		while(it_in != inner_map.end()){
			vector v;
			v = it_in->second;
			int del_id =0;
			for (int i = 0; i < VECTOR_TOTAL(v); i++){
				char *curr_subs = VECTOR_GET(v, char*, i);
				if(strcmp(curr_subs, subscriber_addr) == 0){
					del_id = i;
					break;
				}
			}
			VECTOR_DELETE(v, del_id);
			inner_map[it_in->first] = v;
			it_in++;
		}
		cMap[it_out->first] = inner_map;
		it_out++;
	}
	
}

void MapWrap::delete_all(){
	std::map <std::string, std::map<std::string, vector>>::iterator it_out = cMap.begin();
	while (it_out != cMap.end()){
		std::map<std::string, vector>::iterator it_in = it_out->second.begin();
		while(it_in != it_out->second.end()){
			VECTOR_FREE(it_in->second);
			it_in++;
		}
		it_out++;
	}
	
}


void MapWrap::insert_pointers(const char *names, const char *filter, void *func_ptr, void *func_args){

	std::string c_names(names);
	std::string c_filter(filter);
	std::map<std::string, vector> inner_map = cMap[c_names];
	VECTOR_INIT(v);
	VECTOR_ADD(v, func_ptr);
	VECTOR_ADD(v, func_args);
	inner_map[c_filter] = v;
	cMap[c_names] = inner_map;
	
}

void MapWrap::delete_filter(const char *names, const char *filter){
	std::string c_names(names);
	std::string c_filter(filter);

	std::map<std::string, vector> inner_map = cMap[c_names];
	inner_map.erase(c_filter);
	cMap[c_names] = inner_map;
	
}


