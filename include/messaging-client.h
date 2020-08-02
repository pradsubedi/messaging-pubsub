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



#ifndef __MESSAGING_CLIENT_H
#define __MESSAGING_CLIENT_H

#include <margo.h>
#include <messaging-common.h>
#include <ss_data.h>
#include <mpi.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct messaging_client* messaging_client_t;
#define MESSAGING_CLIENT_NULL ((messaging_client_t)NULL)

/**
 * @brief Creates a MESSAGING client.
 *
 * @param[in] mid Margo instance
 * @param[out] client MESSAGING client
 *
 * @return MESSAGING_SUCCESS or error code defined in messaging-common.h
 */
int client_init( margo_instance_id mid,
        messaging_client_t* client); 

/**
 * @brief Creates a MESSAGING client.
 *
 * @param[in] mid Margo instance
 * @param[in] comm MPI Comminicator
 * @param[out] client MESSAGING client
 *
 * @return MESSAGING_SUCCESS or error code defined in messaging-common.h
 */
int client_init_with_mpi( margo_instance_id mid,
		MPI_Comm comm, 
        messaging_client_t* client); 

/**
 * @brief Finalizes a MESSAGING client.
 *
 * @param[in] client MESSAGING client to finalize
 *
 * @return MESSAGING_SUCCESS or error code defined in messaging-common.h
 */
int client_finalize(messaging_client_t client);


/**
 * @brief Publishes 'messg' of length 'msg_len' to a 'topic' topic in 'namesp' Namespace
 *
 * @param[in] client MESSAGING client that is publishing the message
 * @param[in] namesp Publishes message for namesp
 * @param[in] topic Publishes message to topic topic
 * @param[in] messg Publishes messg message
 * @param[in] msg_len Length of the msg to be published
 *
 * @return MESSAGING_SUCCESS or error code defined in messaging-common.h
 */
int publish(messaging_client_t client, 
        char *namesp, 
        char *topic, 
        void *messg, 
        int msg_len);


/**
 * @brief Subscribes to a 'topic' topic in 'namesp' Namespace.
 * 
 * When client receives notification 'callback' handler will be triggered
 * with arguments (void* callback_args, void* msg), where msg is the
 * published message
 *
 * @param[in] client MESSAGING client that is subscribing to a namespace and topic
 * @param[in] namesp Subscribes to Namespace: 'namesp'
 * @param[in] topic Subscribes to topic: 'topic'
 * @param[in] callback pointer to the handler
 * @param[in] callback_args arguments to callback
 *
 * @return MESSAGING_SUCCESS or error code defined in messaging-common.h
 */

int subscribe(messaging_client_t client, 
        char *namesp, 
        char *topic,
        void (*callback)(void*, void*),
        void *callback_args);

/**
 * @brief Unsubscribes from a 'topic' topic in 'namesp' Namespace.
 *
 * @param[in] client MESSAGING client that is subscribing to a namespace and topic
 * @param[in] namesp Namespace: 'namesp'
 * @param[in] topic topic: 'topic'
 *
 * @return MESSAGING_SUCCESS or error code defined in messaging-common.h
 */

int unsubscribe(messaging_client_t client, 
        char *namesp, 
        char *topic);

#if defined(__cplusplus)
}
#endif

#endif
