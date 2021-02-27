/**
 * Ryzen SMU Userspace Sensor Monitor
 * Copyright (C) 2020-2021
 *    Florian Huehn <hattedsquirrel@gmail.com> (https://hattedsquirrel.net)
 *    Based on work of:
 *    Leonardo Gates <leogatesx9r@protonmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include <stdlib.h>
#include <stdio.h>
#include <cpuid.h>
#include <ctype.h>
#include <libsmu.h>
#include "readinfo.h"

extern smu_obj_t obj;

#define READ_SMN_V1(offs) { if (smu_read_smn_addr(&obj, offs + offset, &value1) != SMU_Return_OK) goto _READ_ERROR; }
#define READ_SMN_V2(offs) { if (smu_read_smn_addr(&obj, offs + offset, &value2) != SMU_Return_OK) goto _READ_ERROR; }

void append_u32_to_str(char* buffer, unsigned int val) {
    buffer[0] = val & 0xff;
    buffer[1] = (val >>  8) & 0xff;
    buffer[2] = (val >> 16) & 0xff;
    buffer[3] = (val >> 24) & 0xff;
}

const char* get_processor_name() {
    unsigned int eax, ebx, ecx, edx;
    int i;
    static char buffer[50] = { 0 }, *p;

    i=0;
    __get_cpuid(0x80000002, &eax, &ebx, &ecx, &edx);
    append_u32_to_str(buffer+i, eax); i+=4;
    append_u32_to_str(buffer+i, ebx); i+=4;
    append_u32_to_str(buffer+i, ecx); i+=4;
    append_u32_to_str(buffer+i, edx); i+=4;

    __get_cpuid(0x80000003, &eax, &ebx, &ecx, &edx);
    append_u32_to_str(buffer+i, eax); i+=4;
    append_u32_to_str(buffer+i, ebx); i+=4;
    append_u32_to_str(buffer+i, ecx); i+=4;
    append_u32_to_str(buffer+i, edx); i+=4;

    __get_cpuid(0x80000004, &eax, &ebx, &ecx, &edx);
    append_u32_to_str(buffer+i, eax); i+=4;
    append_u32_to_str(buffer+i, ebx); i+=4;
    append_u32_to_str(buffer+i, ecx); i+=4;
    append_u32_to_str(buffer+i, edx); i+=4;

    // Trim whitespaces
    p = buffer;
    i = strlen(p)-1;
    while(isspace(p[i]) && i>=0) p[i--]=0;
    while(*p && isspace(*p)) p++;

    return p;
}

unsigned int count_set_bits(unsigned int v) {
    unsigned int result = 0;

    while(v!=0) {
        if(v&1) result++;
        v >>= 1;
    }

    return result;
}

unsigned int get_processor_topology(system_info *sysinfo, unsigned int zen_version) {
    unsigned int ccds_present, ccds_down, ccd_enable_map, ccd_disable_map,
        core_disable_map_addr, core_disable_map_tmp, logical_cores, threads_per_core,
        fam, model, fuse1, fuse2, offs, eax, ebx, ecx, edx;

    __get_cpuid(0x00000001, &eax, &ebx, &ecx, &edx);
    fam = ((eax & 0xf00) >> 8) + ((eax & 0xff00000) >> 20);
    model = ((eax & 0xf0000) >> 12) + ((eax & 0xf0) >> 4);
    logical_cores = (ebx >> 16) & 0xFF;

    __get_cpuid(0x8000001E, &eax, &ebx, &ecx, &edx);
    threads_per_core = ((ebx >> 8) & 0xF) + 1;

    fuse1 = 0x5D218;
    fuse2 = 0x5D21C;
    offs = 0x238;

    if (fam == 0x19) {
        //fuse1 += 0x10;
        //fuse2 += 0x10;
        offs = 0x598;
    }
    else if (fam == 0x17 && model != 0x71) {
        fuse1 += 0x40;
        fuse2 += 0x40;
    }

    if (smu_read_smn_addr(&obj, fuse1, &ccds_present) != SMU_Return_OK ||
        smu_read_smn_addr(&obj, fuse2, &ccds_down) != SMU_Return_OK) {
        perror("Failed to read CCD fuses");
        exit(-1);
    }

    ccd_enable_map = (ccds_present >> 22) & 0xff;
    ccd_disable_map = ((ccds_present >> 30) & 0x3) | ((ccds_down & 0x3f) << 2);

    core_disable_map_addr = (0x30081800 + offs);
    sysinfo->core_disable_map = 0;
    if (ccd_enable_map & 0x01) {
        if (smu_read_smn_addr(&obj, core_disable_map_addr, &core_disable_map_tmp) != SMU_Return_OK) {
            perror("Failed to read disabled core fuse");
            exit(-1);
        }
        sysinfo->core_disable_map |= core_disable_map_tmp & 0xff;
    }
    if (ccd_enable_map & 0x02) {
        if (smu_read_smn_addr(&obj, core_disable_map_addr|0x2000000, &core_disable_map_tmp) != SMU_Return_OK) {
            perror("Failed to read disabled core fuse");
            exit(-1);
        }
        sysinfo->core_disable_map |= (core_disable_map_tmp & 0xff)<<8;
    }


    if (!threads_per_core)
        sysinfo->cores = logical_cores;
    else
        sysinfo->cores = logical_cores / threads_per_core;

    switch(zen_version)
    {
        case 3:
            //Zen3 does not have CCXs anymore. They now have 8 cores per CCD
            //each with the same access to each other and the common L3 cache.
            sysinfo->ccxs = 0;
            sysinfo->ccds = count_set_bits(ccd_enable_map);
            sysinfo->cores_per_ccx = 8 - count_set_bits(sysinfo->core_disable_map & 0xff);
            sysinfo->enabled_cores_count = 8*(sysinfo->ccds) - count_set_bits(sysinfo->core_disable_map);
            break;

        case 2:
        default:
            sysinfo->cores_per_ccx = (8 - count_set_bits(sysinfo->core_disable_map & 0xff)) / 2;
            sysinfo->ccds = count_set_bits(ccd_enable_map);
            sysinfo->ccxs = sysinfo->cores == sysinfo->cores_per_ccx ? 1 : sysinfo->ccds * 2;
            sysinfo->enabled_cores_count = 8*(sysinfo->ccds) - count_set_bits(sysinfo->core_disable_map);
            break;
    }
    sysinfo->available=1;
}

void print_memory_timings() {
    const char* bool_str[2] = { "Disabled", "Enabled" };
    unsigned int value1, value2, offset;

    READ_SMN_V1(0x50200);
    offset = value1 == 0x300 ? 0x100000 : 0;

    READ_SMN_V1(0x50050); READ_SMN_V2(0x50058);
    fprintf(stdout, "BankGroupSwap: %s\n",
        bool_str[!(value1 == value2 && value1 == 0x87654321)]);

    READ_SMN_V1(0x500D0); READ_SMN_V2(0x500D4);
    fprintf(stdout, "BankGroupSwapAlt: %s\n",
        bool_str[(value1 >> 4 & 0x7F) != 0 || (value2 >> 4 & 0x7F) != 0]);

    READ_SMN_V1(0x50200); READ_SMN_V2(0x50204);
    fprintf(stdout, "Memory Clock: %.0f MHz\nGDM: %s\nCR: %s\nTcl: %d\nTras: %d\nTrcdrd: %d\nTrcdwr: %d\n",
        (value1 & 0x7f) / 3.f * 100.f,
        bool_str[((value1 >> 11) & 1) == 1],
        ((value1 & 0x400) >> 10) != 0 ? "2T" : "1T",
        value2 & 0x3f,
        value2 >> 8 & 0x7f,
        value2 >> 16 & 0x3f,
        value2 >> 24 & 0x3f);

    READ_SMN_V1(0x50208); READ_SMN_V2(0x5020C);
    fprintf(stdout, "Trc: %d\nTrp: %d\nTrrds: %d\nTrrdl: %d\nTrtp: %d\n",
        value1 & 0xff,
        value1 >> 16 & 0x3f,
        value2 & 0x1f,
        value2 >> 8 & 0x1f,
        value2 >> 24 & 0x1f);

    READ_SMN_V1(0x50210); READ_SMN_V2(0x50214);
    fprintf(stdout, "Tfaw: %d\nTcwl: %d\nTwtrs: %d\nTwtrl: %d\n",
        value1 & 0xff,
        value2 & 0x3f,
        value2 >> 8 & 0x1f,
        value2 >> 16 & 0x3f);

    READ_SMN_V1(0x50218); READ_SMN_V2(0x50220);
    fprintf(stdout, "Twr: %d\nTrdrddd: %d\nTrdrdsd: %d\nTrdrdsc: %d\nTrdrdscl: %d\n",
        value1 & 0xff,
        value2 & 0xf,
        value2 >> 8 & 0xf,
        value2 >> 16 & 0xf,
        value2 >> 24 & 0x3f);

    READ_SMN_V1(0x50224); READ_SMN_V2(0x50228);
    fprintf(stdout, "Twrwrdd: %d\nTwrwrsd: %d\nTwrwrsc: %d\nTwrwrscl: %d\nTwrrd: %d\nTrdwr: %d\n",
        value1 & 0xf,
        value1 >> 8 & 0xf,
        value1 >> 16 & 0xf,
        value1 >> 24 & 0x3f,
        value2 & 0xf,
        value2 >> 8 & 0x1f);

    READ_SMN_V1(0x50254);
    fprintf(stdout, "Tcke: %d\n", value1 >> 24 & 0x1f);

    READ_SMN_V1(0x50260); READ_SMN_V2(0x50264);
    if (value1 != value2 && value1 == 0x21060138)
        value1 = value2;

    fprintf(stdout, "Trfc: %d\nTrfc2: %d\nTrfc4: %d\n",
        value1 & 0x3ff,
        value1 >> 11 & 0x3ff,
        value1 >> 22 & 0x3ff);

    exit(0);

_READ_ERROR:
    fprintf(stderr, "Unable to read SMN address space.");
    exit(1);
}
