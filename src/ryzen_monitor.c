/**
 * Ryzen SMU Userspace Sensor Monitor
 * Copyright (C) 2020
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

#define PROGRAM_VERSION                 "1.0"

#define READ_SMN_V1(offs) { if (smu_read_smn_addr(&obj, offs + offset, &value1) != SMU_Return_OK) goto _READ_ERROR; }
#define READ_SMN_V2(offs) { if (smu_read_smn_addr(&obj, offs + offset, &value2) != SMU_Return_OK) goto _READ_ERROR; }

// Ryzen 5900X
#define PM_TABLE_VERSION 0x380804
#define PM_TABLE_MAX_CORES 16
#define PM_TABLE_ZEN_VERSION 3
/* Legend for notes in comments:
 * o = ok. I'm confident this is the right value.
 * s = static. Does not change unter load.
 * z = always zero
 * c = changes under load. Don't know if the value is correct.
 */
typedef struct {
    float PPT_LIMIT; //o
    float PPT_VALUE; //o
    float TDC_LIMIT; //o
    float TDC_VALUE; //o
    float THM_LIMIT; //o
    float THM_VALUE; //o
    float FIT_LIMIT; //o
    float FIT_VALUE; //o
    float EDC_LIMIT; //o
    float EDC_VALUE; //o
    float VID_LIMIT; //o
    float VID_VALUE; //o
    float PPT_WC;
    float PPT_ACTUAL; //o
    float TDC_WC;
    float TDC_ACTUAL; //o
    float THM_WC;
    float THM_ACTUAL; //o
    float FIT_WC;
    float FIT_ACTUAL; //o
    float EDC_WC;
    float EDC_ACTUAL; //o
    float VID_WC;
    float VID_ACTUAL; //o
    float VDDCR_CPU_POWER; //o
    float VDDCR_SOC_POWER; //o
    float VDDIO_MEM_POWER; //o
    float VDD18_POWER; //o
    float ROC_POWER; //s
    float SOCKET_POWER; //o
    float PPT_FREQUENCY;
    float TDC_FREQUENCY;
    float THM_FREQUENCY;
    float PROCHOT_FREQUENCY;
    float VOLTAGE_FREQUENCY;
    float CCA_FREQUENCY;
    float FIT_VOLTAGE;
    float FIT_PRE_VOLTAGE;
    float LATCHUP_VOLTAGE;
    float CPU_SET_VOLTAGE; //os
    float CPU_TELEMETRY_VOLTAGE;
    float CPU_TELEMETRY_VOLTAGE2; 
    float CPU_TELEMETRY_CURRENT; //o
    float CPU_TELEMETRY_POWER; //o
    float SOC_SET_VOLTAGE; //os
    float SOC_TELEMETRY_VOLTAGE; //o
    float SOC_TELEMETRY_CURRENT; //o
    float SOC_TELEMETRY_POWER; //o
    float FCLK_FREQ; //o
    float FCLK_FREQ_EFF; //o
    float UCLK_FREQ; //o
    float MEMCLK_FREQ; //o
    float FCLK_DRAM_SETPOINT;
    float FCLK_DRAM_BUSY;
    float FCLK_GMI_SETPOINT;
    float FCLK_GMI_BUSY;
    float FCLK_IOHC_SETPOINT;
    float FCLK_IOHC_BUSY;
    float FCLK_XGMI_SETPOINT;
    float FCLK_XGMI_BUSY;
    float CCM_READS;
    float CCM_WRITES;
    float IOMS;
    float XGMI;
    float CS_UMC_READS;
    float CS_UMC_WRITES;
    float unk0108[4];
    float FCLK_RESIDENCY[4];
    float FCLK_FREQ_TABLE[4];
    float UCLK_FREQ_TABLE[4];
    float MEMCLK_FREQ_TABLE[4];
    float FCLK_VOLTAGE[4];
    float LCLK_SETPOINT_0;
    float LCLK_BUSY_0;
    float LCLK_unk1_0;
    float LCLK_unk2_0;
    float LCLK_FREQ_0;
    float LCLK_FREQ_EFF_0;
    float LCLK_MAX_DPM_0;
    float LCLK_MIN_DPM_0;
    float LCLK_SETPOINT_1;
    float LCLK_BUSY_1;
    float LCLK_unk1_1;
    float LCLK_unk2_1;
    float LCLK_FREQ_1;
    float LCLK_FREQ_EFF_1;
    float LCLK_MAX_DPM_1;
    float LCLK_MIN_DPM_1;
    float LCLK_SETPOINT_2;
    float LCLK_BUSY_2;
    float LCLK_unk1_2;
    float LCLK_unk2_2;
    float LCLK_FREQ_2;
    float LCLK_FREQ_EFF_2;
    float LCLK_MAX_DPM_2;
    float LCLK_MIN_DPM_2;
    float LCLK_SETPOINT_3;
    float LCLK_BUSY_3;
    float LCLK_unk1_3;
    float LCLK_unk2_3;
    float LCLK_FREQ_3;
    float LCLK_FREQ_EFF_3;
    float LCLK_MAX_DPM_3;
    float LCLK_MIN_DPM_3;
    float XGMI_SETPOINT;
    float XGMI_BUSY;
    float XGMI_LANE_WIDTH;
    float XGMI_DATA_RATE;
    float SOC_POWER; //x
    float SOC_TEMP; //o?
    float DDR_VDDP_POWER;
    float DDR_VDDIO_MEM_POWER;
    float GMI2_VDDG_POWER;
    float IO_VDDCR_SOC_POWER;
    float IOD_VDDIO_MEM_POWER;
    float IO_VDD18_POWER; 
    float TDP;
    float DETERMINISM;
    float V_VDDM;
    float V_VDDP;
    float V_VDDG;
    float V_unk1;
    float PEAK_TEMP; //o
    float PEAK_VOLTAGE; //o
    float unk_power; //maybe power?
    float AVG_CORE_COUNT;
    float CCLK_LIMIT; //o GHz
    float MAX_VOLTAGE; //o
    float DC_BTC;
    float package_power; //?
    float unk_0250;
    float unk_0254;
    float CSTATE_BOOST;
    float PROCHOT;
    float PC6;
    float PWM;
    float unk_0268;
    float clk_026c;
    float clk_0270;
    float SOCCLK;
    float SHUBCLK;
    float MP0CLK;
    float MP1CLK;
    float MP5CLK;
    float SMNCLK;
    float TWIXCLK;
    float clk_0290;
    float WAFLCLK; //0 in https://chart-studio.plotly.com/~brettdram/16/
    float DPM_BUSY;
    float MP1_BUSY;
    float MP5_BUSY;
    float CORE_POWER[16]; //0x02A4-0x02E0 //o
    float CORE_VOLTAGE[16]; //0x02E4-0x0320 //o
    float CORE_TEMP[16]; //0x0324-0x0360 //o
    float CORE_FIT[16]; //0x0364-0x03A0 //o
    float CORE_IDDMAX[16]; //0x03A4-0x03E0 //o
    float CORE_FREQ[16]; //0x03E4-0x0420 //o
    float CORE_FREQEFF[16]; //0x0424-0x0460
    float CORE_C0[16]; //0x0464-0x04A0 //o
    float CORE_CC1[16]; //0x04A4-0x04E0 //o
    float CORE_CC6[16]; //0x04E4-0x0520 //o
    float CORE_CKS_FDD[16]; //0x0524-0x0560
    float CORE_CI_FDD[16]; //0x0564-0x05A0
    float CORE_IRM[16]; //0x05A4-0x05E0
    float CORE_PSTATE[16]; //0x05E4-0x0620 //o
    float CORE_FREQ_LIM_MAX[16]; //0x0624-0x0660 //o(s)
    float CORE_FREQ_LIM_MIN[16]; //0x0664-0x06A0 //o(s)
    float CORE_CPPC_MAX[16]; //0x06A4-0x06A4 //c
    float CORE_CPPC_MIN[16]; //0x06E4-0x0720 //c
    float unk_0724[16]; //0x0724-0x0760 //zs
    float CORE_SC_LIMIT[16]; //0x0764-0x07A0 //c
    float CORE_SC_CAC[16]; //0x07A4-0x07E0 //c
    float CORE_SC_RESIDENCY[16]; //0x07E4-0x0820 //c
    float L3_LOGIC_POWER[2]; //o 3W
    float L3_VDDM_POWER[2]; //o 0.5W
    float L3_TEMP[2]; //o
    float L3_FIT[2]; //o
    float L3_IDDMAX[2]; //o 28A
    float L3_FREQ[2]; //o
    float L3_CKS_FDD[2]; //c
    float L3_CCA_THRESHOLD[2]; //c
    float L3_CCA_CAC[2]; //s 2048
    float L3_CCA_ACTIVATION[2]; //c
    float L3_EDC_LIMIT[2]; //z
    float L3_EDC_CAC[2]; //c
    float L3_EDC_RESIDENCY[2]; //c
    //[2] //z
    //[2] //(s)(1)
    //[2] //c 21->31
} pm_table_0x380804, *ppm_table_0x380804;

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

void start_pm_monitor(int force) {
    float core_voltage, core_frequency, package_sleep_time, core_sleep_time, edc_value, average_voltage;
    float peak_core_frequency, peak_core_temp, peak_core_voltage,
          total_core_voltage, total_core_power, total_usage, total_core_CC6;

    const char* name, *codename, *smu_fw_ver;
    unsigned int max_cores, zen_version;
    unsigned int cores, ccds, ccxs, cores_per_ccx, core_disable_map;
    unsigned int if_ver, i;
    int core_disabled, core_renumber;
    ppm_table_0x380804 pmt;
    unsigned char *pm_buf;
    int enabled_cores_count=0;

    if (!smu_pm_tables_supported(&obj)) {
        fprintf(stderr, "PM Tables are not supported on this platform.\n");
        exit(0);
    }

    //Ok, ideally we should support multiple table version. I just don't yet know how.
    if (!force && obj.pm_table_version != PM_TABLE_VERSION) {
        fprintf(stderr, "PM Table version is not currently suppported. Run with \"-pf\" flag to ignore this.\n");
        exit(0);
    }
    max_cores = PM_TABLE_MAX_CORES;
    zen_version = PM_TABLE_ZEN_VERSION;
    enabled_cores_count = max_cores; //Just to be safe. Will be overwritten by get_processor_topology().

    name        = get_processor_name();
    codename    = smu_codename_to_str(&obj);
    smu_fw_ver  = smu_get_fw_version(&obj);

    get_processor_topology(&ccds, &ccxs, &cores_per_ccx, &cores, &core_disable_map, &enabled_cores_count, zen_version);

    pm_buf = calloc(obj.pm_table_size, sizeof(unsigned char));
    pmt = (ppm_table_0x380804)pm_buf;

    switch (obj.smu_if_version) {
        case IF_VERSION_9:
            if_ver = 9;
            break;
        case IF_VERSION_10:
            if_ver = 10;
            break;
        case IF_VERSION_11:
            if_ver = 11;
            break;
        case IF_VERSION_12:
            if_ver = 12;
            break;
        case IF_VERSION_13:
            if_ver = 13;
            break;
        default:
            if_ver = 0;
            break;
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
        if (zen_version!=3) {
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

        package_sleep_time = pmt->PC6 / 100.f;
        average_voltage = (pmt->CPU_TELEMETRY_VOLTAGE - (0.2 * package_sleep_time)) / (1.0 - package_sleep_time);

        fprintf(stdout, "╭─────────┬────────────┬──────────┬─────────┬──────────┬─────────────┬─────────────┬─────────────╮\n");
        for (i = 0; i < max_cores; i++) {
            core_disabled = (core_disable_map >> i)&0x01;
            core_frequency = pmt->CORE_FREQEFF[i] * 1000.f;

            // "Real core frequency" -- excluding gating
            if (pmt->CORE_FREQ[i] != 0.f) {
                core_sleep_time = pmt->CORE_CC6[i] / 100.f;
                core_voltage = ((1.0 - core_sleep_time) * average_voltage) + (0.2 * core_sleep_time);
            }

            if (core_disabled) {
                if (show_disabled_cores)
                      fprintf(stdout,
                          "│ %*s %d │   Disabled | %6.3f W | %5.3f V | %6.2f C | C0: %5.1f %% | C1: %5.1f %% | C6: %5.1f %% │\n",
                        (i<10)+4, "Core", i,
                           pmt->CORE_POWER[i], core_voltage, pmt->CORE_TEMP[i],
                           pmt->CORE_C0[i], pmt->CORE_CC1[i], pmt->CORE_CC6[i]);
            }
            else if (pmt->CORE_C0[i] >= 6.f) {
                // AMD denotes a sleeping core as having spent less than 6% of the time in C0.
                // Source: Ryzen Master
                   fprintf(stdout,
                       "│ %*s %d │   %4.f MHz | %6.3f W | %5.3f V | %6.2f C | C0: %5.1f %% | C1: %5.1f %% | C6: %5.1f %% │\n",
                    (core_renumber<10)+4, "Core", core_renumber,
                    core_frequency, pmt->CORE_POWER[i], core_voltage, pmt->CORE_TEMP[i],
                       pmt->CORE_C0[i], pmt->CORE_CC1[i], pmt->CORE_CC6[i]);
               }
               else {
                   fprintf(stdout,
                       "│ %*s %d │   Sleeping | %6.3f W | %5.3f V | %6.2f C | C0: %5.1f %% | C1: %5.1f %% | C6: %5.1f %% │\n",
                    (core_renumber<10)+4, "Core", core_renumber,
                       pmt->CORE_POWER[i], core_voltage, pmt->CORE_TEMP[i],
                       pmt->CORE_C0[i], pmt->CORE_CC1[i], pmt->CORE_CC6[i]);
            }

            //Don't confuse people by numbering cores that are disabled and hence n't shown on 6 / 12 core CPUs
            //(which actually have 8 / 16 cores)
            if (show_disabled_cores || !core_disabled) core_renumber++;

            //Statistics
            if (!core_disabled) {
                if (peak_core_frequency < core_frequency) peak_core_frequency = core_frequency;
                if (peak_core_temp < pmt->CORE_TEMP[i]) peak_core_temp = pmt->CORE_TEMP[i];
                if (peak_core_voltage < core_voltage) peak_core_voltage = core_voltage;
                total_core_voltage += core_voltage;
                total_core_power += pmt->CORE_POWER[i];
                total_usage += pmt->CORE_C0[i];
                total_core_CC6 += pmt->CORE_CC6[i];
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
        //print_line("Package Power", "%8.3f W", pmt->SOCKET_POWER); //Is listed below in power section
        print_line("Peak Core Voltage", "%5.3f V", pmt->CPU_TELEMETRY_VOLTAGE);
        print_line("Package CC6", "%6.2f %%", pmt->PC6);
        fprintf(stdout, "╰───────────────────────────────────────────────┴────────────────────────────────────────────────╯\n");

        fprintf(stdout, "╭── Electrical & Thermal Constraints ───────────┬────────────────────────────────────────────────╮\n");
        edc_value = pmt->EDC_VALUE * (total_usage / cores / 100);
        if (edc_value < pmt->TDC_VALUE) edc_value = pmt->TDC_VALUE;

        print_line("Peak Temperature", "%8.2f C", pmt->PEAK_TEMP);
        print_line("SoC Temperature", "%8.2f C", pmt->SOC_TEMP);
        //print_line("Core Power", "%8.4f W", pmt->VDDCR_CPU_POWER);

        print_line("Voltage from Core VRM", "%7.3f V | %7.3f V | %8.2f %%", pmt->VID_VALUE, pmt->VID_LIMIT, (pmt->VID_VALUE / pmt->VID_LIMIT * 100));
        print_line("PPT", "%7.3f W | %7.f W | %8.2f %%", pmt->PPT_VALUE, pmt->PPT_LIMIT, (pmt->PPT_VALUE / pmt->PPT_LIMIT * 100));
        print_line("TDC Value", "%7.3f A | %7.f A | %8.2f %%", pmt->TDC_VALUE, pmt->TDC_LIMIT, (pmt->TDC_VALUE / pmt->TDC_LIMIT * 100));
        print_line("TDC Actual", "%7.3f A | %7.f A | %8.2f %%", pmt->TDC_ACTUAL, pmt->TDC_LIMIT, (pmt->TDC_ACTUAL / pmt->TDC_LIMIT * 100));
        print_line("EDC", "%7.3f A | %7.f A | %8.2f %%", edc_value, pmt->EDC_LIMIT, (edc_value / pmt->EDC_LIMIT * 100));
        print_line("THM", "%7.2f C | %7.f C | %8.2f %%", pmt->THM_VALUE, pmt->THM_LIMIT, (pmt->THM_VALUE / pmt->THM_LIMIT * 100));
        print_line("FIT", "%7.f   | %7.f   | %8.2f %%", pmt->FIT_VALUE, pmt->FIT_LIMIT, (pmt->FIT_VALUE / pmt->FIT_LIMIT) * 100.f);
        fprintf(stdout, "╰───────────────────────────────────────────────┴────────────────────────────────────────────────╯\n");

        fprintf(stdout, "╭── Memory Interface ───────────────────────────┬────────────────────────────────────────────────╮\n");
        print_line("Coupled Mode", "%8s", pmt->UCLK_FREQ == pmt->MEMCLK_FREQ ? "ON" : "OFF");
        print_line("Fabric Clock (Average)", "%5.f MHz", pmt->FCLK_FREQ_EFF);
        print_line("Fabric Clock", "%5.f MHz", pmt->FCLK_FREQ);
        print_line("Uncore Clock", "%5.f MHz", pmt->UCLK_FREQ);
        print_line("Memory Clock", "%5.f MHz", pmt->MEMCLK_FREQ);
        //print_line("VDDCR_Mem", "%7.4f W", pmt->VDDIO_MEM_POWER); //Is listed below in power section
        //print_line("VDDCR_SoC", "%7.4f V", pmt->SOC_SET_VOLTAGE); //Might be the default voltage, not the actually set one
        print_line("cLDO_VDDM", "%7.4f V", pmt->V_VDDM);
        print_line("cLDO_VDDP", "%7.4f V", pmt->V_VDDP);
        print_line("cLDO_VDDG", "%7.4f V", pmt->V_VDDG);
        fprintf(stdout, "╰───────────────────────────────────────────────┴────────────────────────────────────────────────╯\n");

        fprintf(stdout, "╭── Power Consumption ──────────────────────────┬────────────────────────────────────────────────╮\n");
        //These powers are drawn via VDDCR_SOC and VDDCR_CPU and thus are pulled from the CPU power connector of the mainboard
        print_line("Total Core Power Sum", "%7.4f W", total_core_power);
        //print_line("VDDCR_CPU Power", "%7.4f W", pmt->VDDCR_CPU_POWER); //This value doesn't correlate with what the cores
                                                                          //report, nor woth what is actually consumed. but is
                                                                          //the value HWiNFO shows.
        print_line("VDDCR_SOC Power", "%7.4f W", pmt->VDDCR_SOC_POWER);
        print_line("GMI2_VDDG Power", "%7.4f W", pmt->GMI2_VDDG_POWER);
        print_line("L3 Logic Power", "%7.3f W + %7.4f W = %7.4f W", pmt->L3_LOGIC_POWER[0], pmt->L3_LOGIC_POWER[1],
                pmt->L3_LOGIC_POWER[0]+pmt->L3_LOGIC_POWER[1]);
        print_line("L3 VDDM Power", "%7.3f W + %7.4f W = %7.4f W", pmt->L3_VDDM_POWER[0], pmt->L3_VDDM_POWER[1],
                pmt->L3_VDDM_POWER[0]+pmt->L3_VDDM_POWER[1]);

        //These powers are supplied by other power lines to the CPU and are drawn from the 24 pin ATX connector on most boards
        print_line("","");
        print_line("VDDIO_MEM Power", "%7.4f W", pmt->VDDIO_MEM_POWER);
        print_line("IOD_VDDIO_MEM Power", "%7.4f W", pmt->IOD_VDDIO_MEM_POWER);
        print_line("DDR_VDDP Power", "%7.4f W", pmt->DDR_VDDP_POWER);
        print_line("VDD18 Power", "%7.4f W", pmt->VDD18_POWER); //Same as pmt->IO_VDD18_POWER

        //The sum is the thermal output of the whole package. Yes, this is higher than PPT and SOCKET_POWER.
        //Confirmed by measuring the actual current draw on the mainboard.
        print_line("","");
        print_line("Calculated Thermal Output", "%7.4f W", total_core_power + pmt->VDDCR_SOC_POWER + pmt->GMI2_VDDG_POWER 
                + pmt->L3_LOGIC_POWER[0] + pmt->L3_LOGIC_POWER[1] + pmt->L3_VDDM_POWER[0] + pmt->L3_VDDM_POWER[1]
                + pmt->VDDIO_MEM_POWER + pmt->IOD_VDDIO_MEM_POWER + pmt->DDR_VDDP_POWER + pmt->VDD18_POWER);

        fprintf(stdout, "├── Additional Reports ─────────────────────────┼────────────────────────────────────────────────┤\n");
        //print_line("ROC_POWER", "%7.4f",pmt->ROC_POWER);
        print_line("SoC Power (SVI2)", "%8.3f V | %7.3f A | %8.3f W", pmt->SOC_TELEMETRY_VOLTAGE, pmt->SOC_TELEMETRY_CURRENT, pmt->SOC_TELEMETRY_POWER);
        print_line("Core Power (SVI2)", "%8.3f V | %7.3f A | %8.3f W", pmt->CPU_TELEMETRY_VOLTAGE, pmt->CPU_TELEMETRY_CURRENT, pmt->CPU_TELEMETRY_POWER);
        print_line("Core Power (SMU)", "%7.3f W", pmt->VDDCR_CPU_POWER);
        print_line("Socket Power (SMU)", "%7.4f W", pmt->SOCKET_POWER);
        print_line("Package Power (SMU)", "%7.4f W", pmt->package_power);
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
            "\t-f          - Force PM table monitoring even if the PM table version is not supported.\n"
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
