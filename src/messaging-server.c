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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include<assert.h>
#include <messaging-server.h>
#include <CppWrapper.h>
#include <vector.h>

struct messaging_server{
    margo_instance_id mid;
    hg_id_t pub_id;
    hg_id_t sub_id;
    hg_id_t unsub_id;
    hg_id_t notify_id;
    hg_id_t finalize_id;
    WrapperMap *t;
};

DECLARE_MARGO_RPC_HANDLER(publish_rpc);
DECLARE_MARGO_RPC_HANDLER(subscribe_rpc);
DECLARE_MARGO_RPC_HANDLER(unsubscribe_rpc);
DECLARE_MARGO_RPC_HANDLER(client_finalize_rpc);

static void publish_rpc(hg_handle_t h);
static void subscribe_rpc(hg_handle_t h);
static void unsubscribe_rpc(hg_handle_t h);
static void client_finalize_rpc(hg_handle_t h);

static int write_address(messaging_server_t server, MPI_Comm comm){

    hg_addr_t my_addr  = HG_ADDR_NULL;
    hg_return_t hret   = HG_SUCCESS;
    hg_size_t my_addr_size;

    int comm_size, rank, ret = 0;
    MPI_Comm_size(comm, &comm_size);
    MPI_Comm_rank(comm, &rank);

    char *my_addr_str = NULL;
    int self_addr_str_size = 0;
    char *addr_str_buf = NULL;
    int *sizes = NULL;
    int *sizes_psum = NULL;
    char **addr_strs = NULL;

    hret = margo_addr_self(server->mid, &my_addr);
    if(hret != HG_SUCCESS) {
        fprintf(stderr, "ERROR: margo_addr_self() returned %d\n", hret);
        ret = -1;
        goto error;
    }

    hret = margo_addr_to_string(server->mid, NULL, &my_addr_size, my_addr);
    if(hret != HG_SUCCESS) {
        fprintf(stderr, "ERROR: margo_addr_to_string() returned %d\n", hret);
        ret = -1;
        goto errorfree;
    }

    my_addr_str = malloc(my_addr_size);
    hret = margo_addr_to_string(server->mid, my_addr_str, &my_addr_size, my_addr);
    if(hret != HG_SUCCESS) {
        fprintf(stderr, "ERROR: margo_addr_to_string() returned %d\n", hret);
        ret = -1;
        goto errorfree;
    }
    fprintf(stdout,"Server running at %s\n", my_addr_str);

    sizes = malloc(comm_size * sizeof(*sizes));
    self_addr_str_size = (int)strlen(my_addr_str) + 1;
    MPI_Allgather(&self_addr_str_size, 1, MPI_INT, sizes, 1, MPI_INT, comm);

    int addr_buf_size = 0;
    for (int i = 0; i < comm_size; ++i)
    {
        addr_buf_size = addr_buf_size + sizes[i];
    }

    sizes_psum = malloc((comm_size) * sizeof(*sizes_psum));
    sizes_psum[0]=0;
    for (int i = 1; i < comm_size; i++)
        sizes_psum[i] = sizes_psum[i-1] + sizes[i-1];

    addr_str_buf = malloc(addr_buf_size);
    MPI_Gatherv(my_addr_str, self_addr_str_size, MPI_CHAR, addr_str_buf, sizes, sizes_psum, MPI_CHAR, 0, comm);
    if(rank==0){
        
        for (int i = 1; i < comm_size; ++i)
        {
            addr_str_buf[sizes_psum[i]-1]='\n';
        }
        addr_str_buf[addr_buf_size-1]='\n';

        int fd;
        fd = open("servids.0", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0)
        {
            fprintf(stderr, "ERROR: unable to write server_ids into file\n");
            ret = -1;
            goto errorfree;

        }
        int bytes_written = 0;
        bytes_written = write(fd, addr_str_buf, addr_buf_size);
        if (bytes_written != addr_buf_size)
        {
            fprintf(stderr, "ERROR: unable to write server_ids into opened file\n");
            ret = -1;
            free(addr_str_buf);
            close(fd);
            goto errorfree;

        }
        close(fd);
    }
//    margo_addr_free(server->mid, my_addr);
    free(my_addr_str);
    free(sizes);
    free(sizes_psum);
    free(addr_str_buf);

finish:
    return ret;
errorfree:
    margo_addr_free(server->mid, my_addr);
error:
    margo_finalize(server->mid);
    goto finish;
}


int server_init(margo_instance_id mid, MPI_Comm comm, messaging_server_t* sv)
{
    
    messaging_server_t server = (messaging_server_t)calloc(1, sizeof(*server));
    if(!server) return MESSAGING_ERR_ALLOCATION;

    int ret = 0;

    hg_return_t hret  = HG_SUCCESS;
    server->mid = mid;

    ret = write_address(server, comm);
    if(ret!=0)
        goto finish;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(server->mid, "publish_rpc", &id, &flag);

    if(flag == HG_TRUE) { /* RPCs already registered */
        margo_registered_name(mid, "publish_rpc",                   &server->pub_id,                   &flag);
        margo_registered_name(mid, "subscribe_rpc",                   &server->sub_id,                   &flag);
        margo_registered_name(mid, "unsubscribe_rpc",                   &server->unsub_id,                   &flag);
        margo_registered_name(mid, "notify_rpc",                   &server->notify_id,                   &flag);
        margo_registered_name(mid, "client_finalize_rpc",                   &server->finalize_id,                   &flag);
   
    } else {

        server->pub_id =
            MARGO_REGISTER(mid, "publish_rpc", bulk_data_t, response_t, publish_rpc);
        margo_register_data(mid, server->pub_id, (void*)server, NULL);
        server->sub_id =
            MARGO_REGISTER(mid, "subscribe_rpc", bulk_data_t, response_t, subscribe_rpc);
        margo_register_data(mid, server->sub_id, (void*)server, NULL);
        server->unsub_id =
            MARGO_REGISTER(mid, "unsubscribe_rpc", bulk_data_t, response_t, unsubscribe_rpc);
        margo_register_data(mid, server->unsub_id, (void*)server, NULL);
        server->notify_id =
            MARGO_REGISTER(mid, "notify_rpc", bulk_data_t, response_t, NULL);
        server->finalize_id =
            MARGO_REGISTER(mid, "client_finalize_rpc", bulk_data_t, response_t, client_finalize_rpc);
        margo_register_data(mid, server->finalize_id, (void*)server, NULL);

    }
    server->t=map_new();
    *sv = server;

    return MESSAGING_SUCCESS;
finish:
    return ret;
}

int server_destroy(messaging_server_t server){
    margo_instance_id mid = server->mid;

    margo_deregister(mid, server->pub_id);
    margo_deregister(mid, server->sub_id);
    margo_deregister(mid, server->unsub_id);
    /* deregister other RPC ids ... */
    map_delete(server->t);
    server->t = NULL;
    free(server);
}

static void publish_rpc(hg_handle_t hndl)
{
    hg_return_t ret;

    bulk_data_t in;
    response_t out;

    margo_instance_id mid = margo_hg_handle_get_instance(hndl);

    const struct hg_info* info = margo_get_info(hndl);
    messaging_server_t server = (messaging_server_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(hndl, &in);
    assert(ret == HG_SUCCESS);

    int i;

    int namespace_len, topic_len, tag_len;
    char *namesp, *topic,  *tag_msg, *raw_buf;
        
    raw_buf = (char*) malloc(in.evnt.size);
    memcpy(raw_buf, in.evnt.raw_data, in.evnt.size);
    
    namespace_len = ((int *)raw_buf)[0];
    topic_len = ((int *)raw_buf)[1];
  
    namesp = (char *)malloc(namespace_len);
    topic = (char *)malloc(topic_len);
    memcpy(namesp, &raw_buf[sizeof(int)*3], namespace_len);
    memcpy(topic, &raw_buf[sizeof(int)*3+namespace_len], topic_len);

    vector sub_list;
    sub_list = map_get_value(server->t,  namesp, topic);
    free(namesp);
    free(topic);

    out.ret = MESSAGING_SUCCESS;
    ret = margo_respond(hndl, &out);
    assert(ret == HG_SUCCESS);

    int total_subscribers = VECTOR_TOTAL(sub_list);

    //now notify to all clients
    margo_request *serv_req;
    hg_handle_t *notify_hndl;
    notify_hndl = (hg_handle_t*)malloc(sizeof(hg_handle_t)*total_subscribers);
    serv_req = (margo_request*)malloc(sizeof(margo_request)*total_subscribers);
     
    //notify
    for (int i = 0; i < total_subscribers; ++i)
    {
        char* client_addr;    
        client_addr = VECTOR_GET(sub_list, char*, i);
        fprintf(stdout, "Sending notification to client %s\n", client_addr);

        hg_addr_t cl_addr;
        margo_addr_lookup(server->mid, client_addr, &cl_addr);

        hg_handle_t h;
        margo_create(server->mid, cl_addr, server->notify_id, &h);

        bulk_data_t notify_in;
        notify_in.evnt.size = in.evnt.size;
        notify_in.evnt.raw_data = raw_buf;
        margo_request req;
        //forward notification async to all subscribers
        margo_iforward(h, &notify_in, &req); 
        notify_hndl[i] = h;
        serv_req[i] = req;

    }
    for (i = 0; i < total_subscribers; ++i){
        margo_wait(serv_req[i]);
        response_t resp;
        margo_get_output(notify_hndl[i], &resp);
        ret = resp.ret;
        margo_free_output(notify_hndl[i], &resp);
        margo_destroy(notify_hndl[i]);
        if(ret!=MESSAGING_SUCCESS){
            fprintf(stderr, "Could not notify client %s \n", VECTOR_GET(sub_list, char*, i));
            //return ret;
        }
        
    }
    free(raw_buf);
    margo_free_input(hndl, &in);
    margo_destroy(hndl);

}
DEFINE_MARGO_RPC_HANDLER(publish_rpc)

static void subscribe_rpc(hg_handle_t hndl)
{
    hg_return_t ret;

    bulk_data_t in;
    response_t out;

    margo_instance_id mid = margo_hg_handle_get_instance(hndl);

    const struct hg_info* info = margo_get_info(hndl);
    messaging_server_t server = (messaging_server_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(hndl, &in);
    assert(ret == HG_SUCCESS);

    char *namesp, *topic, *subs_addr, *raw_buf;
    int namespace_len, topic_len, subs_addr_size;

    raw_buf = (char*)in.evnt.raw_data;

    namespace_len = ((int *)raw_buf)[0];
    topic_len = ((int *)raw_buf)[1];
    subs_addr_size = ((int *)raw_buf)[2];

    namesp = malloc(namespace_len);
    topic = malloc(topic_len);
    subs_addr = malloc(subs_addr_size);

    
    memcpy(namesp, &raw_buf[sizeof(int)*3], namespace_len);
    memcpy(topic, &raw_buf[sizeof(int)*3+namespace_len], topic_len);
    memcpy(subs_addr, &raw_buf[sizeof(int)*3+namespace_len+topic_len], subs_addr_size);
    map_subscribe(server->t, namesp, topic, subs_addr);

    out.ret = MESSAGING_SUCCESS;
    ret = margo_respond(hndl, &out);
    assert(ret == HG_SUCCESS);

    free(namesp);
    free(topic);
    margo_free_input(hndl, &in);
    margo_destroy(hndl);
 
}
DEFINE_MARGO_RPC_HANDLER(subscribe_rpc)

static void unsubscribe_rpc(hg_handle_t hndl)
{
    hg_return_t ret;

    bulk_data_t in;
    response_t out;

    margo_instance_id mid = margo_hg_handle_get_instance(hndl);

    const struct hg_info* info = margo_get_info(hndl);
    messaging_server_t server = (messaging_server_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(hndl, &in);
    assert(ret == HG_SUCCESS);

    char *namesp, *topic, *subs_addr, *raw_buf;
    int namespace_len, topic_len, subs_addr_size;

    raw_buf = (char*)in.evnt.raw_data;

    namespace_len = ((int *)raw_buf)[0];
    topic_len = ((int *)raw_buf)[1];
    subs_addr_size = ((int *)raw_buf)[2];

    namesp = malloc(namespace_len);
    topic = malloc(topic_len);
    subs_addr = malloc(subs_addr_size);

    
    memcpy(namesp, &raw_buf[sizeof(int)*3], namespace_len);
    memcpy(topic, &raw_buf[sizeof(int)*3+namespace_len], topic_len);
    memcpy(subs_addr, &raw_buf[sizeof(int)*3+namespace_len+topic_len], subs_addr_size);
    map_unsubscribe(server->t, namesp, topic, subs_addr);

    out.ret = MESSAGING_SUCCESS;
    ret = margo_respond(hndl, &out);
    assert(ret == HG_SUCCESS);

    free(namesp);
    free(topic);
    free(subs_addr);
    margo_free_input(hndl, &in);
    margo_destroy(hndl);
}
DEFINE_MARGO_RPC_HANDLER(unsubscribe_rpc)

static void client_finalize_rpc(hg_handle_t hndl)
{
    hg_return_t ret;

    bulk_data_t in;
    response_t out;

    margo_instance_id mid = margo_hg_handle_get_instance(hndl);

    const struct hg_info* info = margo_get_info(hndl);
    messaging_server_t server = (messaging_server_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(hndl, &in);
    assert(ret == HG_SUCCESS);

    char *raw_buf;
    raw_buf = (char*)in.evnt.raw_data;

    map_remove(server->t, raw_buf);
    out.ret = MESSAGING_SUCCESS;
    ret = margo_respond(hndl, &out);
    assert(ret == HG_SUCCESS);

    margo_free_input(hndl, &in);
    margo_destroy(hndl);
}
DEFINE_MARGO_RPC_HANDLER(client_finalize_rpc)

