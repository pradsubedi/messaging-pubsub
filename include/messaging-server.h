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


#ifndef __MESSAGING_SERVER_H
#define __MESSAGING_SERVER_H

#include <margo.h>
#include <messaging-common.h>
#include <ss_data.h>
#include <mpi.h>

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct messaging_server* messaging_server_t;
#define MESSAGING_SERVER_NULL ((messaging_server_t)NULL)



/**
 * @brief Creates a MESSAGING server.
 *
 * @param[in] mid Margo instance
 * @param[in] comm MPI Comminicator
 * @param[out] server MESSAGING server
  * @return MESSAGING_SUCCESS or error code defined in messaging-common.h
 */
int server_init(margo_instance_id mid, MPI_Comm comm, messaging_server_t* server);
	

/**
 * @brief Destroys the Messaging server and deregisters its RPC.
 *
 * @param[in] server Messaging server
 *
 * @return MESSAGING_SUCCESS or error code defined in messaging-common.h
 */
int server_destroy(messaging_server_t server);

#if defined(__cplusplus)
}
#endif

#endif
