/* /////////////////////////////////////////////////////////////////////////
 * File:        inetstl/includes/std/in_addr.h
 *
 * Purpose:     Discrimination of various operating-specific header files
 *              for Internet-related types.
 *
 * Created:
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2009, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name(s) of Matthew Wilson and Synesis Software nor the names of
 *   any contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////// */


/** \file inetstl/includes/std/in_addr.h
 *
 * \brief [C, C++] Discrimination of various operating-specific header files
 *   for Internet-related types
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef INETSTL_INCL_INETSTL_INCLUDES_STD_H_IN_ADDR
#define INETSTL_INCL_INETSTL_INCLUDES_STD_H_IN_ADDR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define INETSTL_VER_INETSTL_INCLUDES_STD_H_IN_ADDR_MAJOR       1
# define INETSTL_VER_INETSTL_INCLUDES_STD_H_IN_ADDR_MINOR       0
# define INETSTL_VER_INETSTL_INCLUDES_STD_H_IN_ADDR_REVISION    2
# define INETSTL_VER_INETSTL_INCLUDES_STD_H_IN_ADDR_EDIT        5
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#if defined(WIN32)

# if !defined(_WINSOCKAPI_) && \
     !defined(_WINSOCK2API_)
#  include <winsock2.h>
# endif /* !_WINSOCKAPI_ && !_WINSOCK2API_ */

#else /* ? OS */

# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>

#endif /* OS */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* INETSTL_INCL_INETSTL_INCLUDES_STD_H_IN_ADDR */

/* ///////////////////////////// end of file //////////////////////////// */
