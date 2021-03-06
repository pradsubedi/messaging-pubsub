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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <messaging-client.h>
#include <CppWrapper.h>
#include <vector.h>


struct messaging_client {
    margo_instance_id mid;
    hg_id_t pub_id;
    hg_id_t sub_id;
    hg_id_t unsub_id;
    hg_id_t notify_id;
    hg_id_t finalize_id;
    char **server_address;
    int num_servers;
    //MPI_Comm comm;
    char *addr_string;
    int addr_string_len;
    WrapperMap *t;
};

DECLARE_MARGO_RPC_HANDLER(notify_rpc);

static void notify_rpc(hg_handle_t h);
static int remove_all_subscriptions(messaging_client_t client);
static int remove_all_subscriptions_new(messaging_client_t client);

unsigned long hash(char *str)
    {
        unsigned long hash = 5381;
        int c;

        while (c = *str++)
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

        return hash;
    }

char ** addr_str_buf_to_list(
    char * buf, int num_addrs)
{
    int i;
    char **ret = malloc(num_addrs * sizeof(*ret));
    if (ret == NULL) return NULL;

    ret[0] = (char *)buf;
    for (i = 1; i < num_addrs; i++)
    {
        char * a = ret[i-1];
        ret[i] = a + strlen(a) + 1;
    }
    return ret;
}


static int build_address_with_mpi(messaging_client_t* cl, MPI_Comm comm){
    /* open config file for reading */
    int ret;
    struct stat st;
    char *rd_buf = NULL;
    ssize_t rd_buf_size;
    char *tok;
    void *addr_str_buf = NULL;
    int addr_str_buf_len = 0, num_addrs = 0;
    int fd;

    messaging_client_t client;
    client = *cl;
    int comm_size, rank;
    MPI_Comm_size(comm, &comm_size);
    MPI_Comm_rank(comm, &rank);

    char* file_name = "servids.0";
    if(rank==0){
        fd = open(file_name, O_RDONLY);
        if (fd == -1)
        {
            fprintf(stderr, "Error: Unable to open config file %s for server_address list\n",
                file_name);
            goto fini;
        }

        /* get file size and allocate a buffer to store it */
        ret = fstat(fd, &st);
        if (ret == -1)
        {
            fprintf(stderr, "Error: Unable to stat config file %s for server_address list\n",
                file_name);
            goto fini;
        }
        ret = -1;
        rd_buf = malloc(st.st_size);
        if (rd_buf == NULL) goto fini;

        /* load it all in one fell swoop */
        rd_buf_size = read(fd, rd_buf, st.st_size);
        if (rd_buf_size != st.st_size)
        {
            fprintf(stderr, "Error: Unable to stat config file %s for server_address list\n",
                file_name);
            goto fini;
        }
        rd_buf[rd_buf_size]='\0';

        // strtok the result - each space-delimited address is assumed to be
        // a unique mercury address

        tok = strtok(rd_buf, "\r\n\t ");
        if (tok == NULL) goto fini;

        // build up the address buffer
        addr_str_buf = malloc(rd_buf_size);
        if (addr_str_buf == NULL) goto fini;
        do
        {
            int tok_size = strlen(tok);
            memcpy((char*)addr_str_buf + addr_str_buf_len, tok, tok_size+1);
            addr_str_buf_len += tok_size+1;
            num_addrs++;
            tok = strtok(NULL, "\r\n\t ");
        } while (tok != NULL);
        if (addr_str_buf_len != rd_buf_size)
        {
            // adjust buffer size if our initial guess was wrong
            fprintf(stderr, "Read size and buffer_len are not equal\n");
            void *tmp = realloc(addr_str_buf, addr_str_buf_len);
            if (tmp == NULL) goto fini;
            addr_str_buf = tmp;
        }
        free(rd_buf);
    }

    MPI_Barrier(comm);
    // Broadcasting buffer_len and buffer to all client ranks
    MPI_Bcast(&addr_str_buf_len, 1, MPI_INT, 0, comm);
    MPI_Bcast(&num_addrs, 1, MPI_INT, 0, comm);
    if(rank!=0)
        addr_str_buf = malloc(addr_str_buf_len);
    MPI_Bcast(addr_str_buf, addr_str_buf_len, MPI_CHAR, 0, comm);
    
    /* set up address string array for group members */
    client->server_address = (char **)addr_str_buf_to_list(addr_str_buf, num_addrs);
    client->num_servers = num_addrs;
    *cl = client;
    ret = 0;

fini:
    return ret;
}

static int build_address(messaging_client_t* cl){
    /* open config file for reading */
    int ret;
    struct stat st;
    char *rd_buf = NULL;
    ssize_t rd_buf_size;
    char *tok;
    void *addr_str_buf = NULL;
    int addr_str_buf_len = 0, num_addrs = 0;
    int fd;

    messaging_client_t client;
    client = *cl;

    char* file_name = "servids.0";
    fd = open(file_name, O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Error: Unable to open config file %s for server_address list\n",
            file_name);
        goto fini;
    }

    /* get file size and allocate a buffer to store it */
    ret = fstat(fd, &st);
    if (ret == -1)
    {
        fprintf(stderr, "Error: Unable to stat config file %s for server_address list\n",
            file_name);
        goto fini;
    }
    ret = -1;
    rd_buf = malloc(st.st_size);
    if (rd_buf == NULL) goto fini;

    /* load it all in one fell swoop */
    rd_buf_size = read(fd, rd_buf, st.st_size);
    if (rd_buf_size != st.st_size)
    {
        fprintf(stderr, "Error: Unable to stat config file %s for server_address list\n",
            file_name);
        goto fini;
    }
    rd_buf[rd_buf_size]='\0';

    // strtok the result - each space-delimited address is assumed to be
    // a unique mercury address

    tok = strtok(rd_buf, "\r\n\t ");
    if (tok == NULL) goto fini;

    // build up the address buffer
    addr_str_buf = malloc(rd_buf_size);
    if (addr_str_buf == NULL) goto fini;
    do
    {
        int tok_size = strlen(tok);
        memcpy((char*)addr_str_buf + addr_str_buf_len, tok, tok_size+1);
        addr_str_buf_len += tok_size+1;
        num_addrs++;
        tok = strtok(NULL, "\r\n\t ");
    } while (tok != NULL);
    if (addr_str_buf_len != rd_buf_size)
    {
        void *tmp = realloc(addr_str_buf, addr_str_buf_len);
        if (tmp == NULL) goto fini;
        addr_str_buf = tmp;
    }
    free(rd_buf);
   
    /* set up address string array for group members */
    client->server_address = (char **)addr_str_buf_to_list(addr_str_buf, num_addrs);
    client->num_servers = num_addrs;
    *cl = client;
    ret = 0;

fini:
    return ret;
}

int client_init_with_mpi(margo_instance_id mid, MPI_Comm comm, messaging_client_t* cl)
{
    
    messaging_client_t client  = (messaging_client_t)calloc(1, sizeof(*client));
    if(!client) return MESSAGING_ERR_ALLOCATION;

    int ret = 0;

    hg_return_t hret          = HG_SUCCESS;
    client->mid = mid;

    ret = build_address_with_mpi(&client, comm);
    if(ret!=0)
        goto finish;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "publish_rpc", &id, &flag);

    if(flag == HG_TRUE) { /* RPCs already registered */
        margo_registered_name(mid, "publish_rpc",                   &client->pub_id,                   &flag);
        margo_registered_name(mid, "subscribe_rpc",                   &client->sub_id,                   &flag);
        margo_registered_name(mid, "unsubscribe_rpc",                   &client->unsub_id,                   &flag);
        margo_registered_name(mid, "client_finalize_rpc",                   &client->finalize_id,                   &flag);
        margo_registered_name(mid, "notify_rpc",                   &client->notify_id,                   &flag);
   
    } else {

        client->pub_id =
            MARGO_REGISTER(mid, "publish_rpc", bulk_data_t, response_t, NULL);
        client->sub_id =
            MARGO_REGISTER(mid, "subscribe_rpc", bulk_data_t, response_t, NULL);
        client->unsub_id =
            MARGO_REGISTER(mid, "unsubscribe_rpc", bulk_data_t, response_t, NULL);
        client->finalize_id =
            MARGO_REGISTER(mid, "client_finalize_rpc", bulk_data_t, response_t, NULL);
        client->notify_id =
            MARGO_REGISTER(mid, "notify_rpc", bulk_data_t, response_t, notify_rpc);
        margo_register_data(mid, client->notify_id, (void*)client, NULL);
    }
    
    hg_addr_t my_addr  = HG_ADDR_NULL;
    hg_size_t my_addr_size;
    char *my_addr_str = NULL;
    margo_addr_self(client->mid, &my_addr);
    margo_addr_to_string(client->mid, NULL, &my_addr_size, my_addr);
    my_addr_str = malloc(my_addr_size);
    margo_addr_to_string(client->mid, my_addr_str, &my_addr_size, my_addr);
    margo_addr_free(client->mid, my_addr);

    client->addr_string = my_addr_str;
    client->addr_string_len = my_addr_size;
    client->t = map_new();

    *cl = client;

    return MESSAGING_SUCCESS;
finish:
    return ret;
}

int client_init(margo_instance_id mid, messaging_client_t* cl)
{
    
    messaging_client_t client  = (messaging_client_t)calloc(1, sizeof(*client));
    if(!client) return MESSAGING_ERR_ALLOCATION;

    int ret = 0;

    hg_return_t hret          = HG_SUCCESS;
    client->mid = mid;

    ret = build_address(&client);
    if(ret!=0)
        goto finish;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "publish_rpc", &id, &flag);

    if(flag == HG_TRUE) { /* RPCs already registered */
        margo_registered_name(mid, "publish_rpc",                   &client->pub_id,                   &flag);
        margo_registered_name(mid, "subscribe_rpc",                   &client->sub_id,                   &flag);
        margo_registered_name(mid, "unsubscribe_rpc",                   &client->unsub_id,                   &flag);
        margo_registered_name(mid, "client_finalize_rpc",                   &client->finalize_id,                   &flag);
        margo_registered_name(mid, "notify_rpc",                   &client->notify_id,                   &flag);
   
    } else {

        client->pub_id =
            MARGO_REGISTER(mid, "publish_rpc", bulk_data_t, response_t, NULL);
        client->sub_id =
            MARGO_REGISTER(mid, "subscribe_rpc", bulk_data_t, response_t, NULL);
        client->unsub_id =
            MARGO_REGISTER(mid, "unsubscribe_rpc", bulk_data_t, response_t, NULL);
        client->finalize_id =
            MARGO_REGISTER(mid, "client_finalize_rpc", bulk_data_t, response_t, NULL);
        client->notify_id =
            MARGO_REGISTER(mid, "notify_rpc", bulk_data_t, response_t, notify_rpc);
        margo_register_data(mid, client->notify_id, (void*)client, NULL);
    }
    
    hg_addr_t my_addr  = HG_ADDR_NULL;
    hg_size_t my_addr_size;
    char *my_addr_str = NULL;
    margo_addr_self(client->mid, &my_addr);
    margo_addr_to_string(client->mid, NULL, &my_addr_size, my_addr);
    my_addr_str = malloc(my_addr_size);
    margo_addr_to_string(client->mid, my_addr_str, &my_addr_size, my_addr);
    margo_addr_free(client->mid, my_addr);
    client->addr_string = my_addr_str;
    client->addr_string_len = my_addr_size;
    client->t = map_new();

    *cl = client;

    return MESSAGING_SUCCESS;
finish:
    return ret;
}

int client_finalize(messaging_client_t client){

    //remove_all_subscriptions(client);
    remove_all_subscriptions_new(client);
    margo_deregister(client->mid, client->notify_id);
    map_delete(client->t);
    free(client->addr_string);
    free(client->server_address[0]);
    free(client->server_address);
    //margo_finalize(client->mid);
    free(client);
    return MESSAGING_SUCCESS;

}

int publish(messaging_client_t client, char *namesp, char* topic, void* messg, int msg_len){
    
    int server_id= hash(topic) % client->num_servers;
    
    char* raw_buf;
    
    int name_len, topic_len;
    int ret = 0;

    name_len = strlen(namesp)+1;
    topic_len = strlen(topic)+1;

    bulk_data_t raw_msg;
    raw_msg.evnt.size = sizeof(int)*3 + name_len + topic_len + msg_len;
    raw_buf = malloc(raw_msg.evnt.size);

    ((int *)raw_buf)[0] = name_len;
    ((int *)raw_buf)[1] = topic_len;
    ((int *)raw_buf)[2] = msg_len;

    memcpy(&raw_buf[sizeof(int)*3], namesp, name_len);
    memcpy(&raw_buf[sizeof(int)*3+name_len], topic, topic_len);
    memcpy(&raw_buf[sizeof(int)*3+name_len+topic_len], messg, msg_len);

    raw_msg.evnt.raw_data = raw_buf;

    hg_addr_t svr_addr;
    margo_addr_lookup(client->mid, client->server_address[server_id], &svr_addr);

    hg_handle_t h;
    margo_create(client->mid, svr_addr, client->pub_id, &h);
    margo_forward(h, &raw_msg);
    //margo_request req;
    //margo_iforward(h, &raw_msg, &req);
    //margo_wait(req);
    response_t resp;
    margo_get_output(h, &resp);
    if(resp.ret != MESSAGING_SUCCESS)
        fprintf(stderr, "Publish message got bad response. Publish failed\n");
    
    ret = resp.ret;
    margo_addr_free(client->mid, svr_addr);
    margo_free_output(h, &resp);
    margo_destroy(h);
    return ret;


}

int subscribe(messaging_client_t client, char *namesp, char* topic, void (*handler_func)(void *, void*), void *handler_args){

    int ret=0;
    int server_id= hash(topic) % client->num_servers;

    int name_len, topic_len;

    name_len = strlen(namesp)+1;
    topic_len = strlen(topic)+1;

    bulk_data_t raw_msg;
    raw_msg.evnt.size = sizeof(int)*3 + name_len + topic_len + client->addr_string_len;

    char *raw_buf;
    raw_buf = malloc(raw_msg.evnt.size);
    
    ((int *)raw_buf)[0] = name_len;
    ((int *)raw_buf)[1] = topic_len;
    ((int *)raw_buf)[2] = client->addr_string_len;

    memcpy(&raw_buf[sizeof(int)*3], namesp, name_len);
    memcpy(&raw_buf[sizeof(int)*3+name_len], topic, topic_len);
    memcpy(&raw_buf[sizeof(int)*3+name_len+topic_len], client->addr_string, client->addr_string_len);
    
    raw_msg.evnt.raw_data = raw_buf;

    hg_addr_t svr_addr;
    margo_addr_lookup(client->mid, client->server_address[server_id], &svr_addr);
    hg_handle_t h;
    margo_create(client->mid, svr_addr, client->sub_id, &h);
    margo_forward(h, &raw_msg);
    //margo_request req;
    //margo_iforward(h, &raw_msg, &req);
    //margo_wait(req);
    response_t resp;
    margo_get_output(h, &resp);

    if(resp.ret != MESSAGING_SUCCESS)
        fprintf(stderr, "subscribe message got bad response. subscribe failed\n");
    
    ret = resp.ret;
    insert_handler(client->t, namesp, topic, handler_func, handler_args);
    margo_addr_free(client->mid, svr_addr);
    margo_free_output(h, &resp);
    margo_destroy(h);
    return ret;


}

int unsubscribe(messaging_client_t client, char *namesp, char *topic){

    int server_id= hash(topic) % client->num_servers;
    int ret = 0;

    int name_len, topic_len;
    name_len = strlen(namesp);
    topic_len = strlen(topic);

    bulk_data_t raw_msg;
    raw_msg.evnt.size = sizeof(int)*3 + name_len + topic_len + client->addr_string_len;

    char* raw_buf;
    raw_buf = malloc(raw_msg.evnt.size);
    
    ((int *)raw_buf)[0] = name_len;
    ((int *)raw_buf)[1] = topic_len;
    ((int *)raw_buf)[2] = client->addr_string_len;

    memcpy(&raw_buf[sizeof(int)*3], namesp, name_len);
    memcpy(&raw_buf[sizeof(int)*3+name_len], topic, topic_len);
    memcpy(&raw_buf[sizeof(int)*3+name_len+topic_len], client->addr_string, client->addr_string_len);

    
    raw_msg.evnt.raw_data = raw_buf;

    hg_addr_t svr_addr;
    hg_handle_t h;
    response_t resp;
    margo_addr_lookup(client->mid, client->server_address[server_id], &svr_addr);
    margo_create(client->mid, svr_addr, client->unsub_id, &h);
    margo_forward(h, &raw_msg);
    margo_get_output(h, &resp);

    if(resp.ret != MESSAGING_SUCCESS)
        fprintf(stderr, "Unubscribe message got bad response. Unsubscribe failed\n");
    
    ret = resp.ret;
    delete_handler(client->t, namesp, topic);
    margo_addr_free(client->mid, svr_addr);
    margo_free_output(h, &resp);
    margo_destroy(h);
    return ret;


}

static int remove_all_subscriptions(messaging_client_t client){
    int i, ret;
    char *my_addr_str;
    bulk_data_t in;

    in.evnt.size = client->addr_string_len;
    in.evnt.raw_data = client->addr_string;

    margo_request *serv_req;
    hg_handle_t *hndl;
    hndl = (hg_handle_t*)malloc(sizeof(hg_handle_t)*client->num_servers);
    serv_req = (margo_request*)malloc(sizeof(margo_request)*client->num_servers);
    
    for (i = 0; i < client->num_servers; ++i)
    {
        hg_addr_t svr_addr;
        margo_request req;
        hg_handle_t h;
        margo_addr_lookup(client->mid, client->server_address[i], &svr_addr);
        margo_create(client->mid, svr_addr, client->finalize_id, &h);
        margo_iforward(h, &in, &req);
        hndl[i] = h;
        serv_req[i] = req;
        margo_addr_free(client->mid, svr_addr);
    }
    for (i = 0; i < client->num_servers; ++i){
        margo_wait(serv_req[i]);
        response_t resp;
        margo_get_output(hndl[i], &resp);
        ret = resp.ret;
        margo_free_output(hndl[i], &resp);
        margo_destroy(hndl[i]);
        if(ret!=MESSAGING_SUCCESS){
            fprintf(stderr, "Could not unregister client %s from server %s\n", client->addr_string, client->server_address[i]);
            return ret;
        }
        
    }
    free(hndl);
    free(serv_req);
    return ret;

}

static int remove_all_subscriptions_new(messaging_client_t client){
    int ret;
    char *my_addr_str;
    bulk_data_t in;

    in.evnt.size = client->addr_string_len;
    in.evnt.raw_data = client->addr_string;

    margo_request *serv_req;
    hg_handle_t *hndl;
    vector v;
    int *arr;
    v = map_get_topics(client->t);
    int serv_size = VECTOR_TOTAL(v)/2;
    arr = (int*)malloc(sizeof(int)*serv_size);
    //get server ids in an array
    for (int i = 0; i < VECTOR_TOTAL(v); ++i)
    {
        int j = 0;
        if((i%2)==0)
            continue;
        arr[j] = hash(VECTOR_GET(v, char*, i)) % client->num_servers;
        j++;
        free(VECTOR_GET(v, char*, i)); 
    }
    //remove duplicte server ids
    for (int i = 0; i < serv_size; i++)
    {
        for(int j = i + 1; j < serv_size; j++)
        {
            if(arr[i] == arr[j])
            {
                for(int k = j; k < serv_size; k++)
                {
                    arr[k] = arr[k + 1];
                }
                serv_size--;
                j--;
            }
        }
    }

    hndl = (hg_handle_t*)malloc(sizeof(hg_handle_t)*serv_size);
    serv_req = (margo_request*)malloc(sizeof(margo_request)*serv_size);
    
    for (int i = 0; i < serv_size; ++i)
    {
        hg_addr_t svr_addr;
        margo_request req;
        hg_handle_t h;
        int serv_id = arr[i];
        margo_addr_lookup(client->mid, client->server_address[serv_id], &svr_addr);
        margo_create(client->mid, svr_addr, client->finalize_id, &h);
        margo_iforward(h, &in, &req);
        hndl[i] = h;
        serv_req[i] = req;
        margo_addr_free(client->mid, svr_addr);
    }
    for (int i = 0; i < serv_size; ++i){
        margo_wait(serv_req[i]);
        response_t resp;
        margo_get_output(hndl[i], &resp);
        ret = resp.ret;
        margo_free_output(hndl[i], &resp);
        margo_destroy(hndl[i]);
        int serv_id = arr[i];
        if(ret!=MESSAGING_SUCCESS){
            fprintf(stderr, "Could not unregister client %s from server %s\n", client->addr_string, client->server_address[serv_id]);
            return ret;
        }
        
    }
    free(hndl);
    free(serv_req);
    free(arr);
    return ret;

}


static void notify_rpc(hg_handle_t h)
{
    hg_return_t ret;

    bulk_data_t in;
    response_t out;

    margo_instance_id mid = margo_hg_handle_get_instance(h);
    const struct hg_info* info = margo_get_info(h);
    messaging_client_t client = (messaging_client_t) margo_registered_data(mid, info->id);
  
    ret = margo_get_input(h, &in);
    assert(ret == HG_SUCCESS);

    char *namesp, *topic, *tag_msg, *raw_buf;
    int namespace_len, topic_len, tag_len;

    raw_buf = (char*) in.evnt.raw_data;
    namespace_len = ((int *)raw_buf)[0];
    topic_len = ((int *)raw_buf)[1];
    tag_len = ((int *)raw_buf)[2];

    namesp = (char *)malloc(namespace_len);
    topic = (char *)malloc(topic_len);
    tag_msg = malloc(tag_len);
    memcpy(namesp, &raw_buf[sizeof(int)*3], namespace_len);
    memcpy(topic, &raw_buf[sizeof(int)*3+namespace_len], topic_len);
    memcpy(tag_msg, &raw_buf[sizeof(int)*3+namespace_len+topic_len], tag_len);

    out.ret = MESSAGING_SUCCESS;
    margo_respond(h, &out);


    vector v;
    v = map_get_value(client->t, namesp, topic);
    void *handler_args;
    void (*handler_func)(void *, void *);
    handler_args = VECTOR_GET(v, void*, 1);
    handler_func = VECTOR_GET(v, void*, 0);
    (*handler_func)(handler_args, (void *)tag_msg);

    ret = margo_free_input(h, &in);
    assert(ret == HG_SUCCESS);

    ret = margo_destroy(h);
    assert(ret == HG_SUCCESS);
    
}
DEFINE_MARGO_RPC_HANDLER(notify_rpc)
