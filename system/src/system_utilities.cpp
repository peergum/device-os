/**
 ******************************************************************************
  Copyright (c) 2020 Particle Industries, Inc.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */


// this file is used to compile the Catch unit tests on gcc, so contains few dependencies

#include "system_task.h"
#include <algorithm>
#include "system_version.h"
#include "static_assert.h"
#include "spark_macros.h"
#include "system_utilities.h"
#include <stdio.h>

using std::min;

/**
 * Series 1s (5 times), 2s (5 times), 4s (5 times)...64s (5 times) then to 128s thereafter.
 * @param connection_attempts
 * @return The number of milliseconds to backoff.
 */
unsigned backoff_period(unsigned connection_attempts)
{
    if (!connection_attempts)
        return 0;
    unsigned exponent = min(7u, (connection_attempts-1)/5);
    return 1000*(1<<exponent);
}

STATIC_ASSERT(system_version_info_size, sizeof(SystemVersionInfo)==28);

int system_version_info(SystemVersionInfo* info, void* /*reserved*/)
{
    if (info)
    {
        if (info->size>=28)
        {
            info->versionNumber = SYSTEM_VERSION;
            strncpy(info->versionString, stringify(SYSTEM_VERSION_STRING), sizeof(info->versionString));
        }
    }
    return sizeof(SystemVersionInfo);
}

int update_public_server_domain(ServerAddress* server_addr, void* /*reserved*/)
{
    if (server_addr)
    {
        const size_t MAJOR_VERSION_TOKEN_SIZE_MAX = 6; // good for "v0." ~ "v255." which should last a while
        const char ID_TOKEN_STR[] = "$id.";
        size_t max_size = sizeof(server_addr->domain);
        size_t server_addr_len = strlen(server_addr->domain);
        if ( (server_addr_len + MAJOR_VERSION_TOKEN_SIZE_MAX) < max_size)
        {
            char* id_ptr = strstr(server_addr->domain, ID_TOKEN_STR);
            if (id_ptr && (id_ptr - server_addr->domain) == 0) { // $id. must be leading the domain
                SystemVersionInfo sys_ver;
                char new_domain[sizeof(server_addr->domain)] = {};
                system_version_info(&sys_ver, nullptr);
                uint8_t major_version = BYTE_N(sys_ver.versionNumber, 3);

                int new_size = snprintf(new_domain, server_addr_len + MAJOR_VERSION_TOKEN_SIZE_MAX,
                    "%sv%d.%s", ID_TOKEN_STR, major_version, id_ptr + sizeof(ID_TOKEN_STR)-1);
                if (new_size > 0 && new_size < (int)max_size) {
                    strncpy(server_addr->domain, new_domain, new_size);
                    return new_size;
                }
            }
        }
    }
    return 0;
}
