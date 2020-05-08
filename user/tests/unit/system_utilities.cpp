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

#include "system_utilities.h"
#include "system_version.h"
#include "spark_macros.h"
#undef WARN
#undef INFO

#include "catch.hpp"
#include "hippomocks.h"

namespace {

// $id.udp.particle.io
const unsigned char backup_udp_public_server_address[] = {
  0x01, 0x13, 0x24, 0x69, 0x64, 0x2e, 0x75, 0x64, 0x70, 0x2e, 0x70, 0x61,
  0x72, 0x74, 0x69, 0x63, 0x6c, 0x65, 0x2e, 0x69, 0x6f, 0x00
};
const size_t backup_udp_public_server_address_size = sizeof(backup_udp_public_server_address);

// $id.udp-mesh.particle.io
const unsigned char backup_udp_mesh_public_server_address[] = {
  0x01, 0x18, 0x24, 0x69, 0x64, 0x2e, 0x75, 0x64, 0x70, 0x2d, 0x6d, 0x65,
  0x73, 0x68, 0x2e, 0x70, 0x61, 0x72, 0x74, 0x69, 0x63, 0x6c, 0x65, 0x2e,
  0x69, 0x6f, 0x00
};
const size_t backup_udp_mesh_public_server_address_size = sizeof(backup_udp_mesh_public_server_address);

// devices.spark.io
const unsigned char backup_tcp_public_server_address[] = {
  0x01, 0x0f, 0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x2e, 0x73, 0x70, 0x61,
  0x72, 0x6b, 0x2e, 0x69, 0x6f, 0x00
};
const size_t backup_tcp_public_server_address_size = sizeof(backup_tcp_public_server_address);

#define MOCK_SYSTEM_VERSION_STRING ("vX.Y.Z-unused")
uint32_t mock_system_version = 0;

int get_major_version(SystemVersionInfo* sys_ver) {
    if (sys_ver) {
        int sys_ver_size = system_version_info(sys_ver, nullptr);
        return BYTE_N(sys_ver->versionNumber, 3);
    }
    return -1;
}

int get_major_version_size(int ver) {
    if (ver > 99) {
        return 3;
    } else if (ver > 9) {
        return 2;
    } else {
        return 1;
    }
}

class Mocks {
public:
    Mocks() {
        mocks_.OnCallFunc(system_version_info).Do([](SystemVersionInfo* info, void*)->int {
            if (info)
            {
                if (info->size>=28)
                {
                    info->versionNumber = mock_system_version;
                    strncpy(info->versionString, stringify(MOCK_SYSTEM_VERSION_STRING), sizeof(info->versionString));
                }
            }
            return sizeof(SystemVersionInfo);
        });
    }

private:
    MockRepository mocks_;
};

} // namespace

TEST_CASE("system_utilities") {

    Mocks mocks;
    ServerAddress server_addr = {};

    SECTION("[UDP] Server public address is updated with major system version v1") {
        mock_system_version = SYSTEM_VERSION_DEFAULT(1, 2, 3);
        SystemVersionInfo sys_ver;
        int major_version = get_major_version(&sys_ver);
        int major_version_size = get_major_version_size(major_version) + 2;

        char expected_str1[] = "$id.v";
        char expected_str2[] = ".udp.particle.io";
        char expected_buf[sizeof(server_addr.domain)] = {};
        sprintf(expected_buf, "%s%d%s", expected_str1, major_version, expected_str2);
        memcpy(&server_addr, backup_udp_public_server_address, backup_udp_public_server_address_size);

        // update server address after it was potentially restored from backup
        int ret = update_public_server_domain(&server_addr, nullptr);

        REQUIRE_THAT( expected_buf, Equals(server_addr.domain));

        // Don't be fooled that these are really equal, backup_udp_public_server_address_size
        // is already +3 due to leading .type, .size and trailing null character.
        REQUIRE(ret==(backup_udp_public_server_address_size - 3) + major_version_size);
    }

    SECTION("[UDP-MESH] Server public address is updated with major system version v2") {

        mock_system_version = SYSTEM_VERSION_DEFAULT(2, 3, 4);
        SystemVersionInfo sys_ver;
        int major_version = get_major_version(&sys_ver);
        int major_version_size = get_major_version_size(major_version) + 2;

        char expected_str1[] = "$id.v";
        char expected_str2[] = ".udp-mesh.particle.io";
        char expected_buf[sizeof(server_addr.domain)] = {};
        sprintf(expected_buf, "%s%d%s", expected_str1, major_version, expected_str2);
        memcpy(&server_addr, backup_udp_mesh_public_server_address, backup_udp_mesh_public_server_address_size);

        // update server address after it was potentially restored from backup
        int ret = update_public_server_domain(&server_addr, nullptr);

        REQUIRE_THAT( expected_buf, Equals(server_addr.domain));

        // Don't be fooled that these are really equal, backup_udp_public_server_address_size
        // is already +3 due to leading .type, .size and trailing null character.
        REQUIRE(ret==(backup_udp_mesh_public_server_address_size - 3) + major_version_size);
    }

    SECTION("[UDP-MESH] Server public address is updated with major system version v255") {

        mock_system_version = SYSTEM_VERSION_DEFAULT(255, 3, 4);
        SystemVersionInfo sys_ver;
        int major_version = get_major_version(&sys_ver);
        int major_version_size = get_major_version_size(major_version) + 2;

        char expected_str1[] = "$id.v";
        char expected_str2[] = ".udp-mesh.particle.io";
        char expected_buf[sizeof(server_addr.domain)] = {};
        sprintf(expected_buf, "%s%d%s", expected_str1, major_version, expected_str2);
        memcpy(&server_addr, backup_udp_mesh_public_server_address, backup_udp_mesh_public_server_address_size);

        // update server address after it was potentially restored from backup
        int ret = update_public_server_domain(&server_addr, nullptr);

        REQUIRE_THAT( expected_buf, Equals(server_addr.domain));

        // Don't be fooled that these are really equal, backup_udp_public_server_address_size
        // is already +3 due to leading .type, .size and trailing null character.
        REQUIRE(ret==(backup_udp_mesh_public_server_address_size - 3) + major_version_size);
    }

    SECTION("[TCP] Server public address is NOT updated with major system version") {

        mock_system_version = SYSTEM_VERSION_DEFAULT(9, 1, 1);
        SystemVersionInfo sys_ver;

        char expected_buf[] = "device.spark.io";
        memcpy(&server_addr, backup_tcp_public_server_address, sizeof(backup_tcp_public_server_address));

        // update server address after it was potentially restored from backup
        int ret = update_public_server_domain(&server_addr, nullptr);

        REQUIRE_THAT( expected_buf, Equals(server_addr.domain));

        // expecting not to alter TCP PSK
        REQUIRE(ret==0);
    }

    SECTION("update_public_server_domain() returns when passed a null pointer") {

        mock_system_version = SYSTEM_VERSION_DEFAULT(1, 2, 3);
        SystemVersionInfo sys_ver;

        int ret = update_public_server_domain(nullptr, nullptr);
        REQUIRE(ret==0);
    }

    SECTION("update_public_server_domain() $id. must be leading the domain") {

        mock_system_version = SYSTEM_VERSION_DEFAULT(1, 2, 3);
        SystemVersionInfo sys_ver;

        ServerAddress server_addr = {};
        char fake_domain[64] = "udp-mesh.particle.io.$id.udp-mesh.particle.io";
        memcpy(&server_addr.domain, fake_domain, sizeof(fake_domain));
        int ret = update_public_server_domain(&server_addr, nullptr);
        REQUIRE_THAT( fake_domain, Equals(server_addr.domain));
        REQUIRE(ret==0);
    }

    SECTION("update_public_server_domain() cannot force a buffer overflow") {

        mock_system_version = SYSTEM_VERSION_DEFAULT(255, 2, 3);
        SystemVersionInfo sys_ver;
        int major_version = get_major_version(&sys_ver);
        int major_version_size = get_major_version_size(major_version) + 2;

        // 57 chars is the max
        ServerAddress server_addr = {};
        char fake_domain[64] = "$id.udp-mesh.particle.io.udp-mesh.particle.io.udp-mesh.pa";
        char expected_str1[] = "$id.v";
        char expected_str2[] = ".udp-mesh.particle.io.udp-mesh.particle.io.udp-mesh.pa";
        char expected_buf[sizeof(server_addr.domain)] = {};
        sprintf(expected_buf, "%s%d%s", expected_str1, major_version, expected_str2);
        memcpy(&server_addr.domain, fake_domain, sizeof(fake_domain));
        int ret = update_public_server_domain(&server_addr, nullptr);
        REQUIRE_THAT( server_addr.domain, Equals(expected_buf));
        REQUIRE(ret==strlen(fake_domain) + major_version_size);

        // 58 chars is 1 too many
        char fake_domain1[64] = "$id.udp-mesh.particle.io.udp-mesh.particle.io.udp-mesh.par";
        memcpy(&server_addr.domain, fake_domain1, sizeof(fake_domain1));
        REQUIRE_THAT( server_addr.domain, Equals(fake_domain1));
        ret = update_public_server_domain(&server_addr, nullptr);
        REQUIRE_THAT( server_addr.domain, Equals(fake_domain1));
        REQUIRE(ret==0);
    }

    SECTION("update_public_server_domain() returns without modifying string if forced to be too large") {

        mock_system_version = SYSTEM_VERSION_DEFAULT(1, 2, 3);
        SystemVersionInfo sys_ver;

        ServerAddress server_addr = {};
        char fake_domain[65] = "$id.udp-mesh.particle.io.$id.udp-mesh.particle.io.$id.udpmesh.io";
        memcpy(&server_addr.domain, fake_domain, sizeof(fake_domain));
        REQUIRE_THAT( server_addr.domain, Equals(fake_domain));
        int ret = update_public_server_domain(&server_addr, nullptr);
        REQUIRE_THAT( server_addr.domain, Equals(fake_domain));
        REQUIRE(ret==0);
    }
}
