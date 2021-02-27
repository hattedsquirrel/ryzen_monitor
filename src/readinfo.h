/**
 * Ryzen SMU Userspace Sensor Monitor
 * Copyright (C) 2020-2021
 *    Florian Huehn <hattedsquirrel@gmail.com> (https://hattedsquirrel.net)
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

#ifndef READINFO_H
#define READINFO_H

typedef struct {
    const char *cpu_name;
    const char *codename;
    const char *smu_fw_ver;
    unsigned int if_ver;   
    unsigned int cores;
    unsigned int ccds;
    unsigned int ccxs;
    unsigned int cores_per_ccx;
    unsigned int core_disable_map;
    unsigned int enabled_cores_count;
} system_info;

void print_memory_timings();
unsigned int get_processor_topology(system_info *sysinfo, unsigned int zen_version);
unsigned int count_set_bits(unsigned int v);
const char* get_processor_name();
void append_u32_to_str(char* buffer, unsigned int val);

#endif