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
#include <assert.h>
#include <margo.h>
#include <unistd.h>
#include <messaging-client.h>
#include <mpi.h>
#include "timer.h"

static struct timer timer_;
double tm_st, tm_end, tm_diff, tm_max;
int counter, rank, num_steps, num_subscribers;
messaging_client_t c;
margo_instance_id mid;


void A(void* harg, void* received_msg) 
{ 
    counter++;
    if(counter == num_steps){
        if(rank < num_subscribers){
            tm_end = timer_read(&timer_);
            fprintf(stderr, "Rank %d: total workflow time %lf\n", rank, tm_end - tm_st);
        }
    }
} 

void B(void* harg, void* received_msg) 
{ 
    fprintf(stderr, "running callback function B\n"); 
    fprintf(stderr, "Func B, Rank %d: Msg from publisher: %s\n", *(int*)harg, (char*)received_msg);
} 

int main(int argc, char **argv){
    
    if(argc < 2){
        fprintf(stderr, "Usage: mpirun -n num_processes ./client num_steps num_subscribers\n");
        return -1;
    }
    counter = 0;
    char *listen_addr_str = "verbs";
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm gcomm = MPI_COMM_WORLD;
    char msg[1024];
    for (int i = 0; i < 1023; ++i)
    {
        msg[i] = 'a';
    }
    msg[1023] = '\0';
    num_steps = atoi(argv[1]);
    num_subscribers = atoi(argv[2]);

    int color = 1;
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &gcomm);

    mid = margo_init(listen_addr_str, MARGO_SERVER_MODE, 1, -1);
    assert(mid);
    //Use client_init() if clients do not use gcomm
    //int ret = client_init(mid, &c);
    int ret = client_init_with_mpi(mid, gcomm, &c);
    if(c == MESSAGING_CLIENT_NULL){
        fprintf(stderr, "Client is NULL\n");
        return 0;
    }
    void (*handler)(void*, void*);
    void *hargs;
    hargs = malloc(sizeof(int));
    memcpy(hargs, &rank, sizeof(int));
    
    if(ret != 0) return ret;

    /*
    if(rank==0){
        handler = A;
        sleep(3);
        char* message = "msg_from_rank_0";
        ret = publish(c, "first_namespace", "first_subscribe", (void*)message, strlen(message));
        sleep(8);

        ret = subscribe(c, "first_namespace", "second_msg", handler, hargs);
        ret = publish(c, "second_namespace", "second_subscribe", (void*)message, strlen(message));
        sleep(10);

        ret = publish(c, "first_namespace", "first_subscribe", (void*)message, strlen(message));
        sleep(20);

    }
    if(rank == 1){
        handler = B;
        char* message = "msg_from_rank_1";
        ret = subscribe(c, "second_namespace", "second_subscribe", handler, hargs);
        sleep(10);
        ret = subscribe(c, "first_namespace", "first_subscribe", handler, hargs);
        ret = unsubscribe(c, "first_namespace", "first_subscribe");
        sleep(5);
        ret = publish(c, "first_namespace", "second_msg", (void*)message, strlen(message));
        sleep(10);
    }
    */
    timer_init(&timer_, 1);
    timer_start(&timer_);   
    if(rank < num_subscribers){
        handler = A;
        tm_st = timer_read(&timer_);
        ret = subscribe(c, "subs", "pub_msg", handler, hargs);
        fprintf(stderr, "Subscribe complete for rank %d\n", rank);
        //margo_wait_for_finalize(mid);
        
    }else{
        sleep(1);
        //tm_st = timer_read(&timer_);
        for (int i = 0; i < num_steps; ++i)
        {
            ret = publish(c, "subs", "pub_msg", (void*)msg, strlen(msg));
        }
        //tm_end = timer_read(&timer_);
        
    }  

    
    margo_wait_for_finalize(mid);
    client_finalize(c);
    MPI_Finalize();
    return 0;

    // make margo wait for finalize
    //margo_wait_for_finalize(mid);

}