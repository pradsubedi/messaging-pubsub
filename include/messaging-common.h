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

#ifndef __MESSAGING_COMMON_H
#define __MESSAGING_COMMON_H

#if defined(__cplusplus)
extern "C" {
#endif

#define MESSAGING_SUCCESS          0 /* Success */
#define MESSAGING_ERR_ALLOCATION  -1 /* Error allocating something */
#define MESSAGING_ERR_INVALID_ARG -2 /* An argument is invalid */
#define MESSAGING_ERR_MERCURY     -3 /* An error happened calling a Mercury function */
#define MESSAGING_ERR_SIZE        -5 /* Client did not allocate enough for the requested data */
#define MESSAGING_ERR_ARGOBOTS    -6 /* Argobots related error */
#define MESSAGING_ERR_UNKNOWN_PR    -7 /* Could not find server */
#define MESSAGING_ERR_UNKNOWN_OBJ    -8 /* Could not find the object*/
#define MESSAGING_ERR_END         -9 /* End of range for valid error codes */




#if defined(__cplusplus)
}
#endif


#endif
