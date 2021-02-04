/**
 * Ryzen SMU Userspace Sensor Monitor
 * Copyright (C) 2020-2021
 *    Florian Huehn <hattedsquirrel@gmail.com> (https://hattedsquirrel.net)
 *    Leonardo Gates <leogatesx9r@protonmail.com>
 *
 * This program is free software: you can redistribute it &&/or modify
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

#define _GNU_SOURCE

#include <math.h>
#include <sched.h>
#include <ctype.h>
#include <fcntl.h>
#include <cpuid.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <libsmu.h>
#include "pm_tables.h"

#define PROGRAM_VERSION "1.0.1"

#define READ_SMN_V1(offs) { if (smu_read_smn_addr(&obj, offs + offset, &value1) != SMU_Return_OK) goto _READ_ERROR; }
#define READ_SMN_V2(offs) { if (smu_read_smn_addr(&obj, offs + offset, &value2) != SMU_Return_OK) goto _READ_ERROR; }

static smu_obj_t obj;
static int update_time_s = 1;
static int show_disabled_cores = 0;

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

void append_u32_to_str(char* buffer, unsigned int val) {
    char tmp[12] = { 0 };

    sprintf(tmp, "%c%c%c%c", val & 0xff, val >> 8 & 0xff, val >> 16 & 0xff, val >> 24 & 0xff);
    strcat(buffer, tmp);
}

const char* get_processor_name() {
    unsigned int eax, ebx, ecx, edx, l;
    static char buffer[50] = { 0 }, *p;

    __get_cpuid(0x80000002, &eax, &ebx, &ecx, &edx);
    append_u32_to_str(buffer, eax);
    append_u32_to_str(buffer, ebx);
    append_u32_to_str(buffer, ecx);
    append_u32_to_str(buffer, edx);

    __get_cpuid(0x80000003, &eax, &ebx, &ecx, &edx);
    append_u32_to_str(buffer, eax);
    append_u32_to_str(buffer, ebx);
    append_u32_to_str(buffer, ecx);
    append_u32_to_str(buffer, edx);

    __get_cpuid(0x80000004, &eax, &ebx, &ecx, &edx);
    append_u32_to_str(buffer, eax);
    append_u32_to_str(buffer, ebx);
    append_u32_to_str(buffer, ecx);
    append_u32_to_str(buffer, edx);

    // Trim whitespaces
    p = buffer;
    l = strlen(p);
    while(isspace(p[l - 1])) p[--l] = 0;
    while(* p && isspace(* p)) ++p, --l;

    return buffer;
}

unsigned int count_set_bits(unsigned int v) {
    unsigned int result = 0;

    while(v != 0) {
        if (v & 1)
            result++;

        v >>= 1;
    }

    return result;
}

unsigned int get_processor_topology(unsigned int* ccds, unsigned int *ccxs,
    unsigned int *cores_per_ccx, unsigned int* cores, unsigned int* core_disable_map,
    int* enabled_cores_count, unsigned int zen_version) {
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
    *core_disable_map = 0;
    if (ccd_enable_map & 0x01) {
        if (smu_read_smn_addr(&obj, core_disable_map_addr, &core_disable_map_tmp) != SMU_Return_OK) {
            perror("Failed to read disabled core fuse");
            exit(-1);
        }
        *core_disable_map |= core_disable_map_tmp & 0xff;
    }
    if (ccd_enable_map & 0x02) {
        if (smu_read_smn_addr(&obj, core_disable_map_addr|0x2000000, &core_disable_map_tmp) != SMU_Return_OK) {
            perror("Failed to read disabled core fuse");
            exit(-1);
        }
        *core_disable_map |= (core_disable_map_tmp & 0xff)<<8;
    }


    if (!threads_per_core)
        *cores = logical_cores;
    else
        *cores = logical_cores / threads_per_core;

    switch(zen_version)
    {
        case 3:
            //Zen3 does not have CCXs anymore. They now have 8 cores per CCD
            //each with the same access to each other and the common L3 cache.
            *ccxs = 0;
            *ccds = count_set_bits(ccd_enable_map);
            *cores_per_ccx = 8 - count_set_bits(*core_disable_map & 0xff);
            *enabled_cores_count = 8*(*ccds) - count_set_bits(*core_disable_map);
            break;

        case 2:
        default:
            *cores_per_ccx = (8 - count_set_bits(*core_disable_map & 0xff)) / 2;
            *ccds = count_set_bits(ccd_enable_map);
            *ccxs = *cores == *cores_per_ccx ? 1 : *ccds * 2;
            *enabled_cores_count = 8*(*ccds) - count_set_bits(~*core_disable_map);
            break;
    }
}

void print_line(const char* label, const char* value_format, ...) {
    static char buffer[1024];
    va_list list;

    va_start(list, value_format);
    vsnprintf(buffer, sizeof(buffer), value_format, list);
    va_end(list);

    fprintf(stdout, "│ %45s │ %46s │\n", label, buffer);
}

//Helper to access the PM Table elements. If an element doesn't exist in the
//current PM Table version, it's pointer is set to 0. This helper returns
//NAN for not available fields.
#define pmta(elem) ((pmt.elem)?(*pmt.elem):NAN)

void start_pm_monitor(int force) {
    float core_voltage, core_frequency, package_sleep_time, core_sleep_time, edc_value, average_voltage;
    float peak_core_frequency, peak_core_temp, peak_core_voltage;
    float total_core_voltage, total_core_power, total_usage, total_core_CC6;
    const char* name, *codename, *smu_fw_ver;
    unsigned int cores, ccds, ccxs, cores_per_ccx, core_disable_map, enabled_cores_count=0;
    unsigned int if_ver, i;
    int core_disabled, core_renumber;
    unsigned char *pm_buf;
    pm_table pmt;

    if (!smu_pm_tables_supported(&obj)) {
        fprintf(stderr, "PM Tables are not supported on this platform.\n");
        exit(0);
    }

    pm_buf = calloc(obj.pm_table_size, sizeof(unsigned char));
    if (!pm_buf) {
        fprintf(stderr, "Could not allocate memory for the PM Table.\n");
        exit(0);
    }

    //Initialize pmt to 0. This also sets all pointers to 0, which signifies non-existiting fields.
    //Access via pmta(...) will check if pointer is 0 before trying to access the value.
    memset(&pmt, 0, sizeof(pmt));

    //Select matching PM Table
    //ToDo: implement "force" table version
    switch(obj.pm_table_version) {
        case 0x380804: pm_table_0x380804(&pmt, pm_buf); break;
        default:
            fprintf(stderr, "This PM Table version is currently not supported.\n");
            exit(0);
            break;
    }
    //Prevent illegal memory access
    if (obj.pm_table_size < pmt.min_size) {
        fprintf(stderr, "Selected PM Table is larger than the PM Table returned by the SMU.\n");
        exit(0);
    }
    //Maximum core count. Just to be safe. Will be overwritten by get_processor_topology(...).
    enabled_cores_count = pmt.max_cores;

    name        = get_processor_name();
    codename    = smu_codename_to_str(&obj);
    smu_fw_ver  = smu_get_fw_version(&obj);

    get_processor_topology(&ccds, &ccxs, &cores_per_ccx, &cores, &core_disable_map, &enabled_cores_count, pmt.zen_version);

    switch (obj.smu_if_version) {
        case IF_VERSION_9:  if_ver =  9; break;
        case IF_VERSION_10: if_ver = 10; break;
        case IF_VERSION_11: if_ver = 11; break;
        case IF_VERSION_12: if_ver = 12; break;
        case IF_VERSION_13: if_ver = 13; break;
        default:            if_ver =  0; break;
    }

    while(1) {
        if (smu_read_pm_table(&obj, pm_buf, obj.pm_table_size) != SMU_Return_OK)
            continue;

        fprintf(stdout, "\e[1;1H\e[2J"); //Move cursor to (1,1); Clear entire screen

        fprintf(stdout, "╭───────────────────────────────────────────────┬────────────────────────────────────────────────╮\n");
        print_line("CPU Model", name);
        print_line("Processor Code Name", codename);
        print_line("Cores", "%d", cores);
        print_line("Core CCDs", "%d", ccds);
        if (pmt.zen_version!=3) {
            print_line("Core CCXs", "%d", ccxs);
            print_line("Cores Per CCX", "%d", cores_per_ccx);
        }
        else
            print_line("Cores Per CCD", "%d", cores_per_ccx); //Zen3 does not have CCXs anymore
        print_line("SMU FW Version", "v%s", smu_fw_ver);
        print_line("MP1 IF Version", "v%d", if_ver);
        fprintf(stdout, "╰───────────────────────────────────────────────┴────────────────────────────────────────────────╯\n");


        peak_core_frequency = peak_core_temp = peak_core_voltage = 0;
        total_core_voltage = total_core_power = total_usage = total_core_CC6 = 0;
        core_renumber = 0;

        package_sleep_time = pmta(PC6) / 100.f;
        average_voltage = (pmta(CPU_TELEMETRY_VOLTAGE) - (0.2 * package_sleep_time)) / (1.0 - package_sleep_time);

        fprintf(stdout, "╭─────────┬────────────┬──────────┬─────────┬──────────┬─────────────┬─────────────┬─────────────╮\n");
        for (i = 0; i < pmt.max_cores; i++) {
            core_disabled = (core_disable_map >> i)&0x01;
            core_frequency = pmta(CORE_FREQEFF[i]) * 1000.f;

            // "Real core frequency" -- excluding gating
            if (pmta(CORE_FREQ[i]) != 0.f) {
                core_sleep_time = pmta(CORE_CC6[i]) / 100.f;
                core_voltage = ((1.0 - core_sleep_time) * average_voltage) + (0.2 * core_sleep_time);
            }

            if (core_disabled) {
                if (show_disabled_cores)
                      fprintf(stdout,
                          "│ %*s %d │   Disabled | %6.3f W | %5.3f V | %6.2f C | C0: %5.1f %% | C1: %5.1f %% | C6: %5.1f %% │\n",
                        (i<10)+4, "Core", i,
                           pmta(CORE_POWER[i]), core_voltage, pmta(CORE_TEMP[i]),
                           pmta(CORE_C0[i]), pmta(CORE_CC1[i]), pmta(CORE_CC6[i]));
            }
            else if (pmta(CORE_C0[i]) >= 6.f) {
                // AMD denotes a sleeping core as having spent less than 6% of the time in C0.
                // Source: Ryzen Master
                   fprintf(stdout,
                       "│ %*s %d │   %4.f MHz | %6.3f W | %5.3f V | %6.2f C | C0: %5.1f %% | C1: %5.1f %% | C6: %5.1f %% │\n",
                    (core_renumber<10)+4, "Core", core_renumber,
                    core_frequency, pmta(CORE_POWER[i]), core_voltage, pmta(CORE_TEMP[i]),
                       pmta(CORE_C0[i]), pmta(CORE_CC1[i]), pmta(CORE_CC6[i]));
               }
               else {
                   fprintf(stdout,
                       "│ %*s %d │   Sleeping | %6.3f W | %5.3f V | %6.2f C | C0: %5.1f %% | C1: %5.1f %% | C6: %5.1f %% │\n",
                    (core_renumber<10)+4, "Core", core_renumber,
                       pmta(CORE_POWER[i]), core_voltage, pmta(CORE_TEMP[i]),
                       pmta(CORE_C0[i]), pmta(CORE_CC1[i]), pmta(CORE_CC6[i]));
            }

            //Don't confuse people by numbering cores that are disabled and hence not shown on 6 | 12 core CPUs
            //(which actually have 8 | 16 cores)
            if (show_disabled_cores || !core_disabled) core_renumber++;

            //Statistics
            if (!core_disabled) {
                if (peak_core_frequency < core_frequency) peak_core_frequency = core_frequency;
                if (peak_core_temp < pmta(CORE_TEMP[i])) peak_core_temp = pmta(CORE_TEMP[i]);
                if (peak_core_voltage < core_voltage) peak_core_voltage = core_voltage;
                total_core_voltage += core_voltage;
                total_core_power += pmta(CORE_POWER[i]);
                total_usage += pmta(CORE_C0[i]);
                total_core_CC6 += pmta(CORE_CC6[i]);
            }
        }

        fprintf(stdout, "╰─────────┴────────────┴──────────┴─────────┴──────────┴─────────────┴─────────────┴─────────────╯\n");

        fprintf(stdout, "╭── Core Statistics (Calculated) ───────────────┬────────────────────────────────────────────────╮\n");
        print_line("Highest Effective Core Frequency", "%8.0f MHz", peak_core_frequency);
        print_line("Highest Core Temperature", "%8.2f C", peak_core_temp);
        print_line("Highest Core Voltage", "%8.3f V", peak_core_voltage);
        print_line("Average Core Voltage", "%5.3f V", total_core_voltage/enabled_cores_count);
        print_line("Average Core CC6", "%6.2f %%", total_core_CC6/enabled_cores_count);
        print_line("Total Core Power Sum", "%7.4f W", total_core_power);

        fprintf(stdout, "├── Reported by SMU ────────────────────────────┼────────────────────────────────────────────────┤\n");
        //print_line("Package Power", "%8.3f W", pmta(SOCKET_POWER)); //Is listed below in power section
        print_line("Peak Core Voltage", "%5.3f V", pmta(CPU_TELEMETRY_VOLTAGE));
        print_line("Package CC6", "%6.2f %%", pmta(PC6));
        fprintf(stdout, "╰───────────────────────────────────────────────┴────────────────────────────────────────────────╯\n");

        fprintf(stdout, "╭── Electrical & Thermal Constraints ───────────┬────────────────────────────────────────────────╮\n");
        edc_value = pmta(EDC_VALUE) * (total_usage / cores / 100);
        if (edc_value < pmta(TDC_VALUE)) edc_value = pmta(TDC_VALUE);

        print_line("Peak Temperature", "%8.2f C", pmta(PEAK_TEMP));
        print_line("SoC Temperature", "%8.2f C", pmta(SOC_TEMP));
        //print_line("Core Power", "%8.4f W", pmta(VDDCR_CPU_POWER));

        print_line("Voltage from Core VRM", "%7.3f V | %7.3f V | %8.2f %%", pmta(VID_VALUE), pmta(VID_LIMIT), (pmta(VID_VALUE) / pmta(VID_LIMIT) * 100));
        print_line("PPT", "%7.3f W | %7.f W | %8.2f %%", pmta(PPT_VALUE), pmta(PPT_LIMIT), (pmta(PPT_VALUE) / pmta(PPT_LIMIT) * 100));
        print_line("TDC Value", "%7.3f A | %7.f A | %8.2f %%", pmta(TDC_VALUE), pmta(TDC_LIMIT), (pmta(TDC_VALUE) / pmta(TDC_LIMIT) * 100));
        print_line("TDC Actual", "%7.3f A | %7.f A | %8.2f %%", pmta(TDC_ACTUAL), pmta(TDC_LIMIT), (pmta(TDC_ACTUAL) / pmta(TDC_LIMIT) * 100));
        print_line("EDC", "%7.3f A | %7.f A | %8.2f %%", edc_value, pmta(EDC_LIMIT), (edc_value / pmta(EDC_LIMIT) * 100));
        print_line("THM", "%7.2f C | %7.f C | %8.2f %%", pmta(THM_VALUE), pmta(THM_LIMIT), (pmta(THM_VALUE) / pmta(THM_LIMIT) * 100));
        print_line("FIT", "%7.f   | %7.f   | %8.2f %%", pmta(FIT_VALUE), pmta(FIT_LIMIT), (pmta(FIT_VALUE) / pmta(FIT_LIMIT)) * 100.f);
        fprintf(stdout, "╰───────────────────────────────────────────────┴────────────────────────────────────────────────╯\n");

        fprintf(stdout, "╭── Memory Interface ───────────────────────────┬────────────────────────────────────────────────╮\n");
        print_line("Coupled Mode", "%8s", pmta(UCLK_FREQ) == pmta(MEMCLK_FREQ) ? "ON" : "OFF");
        print_line("Fabric Clock (Average)", "%5.f MHz", pmta(FCLK_FREQ_EFF));
        print_line("Fabric Clock", "%5.f MHz", pmta(FCLK_FREQ));
        print_line("Uncore Clock", "%5.f MHz", pmta(UCLK_FREQ));
        print_line("Memory Clock", "%5.f MHz", pmta(MEMCLK_FREQ));
        //print_line("VDDCR_Mem", "%7.4f W", pmta(VDDIO_MEM_POWER)); //Is listed below in power section
        //print_line("VDDCR_SoC", "%7.4f V", pmta(SOC_SET_VOLTAGE)); //Might be the default voltage, not the actually set one
        print_line("cLDO_VDDM", "%7.4f V", pmta(V_VDDM));
        print_line("cLDO_VDDP", "%7.4f V", pmta(V_VDDP));
        print_line("cLDO_VDDG", "%7.4f V", pmta(V_VDDG));
        fprintf(stdout, "╰───────────────────────────────────────────────┴────────────────────────────────────────────────╯\n");

        fprintf(stdout, "╭── Power Consumption ──────────────────────────┬────────────────────────────────────────────────╮\n");
        //These powers are drawn via VDDCR_SOC and VDDCR_CPU and thus are pulled from the CPU power connector of the mainboard
        print_line("Total Core Power Sum", "%7.4f W", total_core_power);
        //print_line("VDDCR_CPU Power", "%7.4f W", pmta(VDDCR_CPU_POWER)); //This value doesn't correlate with what the cores
                                                                          //report, nor woth what is actually consumed. but is
                                                                          //the value HWiNFO shows.
        print_line("VDDCR_SOC Power", "%7.4f W", pmta(VDDCR_SOC_POWER));
        print_line("GMI2_VDDG Power", "%7.4f W", pmta(GMI2_VDDG_POWER));
        print_line("L3 Logic Power", "%7.3f W + %7.4f W = %7.4f W", pmta(L3_LOGIC_POWER[0]), pmta(L3_LOGIC_POWER[1]),
                pmta(L3_LOGIC_POWER[0])+pmta(L3_LOGIC_POWER[1]));
        print_line("L3 VDDM Power", "%7.3f W + %7.4f W = %7.4f W", pmta(L3_VDDM_POWER[0]), pmta(L3_VDDM_POWER[1]),
                pmta(L3_VDDM_POWER[0])+pmta(L3_VDDM_POWER[1]));

        //These powers are supplied by other power lines to the CPU and are drawn from the 24 pin ATX connector on most boards
        print_line("","");
        print_line("VDDIO_MEM Power", "%7.4f W", pmta(VDDIO_MEM_POWER));
        print_line("IOD_VDDIO_MEM Power", "%7.4f W", pmta(IOD_VDDIO_MEM_POWER));
        print_line("DDR_VDDP Power", "%7.4f W", pmta(DDR_VDDP_POWER));
        print_line("VDD18 Power", "%7.4f W", pmta(VDD18_POWER)); //Same as pmta(IO_VDD18_POWER)

        //The sum is the thermal output of the whole package. Yes, this is higher than PPT and SOCKET_POWER.
        //Confirmed by measuring the actual current draw on the mainboard.
        print_line("","");
        print_line("Calculated Thermal Output", "%7.4f W", total_core_power + pmta(VDDCR_SOC_POWER) + pmta(GMI2_VDDG_POWER) 
                + pmta(L3_LOGIC_POWER[0]) + pmta(L3_LOGIC_POWER[1]) + pmta(L3_VDDM_POWER[0]) + pmta(L3_VDDM_POWER[1])
                + pmta(VDDIO_MEM_POWER) + pmta(IOD_VDDIO_MEM_POWER) + pmta(DDR_VDDP_POWER) + pmta(VDD18_POWER));

        fprintf(stdout, "├── Additional Reports ─────────────────────────┼────────────────────────────────────────────────┤\n");
        //print_line("ROC_POWER", "%7.4f",pmta(ROC_POWER));
        print_line("SoC Power (SVI2)", "%8.3f V | %7.3f A | %8.3f W", pmta(SOC_TELEMETRY_VOLTAGE), pmta(SOC_TELEMETRY_CURRENT), pmta(SOC_TELEMETRY_POWER));
        print_line("Core Power (SVI2)", "%8.3f V | %7.3f A | %8.3f W", pmta(CPU_TELEMETRY_VOLTAGE), pmta(CPU_TELEMETRY_CURRENT), pmta(CPU_TELEMETRY_POWER));
        print_line("Core Power (SMU)", "%7.3f W", pmta(VDDCR_CPU_POWER));
        print_line("Socket Power (SMU)", "%7.4f W", pmta(SOCKET_POWER));
        print_line("Package Power (SMU)", "%7.4f W", pmta(PACKAGE_POWER));
        fprintf(stdout, "╰───────────────────────────────────────────────┴────────────────────────────────────────────────╯\n");


        // Hide Cursor
        fprintf(stdout, "\e[?25l");

        fflush(stdout);
        sleep(update_time_s);
    }
}

void print_version() {
    fprintf(stdout, "SMU Monitor " PROGRAM_VERSION "\n");
    exit(0);
}

void show_help(char* program) {
    fprintf(stdout,
        "SMU Monitor " PROGRAM_VERSION "\n\n"

        "Usage: %s <option(s)>\n\n"

        "Options:\n"
            "\t-h          - Show this help screen.\n"
            "\t-v          - Show program version.\n"
            "\t-m          - Print DRAM Timings and exit.\n"
//            "\t-f          - Force PM table monitoring even if the PM table version is not supported.\n"
            "\t-d          - Show disabled cores.\n"
            "\t-u<seconds> - Update the monitoring only after this number of second(s) have passed. Defaults to 1.\n",
        program
    );
}

void parse_args(int argc, char** argv) {
    int c = 0, force, core;

    core = 0;
    force = 0;

    while ((c = getopt(argc, argv, "vmd::fu:h")) != -1) {
        switch (c) {
            case 'v':
                print_version();
                exit(0);
            case 'm':
                print_memory_timings();
                exit(0);
            case 'd':
                if (optarg)
                    show_disabled_cores = atoi(optarg);
                else
                    show_disabled_cores = 1;
                break;
            case 'f':
                force = 1;
                break;
            case 'u':
                update_time_s = atoi(optarg);
                break;
            case 'h':
                show_help(argv[0]);
                exit(0);
            case '?':
                exit(0);
            default:
                break;
        }
    }

    start_pm_monitor(force);
}

void signal_interrupt(int sig) {
    switch (sig) {
        case SIGINT:
        case SIGABRT:
        case SIGTERM:
            // Re-enable the cursor.
            fprintf(stdout, "\e[?25h");
            exit(0);
        default:
            break;
    }
}

int main(int argc, char** argv) {
    smu_return_val ret;

    if ((signal(SIGABRT, signal_interrupt) == SIG_ERR) ||
        (signal(SIGTERM, signal_interrupt) == SIG_ERR) ||
        (signal(SIGINT, signal_interrupt) == SIG_ERR)) {
        fprintf(stderr, "Can't set up signal hooks.\n");
        exit(-1);
    }

    if (getuid() != 0 && geteuid() != 0) {
        fprintf(stderr, "Program must be run as root.\n");
        exit(-2);
    }

    ret = smu_init(&obj);
    if (ret != SMU_Return_OK) {
        fprintf(stderr, "%s\n", smu_return_to_str(ret));
        exit(-2);
    }

    parse_args(argc, argv);

    return 0;
}
