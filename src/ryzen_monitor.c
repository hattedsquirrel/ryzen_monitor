/**
 * Ryzen SMU Userspace Sensor Monitor
 * Copyright (C) 2020-2021
 *    Florian Huehn <hattedsquirrel@gmail.com> (https://hattedsquirrel.net)
 *    Jeffrey Bosboom <jbosboom@jeffreybosboom.com>
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

#define _GNU_SOURCE

#include <math.h>
#include <sched.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <libsmu.h>
#include "readinfo.h"
#include "pm_tables.h"
#include "output_ops.h"

#define PROGRAM_VERSION "1.0.6"

smu_obj_t obj;
static struct timespec update_time_s = {1, 0};
static int show_disabled_cores = 0;

//Helper to access the PM Table elements. If an element doesn't exist in the
//current PM Table version, it's pointer is set to 0. This helper returns
//NAN for not available fields.
#define pmta(elem) ((pmt->elem)?(*pmt->elem):NAN)
//Same, but with 0 as return. For summations that should not fail if one value is not present.
#define pmta0(elem) ((pmt->elem)?(*pmt->elem):0)

void draw_screen(pm_table *pmt, system_info *sysinfo, const struct output_ops *ops) {
    //general
    int i, j;
    //core block
    float core_voltage, core_frequency, package_sleep_time, core_sleep_time, average_voltage;
    float peak_core_frequency, peak_core_temp, peak_core_voltage;
    float total_core_voltage, total_core_power, total_usage, total_core_CC6;
    int core_disabled, core_number;
    //constraints block
    float edc_value;
    //power block
    float l3_logic_power, l3_vddm_power;
    char strbuf[100];

    if (pmt->experimental) {
        fprintf(stdout, "Warning: Support for this PM table version is expermiental. Can't trust anything.\n");
    }

    ops->begin();

    if (sysinfo->available) {
        ops->begin_group(GROUP_SYSINFO);
        ops->datum_string(DATUM_MODEL, sysinfo->cpu_name);
        ops->datum_string(DATUM_CODENAME, sysinfo->codename);
        ops->datum_int(DATUM_CORES, sysinfo->cores, UNIT_COUNT);
        ops->datum_int(DATUM_CCDS, sysinfo->ccds, UNIT_COUNT);
        if (pmt->zen_version < 3) {
            ops->datum_int(DATUM_CCXS, sysinfo->ccxs, UNIT_COUNT);
            ops->datum_int(DATUM_CORES_PER_CCX, sysinfo->cores_per_ccx, UNIT_COUNT);
        } else
            ops->datum_int(DATUM_CORES_PER_CCD, sysinfo->cores_per_ccx, UNIT_COUNT); //Zen3 does not have CCXs anymore
        ops->datum_string(DATUM_SMU_FW_VERSION, sysinfo->smu_fw_ver);
        ops->datum_int(DATUM_MP1_IF_VERSION, sysinfo->if_ver, UNIT_COUNT);
        ops->end_group(GROUP_SYSINFO);
    }


    peak_core_frequency = peak_core_temp = peak_core_voltage = 0;
    total_core_voltage = total_core_power = total_usage = total_core_CC6 = 0;
    core_number = 0;

    if(pmt->PC6)
    {
        package_sleep_time = pmta(PC6) / 100.f;
        average_voltage = (pmta(CPU_TELEMETRY_VOLTAGE) - (0.2 * package_sleep_time)) / (1.0 - package_sleep_time);
    }
    else
    {
        average_voltage = pmta(CPU_TELEMETRY_VOLTAGE);
    }

    ops->begin_group(GROUP_CORES);
    for (i = 0; i < pmt->max_cores; i++) {
        core_disabled = (sysinfo->core_disable_map >> i)&0x01;
        core_frequency = pmta(CORE_FREQEFF[i]) * 1000.f;

        core_voltage = pmta(CORE_VOLTAGE[i]); // True core voltage
        // Rumours say this is how AMD calculates core voltage
        //if (pmta(CORE_FREQ[i]) != 0.f) {
            core_sleep_time = pmta(CORE_CC6[i]) / 100.f;
            core_voltage = ((1.0 - core_sleep_time) * average_voltage) + (0.2 * core_sleep_time);
        //}

        // AMD denotes a sleeping core as having spent less than 6% of the time in C0.
        // Source: Ryzen Master
        _Bool core_sleeping = pmta(CORE_C0[i]) <= 6.f;

        if (show_disabled_cores || !core_disabled)
            ops->core(core_number, core_disabled, core_sleeping, core_frequency,
                    pmta(CORE_POWER[i]), core_voltage, pmta(CORE_TEMP[i]),
                    pmta(CORE_C0[i]), pmta(CORE_CC1[i]), pmta(CORE_CC6[i]));

        //Don't confuse people by numbering cores that are disabled and hence not shown on 6 | 12 core CPUs
        //(which actually have 8 | 16 cores)
        if (show_disabled_cores || !core_disabled) core_number++;

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
    ops->end_group(GROUP_CORES);

    ops->begin_group(GROUP_CORE_STATS_CALC);
    ops->datum_float(DATUM_PEAK_CORE_FREQ, peak_core_frequency, UNIT_MEGAHERTZ);
    ops->datum_float(DATUM_PEAK_CORE_TEMP, peak_core_temp, UNIT_CELSIUS);
    ops->datum_float(DATUM_PEAK_CORE_VOLTAGE, peak_core_voltage, UNIT_VOLTS);
    ops->datum_float(DATUM_AVG_CORE_VOLTAGE, total_core_voltage/sysinfo->enabled_cores_count, UNIT_VOLTS);
    ops->datum_float(DATUM_AVG_CORE_C6, (total_core_CC6/sysinfo->enabled_cores_count)/100.f, UNIT_RATIO);
    ops->datum_float(DATUM_TOTAL_CORE_POWER, total_core_power, UNIT_WATTS);
    ops->end_group(GROUP_CORE_STATS_CALC);

    ops->begin_group(GROUP_CORE_STATS_SMU);
    //SOCKET_POWER reported in GROUP_POWER
    ops->datum_float(DATUM_PEAK_CORE_VOLTAGE_SMU, pmta(CPU_TELEMETRY_VOLTAGE), UNIT_VOLTS);
    ops->datum_float(DATUM_PACKAGE_C6_SMU, pmta(PC6)/100.f, UNIT_RATIO);
    ops->end_group(GROUP_CORE_STATS_SMU);

    ops->begin_group(GROUP_LIMITS);
    ops->datum_float(DATUM_PEAK_TEMP, pmta(PEAK_TEMP), UNIT_CELSIUS);
    if (pmt->SOC_TEMP) ops->datum_float(DATUM_SOC_TEMP, pmta(SOC_TEMP), UNIT_CELSIUS);
    if (pmt->GFX_TEMP) ops->datum_float(DATUM_GFX_TEMP, pmta(GFX_TEMP), UNIT_CELSIUS);

    edc_value = pmta(EDC_VALUE) * (total_usage / sysinfo->cores / 100);
    if (edc_value < pmta(TDC_VALUE)) edc_value = pmta(TDC_VALUE);

    ops->datum_limit(DATUM_CORE_VRM_VOLTAGE_LIMIT, pmta(VID_VALUE), pmta(VID_LIMIT), UNIT_VOLTS);
    //STAPM_VALUE / STAPM_LIMIT not reported with no explanation
    ops->datum_limit(DATUM_PPT_LIMIT, pmta(PPT_VALUE), pmta(PPT_LIMIT), UNIT_WATTS);
    if (pmt->PPT_VALUE_APU) ops->datum_limit(DATUM_PPT_APU_LIMIT, pmta(PPT_VALUE_APU), pmta(PPT_LIMIT_APU), UNIT_WATTS);
    ops->datum_limit(DATUM_TDC_LIMIT_NOMINAL, pmta(TDC_VALUE), pmta(TDC_LIMIT), UNIT_AMPS);
    if (pmt->TDC_ACTUAL) ops->datum_limit(DATUM_TDC_LIMIT_ACTUAL, pmta(TDC_ACTUAL), pmta(TDC_LIMIT), UNIT_AMPS);
    if (pmt->TDC_VALUE_SOC) ops->datum_limit(DATUM_TDC_SOC_LIMIT_NOMINAL, pmta(TDC_VALUE_SOC), pmta(TDC_LIMIT_SOC), UNIT_AMPS);
    ops->datum_limit(DATUM_EDC_LIMIT, edc_value, pmta(EDC_LIMIT), UNIT_AMPS);
    if (pmt->EDC_VALUE_SOC) ops->datum_limit(DATUM_EDC_SOC_LIMIT, pmta(EDC_VALUE_SOC), pmta(EDC_LIMIT_SOC), UNIT_AMPS);
    ops->datum_limit(DATUM_THM_LIMIT, pmta(THM_VALUE), pmta(THM_LIMIT), UNIT_CELSIUS);
    if (pmt->THM_VALUE_SOC) ops->datum_limit(DATUM_THM_SOC_LIMIT, pmta(THM_VALUE_SOC), pmta(THM_LIMIT_SOC), UNIT_CELSIUS);
    if (pmt->THM_VALUE_GFX) ops->datum_limit(DATUM_THM_GFX_LIMIT, pmta(THM_VALUE_GFX), pmta(THM_LIMIT_GFX), UNIT_CELSIUS);
    // STT_VALUE_APU / STT_LIMIT_APU always zero
    // STT_VALUE_DGPU / STT_LIMIT_DGPU always zero
    ops->datum_limit(DATUM_FIT_LIMIT, pmta(FIT_VALUE), pmta(FIT_LIMIT), UNIT_COUNT);
    ops->end_group(GROUP_LIMITS);

    ops->begin_group(GROUP_MEMORY);
    ops->datum_bool(DATUM_MEMORY_COUPLED, pmta(UCLK_FREQ) == pmta(MEMCLK_FREQ));
    ops->datum_float(DATUM_FCLK_AVG, pmta(FCLK_FREQ_EFF), UNIT_MEGAHERTZ);
    ops->datum_float(DATUM_FCLK, pmta(FCLK_FREQ), UNIT_MEGAHERTZ);
    ops->datum_float(DATUM_UCLK, pmta(UCLK_FREQ), UNIT_MEGAHERTZ);
    ops->datum_float(DATUM_MCLK, pmta(MEMCLK_FREQ), UNIT_MEGAHERTZ);
    // VDDIO_MEM_POWER reported in GROUP_POWER
    // SOC_SET_VOLTAGE might be the default voltage, not the actually set one
    ops->datum_float(DATUM_VDDM, pmta(V_VDDM), UNIT_VOLTS);
    ops->datum_float(DATUM_VDDP, pmta(V_VDDP), UNIT_VOLTS);
    if (pmt->V_VDDG) ops->datum_float(DATUM_VDDG, pmta(V_VDDG), UNIT_VOLTS);
    if (pmt->V_VDDG_IOD) ops->datum_float(DATUM_VDDG_IOD, pmta(V_VDDG_IOD), UNIT_VOLTS);
    if (pmt->V_VDDG_CCD) ops->datum_float(DATUM_VDDG_CCD, pmta(V_VDDG_CCD), UNIT_VOLTS);
    ops->end_group(GROUP_MEMORY);

    if (pmt->has_graphics) {
        ops->begin_group(GROUP_GRAPHICS);
        ops->datum_float2(DATUM_GFX_VOLTAGE_ROC_POWER, pmta(GFX_VOLTAGE), UNIT_VOLTS, pmta(ROC_POWER), UNIT_WATTS);
        ops->datum_float(DATUM_GFX_TEMP, pmta(GFX_TEMP), UNIT_CELSIUS);
        ops->datum_float2(DATUM_GFX_FREQ_REAL_EFF, pmta(GFX_FREQ), UNIT_MEGAHERTZ, pmta(GFX_FREQEFF), UNIT_MEGAHERTZ);
        ops->datum_float(DATUM_GFX_BUSY, pmta(GFX_BUSY), UNIT_RATIO);
        ops->datum_float2(DATUM_GFX_EDC_LIMIT_RESIDENCY, pmta(GFX_EDC_LIM), UNIT_AMPS, pmta(GFX_EDC_RESIDENCY), UNIT_RATIO);
        ops->datum_float2(DATUM_GFX_DISPLAY_COUNT_FPS, pmta(DISPLAY_COUNT), UNIT_COUNT, pmta(FPS), UNIT_COUNT);
        ops->datum_float3(DATUM_GFX_DGPU_POWER_FREQ_TARGET_BUSY, pmta(DGPU_POWER), UNIT_WATTS, pmta(DGPU_FREQ_TARGET), UNIT_MEGAHERTZ, pmta(DGPU_GFX_BUSY), UNIT_RATIO);
        ops->end_group(GROUP_GRAPHICS);
    }

    ops->begin_group(GROUP_POWER);
    //These powers are drawn via VDDCR_SOC and VDDCR_CPU and thus are pulled from the CPU power connector of the mainboard
    ops->datum_float(DATUM_TOTAL_CORE_POWER, total_core_power, UNIT_WATTS);
    // VDDCR_CPU_POWER doesn't correlate with what the cores report, nor with
    // what is actually consumed, but is the value HWiNFO shows.  It is
    // reported in GROUP_POWER_REPORTS.
    ops->datum_float(DATUM_VDDR_SOC_POWER, pmta(VDDCR_SOC_POWER), UNIT_WATTS);
    if (pmt->IO_VDDCR_SOC_POWER) ops->datum_float(DATUM_IO_VDDR_SOC_POWER, pmta(IO_VDDCR_SOC_POWER), UNIT_WATTS);
    if (pmt->GMI2_VDDG_POWER) ops->datum_float(DATUM_GMI2_VDDG_POWER, pmta(GMI2_VDDG_POWER), UNIT_WATTS);
    if (pmt->ROC_POWER) ops->datum_float(DATUM_ROC_POWER, pmta(ROC_POWER), UNIT_WATTS);

    //L3 caches (2 per CCD on Zen2, 1 per CCD on Zen3)
    l3_logic_power=0;
    l3_vddm_power=0;
    for (i=0; i<pmt->max_l3; i++) {
        l3_logic_power += pmta0(L3_LOGIC_POWER[i]);
        l3_vddm_power += pmta0(L3_VDDM_POWER[i]);
    }
    ops->datum_sum(DATUM_L3_LOGIC_POWER, pmt->L3_LOGIC_POWER[0], pmt->L3_LOGIC_POWER[0] + pmt->max_l3, UNIT_WATTS);
    ops->datum_sum(DATUM_L3_VDDM_POWER, pmt->L3_VDDM_POWER[0], pmt->L3_VDDM_POWER[0] + pmt->max_l3, UNIT_WATTS);

    //These powers are supplied by other power lines to the CPU and are drawn from the 24 pin ATX connector on most boards
    ops->datum_float(DATUM_VDDIO_MEM_POWER, pmta(VDDIO_MEM_POWER), UNIT_WATTS);
    ops->datum_float(DATUM_IOD_VDDIO_MEM_POWER, pmta(IOD_VDDIO_MEM_POWER), UNIT_WATTS);
    if (pmt->DDR_VDDP_POWER) ops->datum_float(DATUM_DDR_VDDP_POWER, pmta(DDR_VDDP_POWER), UNIT_WATTS);
    if (pmt->DDR_PHY_POWER) ops->datum_float(DATUM_DDR_PHY_POWER, pmta(DDR_PHY_POWER), UNIT_WATTS);
    ops->datum_float(DATUM_VDD18_POWER, pmta(VDD18_POWER), UNIT_WATTS); //Same as pmta(IO_VDD18_POWER)
    if (pmt->IO_DISPLAY_POWER) ops->datum_float(DATUM_IO_DISPLAY_POWER, pmta(IO_DISPLAY_POWER), UNIT_WATTS);
    if (pmt->IO_USB_POWER) ops->datum_float(DATUM_IO_USB_POWER, pmta(IO_USB_POWER), UNIT_WATTS);

    if (!pmt->powersum_unclear)
        //The sum is the thermal output of the whole package. Yes, this is higher than PPT and SOCKET_POWER.
        //Confirmed by measuring the actual current draw on the mainboard.
        ops->datum_float(DATUM_CALC_TOTAL_POWER, total_core_power + pmta0(VDDCR_SOC_POWER) + pmta0(GMI2_VDDG_POWER)
                + l3_logic_power + l3_vddm_power
                + pmta0(VDDIO_MEM_POWER) + pmta0(IOD_VDDIO_MEM_POWER) + pmta0(DDR_VDDP_POWER) + pmta0(VDD18_POWER), UNIT_WATTS);
    ops->end_group(GROUP_POWER);

    ops->begin_group(GROUP_POWER_REPORTS);
    //ROC_POWER reported in GROUP_GRAPHICS and GROUP_POWER
    ops->datum_float3(DATUM_SVI2_SOC_POWER, pmta(SOC_TELEMETRY_VOLTAGE), UNIT_VOLTS, pmta(SOC_TELEMETRY_CURRENT), UNIT_AMPS, pmta(SOC_TELEMETRY_POWER), UNIT_WATTS);
    ops->datum_float3(DATUM_SVI2_CORE_POWER, pmta(CPU_TELEMETRY_VOLTAGE), UNIT_VOLTS, pmta(CPU_TELEMETRY_CURRENT), UNIT_AMPS, pmta(CPU_TELEMETRY_POWER), UNIT_WATTS);
    ops->datum_float(DATUM_SMU_CORE_POWER, pmta(VDDCR_CPU_POWER), UNIT_WATTS);
    ops->datum_float(DATUM_SMU_SOCKET_POWER, pmta(SOCKET_POWER), UNIT_WATTS);
    if (pmt->PACKAGE_POWER) ops->datum_float(DATUM_SMU_PACKAGE_POWER, pmta(PACKAGE_POWER), UNIT_WATTS);
    ops->end_group(GROUP_POWER_REPORTS);

    ops->end();
}

int select_pm_table_version(unsigned int version, pm_table *pmt, unsigned char *pm_buf) {
    //Initialize pmt to 0. This also sets all pointers to 0, which signifies non-existiting fields.
    //Access via pmta(...) will check if pointer is 0 before trying to access the value.
    memset(pmt, 0, sizeof(pm_table));

     //Select matching PM Table
    switch(version) {
        case 0x380904: pm_table_0x380904(pmt, pm_buf); break; //Ryzen 5600X
        case 0x380905: pm_table_0x380905(pmt, pm_buf); break; //Ryzen 5600X
        case 0x380804: pm_table_0x380804(pmt, pm_buf); break; //Ryzen 5900X / 5950X
        case 0x380805: pm_table_0x380805(pmt, pm_buf); break; //Ryzen 5900X / 5950X
        case 0x400005: pm_table_0x400005(pmt, pm_buf); break; //Ryzen 5700G
        case 0x240903: pm_table_0x240903(pmt, pm_buf); break; //Ryzen 3700X / 3800X
        case 0x240803: pm_table_0x240803(pmt, pm_buf); break; //Ryzen 3950X
        default:
            return 0;
    }

    //Avoid access bejond bounds of the defined arrays.
    if (pmt->max_l3 > PMT_MAX_NUM_L3) pmt->max_l3 = PMT_MAX_NUM_L3;
    if (pmt->max_cores > PMT_MAX_NUM_CORES) pmt->max_cores = PMT_MAX_NUM_CORES;

    //APML_POWER is probably identical to PACKAGE_POWER
    if (pmt->PACKAGE_POWER == NULL) pmt->PACKAGE_POWER = pmt->APML_POWER;

    if (pmt->VDD18_POWER == NULL) pmt->VDD18_POWER = pmt->IO_VDD18_POWER;

    return 1;
}

void disabled_cores_0x400005(pm_table *pmt, system_info *sysinfo) {
    int i, mask;
    float power, voltage, fit, iddmax, freq, freqeff, c0, cc1, irm;
    for (i = 0; i < 8; i++) {
        power = pmta(CORE_POWER[i]);
        voltage = pmta(CORE_VOLTAGE[i]);
        fit = pmta(CORE_FIT[i]);
        iddmax = pmta(CORE_IDDMAX[i]);
        freq = pmta(CORE_FREQ[i]);
        freqeff = pmta(CORE_FREQEFF[i]);
        c0 = pmta(CORE_C0[i]);
        cc1 = pmta(CORE_CC1[i]);
        irm = pmta(CORE_IRM[i]);
        
        if (power == 0 && voltage == 0 && fit == 0 && iddmax == 0 && freq == 0 && freqeff == 0 && c0 == 0 && cc1 == 0 && irm == 0 ) {
            mask = 1 << i;
        } else {
            mask = 0 << i;
        }
        sysinfo->core_disable_map_pmt = sysinfo->core_disable_map_pmt | mask;
    }
}

static sig_atomic_t stop_requested = 0;
void signal_interrupt(int sig) {
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            if (stop_requested)
                // We got stuck cleaning up and the user sent another signal.
                // Exit now without cleaning up.
                _exit(1);
            stop_requested = 1;
            break;
        default:
            break;
    }
}

void start_pm_monitor(unsigned int force, int repeating, const struct output_ops* ops) {
    unsigned char *pm_buf;
    pm_table pmt;
    system_info sysinfo;

    if (!smu_pm_tables_supported(&obj)) {
        fprintf(stderr, "PM Tables are not supported on this platform.\n");
        exit(0);
    }

    pm_buf = calloc(obj.pm_table_size, sizeof(unsigned char));
    if (!pm_buf) {
        fprintf(stderr, "Could not allocate memory for the PM Table.\n");
        exit(0);
    }

    if (force) {
        fprintf(stderr, "Forcing to use PM Table version 0x%x. System reports version 0x%x.\n", force, obj.pm_table_version);
    }

    //Select matching PM Table
    if(!select_pm_table_version(force?force:obj.pm_table_version, &pmt, pm_buf)) {
        fprintf(stderr, "This PM Table version (0x%x) is currently not supported.\n", force?force:obj.pm_table_version);
        fprintf(stderr, "Processor name: %s\n", get_processor_name());
        fprintf(stderr, "SMU FW version: %s\n", smu_get_fw_version(&obj));
        exit(0);
    }
    //Prevent illegal memory access
    if (obj.pm_table_size < pmt.min_size) {
        fprintf(stderr, "Selected PM Table is larger than the PM Table returned by the SMU.\n");
        exit(0);
    }
    //Maximum core count. Just to be safe. Will be overwritten by get_processor_topology(...).
    sysinfo.enabled_cores_count = pmt.max_cores;

    sysinfo.cpu_name    = get_processor_name();
    sysinfo.codename    = smu_codename_to_str(&obj);
    sysinfo.smu_fw_ver  = smu_get_fw_version(&obj);

    //PMT hack for Cezanne's core_disabled_map 
    if (obj.pm_table_version == 0x400005) {
        if (smu_read_pm_table(&obj, pm_buf, obj.pm_table_size) == SMU_Return_OK) {
            disabled_cores_0x400005(&pmt, &sysinfo);
        }
    }
    
    get_processor_topology(&sysinfo, pmt.zen_version);

    switch (obj.smu_if_version) {
        case IF_VERSION_9:  sysinfo.if_ver =  9; break;
        case IF_VERSION_10: sysinfo.if_ver = 10; break;
        case IF_VERSION_11: sysinfo.if_ver = 11; break;
        case IF_VERSION_12: sysinfo.if_ver = 12; break;
        case IF_VERSION_13: sysinfo.if_ver = 13; break;
        default:            sysinfo.if_ver =  0; break;
    }

    ops->init(repeating, isatty(fileno(stdout)));

    do {
        if (smu_read_pm_table(&obj, pm_buf, obj.pm_table_size) != SMU_Return_OK)
            continue;

        draw_screen(&pmt, &sysinfo, ops);

        nanosleep(&update_time_s, NULL);
    } while (repeating && !stop_requested);

    ops->cleanup();
}

void read_from_dumpfile(char *dumpfile, unsigned int version, const struct output_ops* ops) {
    unsigned char readbuf[10240];
    unsigned int bytes_read;
    pm_table pmt;
    system_info sysinfo;
    FILE *fd;

    if (!version) {
        fprintf(stderr, "You need to specify a PM Table version with -f.\n");
        exit(0);
    }

    //Read file
    fd = fopen(dumpfile, "rb");
    if(!fd) {
        fprintf(stderr, "Could not read the dumpfile (\"%s\").\n", dumpfile);
        exit(0);
    }
    bytes_read=fread(readbuf,sizeof(char),sizeof(readbuf),fd);
    fclose(fd);

    //Select matching PM Table
    if(!select_pm_table_version(version, &pmt, readbuf)) {
        fprintf(stderr, "This PM Table version (0x%x) is currently not supported.\n", version);
        exit(0);
    }
    else fprintf(stderr, "Using PM Table version 0x%x.\n", version);

    //Prevent illegal memory access
    if (bytes_read < pmt.min_size) {
        fprintf(stderr, "Read %d bytes from \"%s\", but the selected PM Table is %d bytes long.\n", bytes_read, dumpfile, pmt.min_size);
        exit(0);
    }
    
    sysinfo.available=0; //Did not read sysinfo
    sysinfo.enabled_cores_count = pmt.max_cores;
    sysinfo.core_disable_map=0;
    sysinfo.cores=sysinfo.enabled_cores_count;

    ops->init(0, isatty(fileno(stdout)));
    draw_screen(&pmt, &sysinfo, ops);
    ops->cleanup();
}

void print_version() {
    fprintf(stdout, "Ryzen Monitor " PROGRAM_VERSION "\n");
    exit(0);
}

void show_help(char* program) {
    fprintf(stdout,
        "Ryzen Monitor " PROGRAM_VERSION "\n\n"

        "Usage: %s <option(s)>\n\n"

        "Options:\n"
            "\t-h            - Show this help screen.\n"
            "\t-v            - Show program version.\n"
            "\t-m            - Print DRAM Timings and exit.\n"
            "\t-d            - Show disabled cores.\n"
            "\t-u<seconds>   - Update the monitoring only after this number of second(s) have passed. Defaults to 1.\n"
            "\t-f<hex-value> - Force to use a specific PM table version.\n"
            "\t-t<filename>  - Test mode. Read PM Table from raw-dumfile. Use in conjunction with -f\n",
            "\t-o<format>    - Output format (boxdrawing, json, ndjson)\n",
            "\t-1            - Print once and exit\n",
        program
    );
}

struct timespec parse_fractional_time(const char* s) {
    // In "3.14", the "3" represents 3 seconds, but the "14" represents
    // 140000000 nanoseconds -- it's a "left-justified" number.  Deal with that
    // by writing the fractional part to the front of a buffer of "0" and then
    // scanning that buffer.
    struct timespec t;
    char fract[] = "000000000";
    sscanf(s, "%ld.%9c", &t.tv_sec, fract);
    t.tv_nsec = atol(fract);
    return t;
}

int main(int argc, char** argv) {
    smu_return_val ret;
    int c=0, force=0, core=0, printtimings=0, repeating=1;
    char *dumpfile=0;
    const struct output_ops* ops = &box_drawing_ops;

    //Set up signal handlers
    if ((signal(SIGTERM, signal_interrupt) == SIG_ERR) ||
        (signal(SIGINT, signal_interrupt) == SIG_ERR)) {
        fprintf(stderr, "Can't set up signal hooks.\n");
        exit(-1);
    }

    //Parse arguments
    while ((c = getopt(argc, argv, "vmd::f:t:u:o:1h")) != -1) {
        switch (c) {
            case 'v':
                print_version();
                exit(0);
            case 'm':
                printtimings = 1;
                break;
            case 'd':
                if (optarg)
                    show_disabled_cores = atoi(optarg);
                else
                    show_disabled_cores = 1;
                break;
            case 'f':
                if(!optarg || sscanf(optarg,"%x", &force)!=1) {
                    show_help(argv[0]);
                    exit(0);
                }
                break;
            case 't':
                if(!optarg || strlen(optarg)==0) {
                    show_help(argv[0]);
                    exit(0);
                }
                dumpfile=optarg;
                break;
            case 'o':
                if (!optarg) optarg = "";
                if (!strcmp(optarg, "boxdrawing") || !strcmp(optarg, "box-drawing"))
                    ops = &box_drawing_ops;
                else if (!strcmp(optarg, "json"))
                    ops = &json_ops;
                else if (!strcmp(optarg, "ndjson"))
                    ops = &ndjson_ops;
                else {
                    show_help(argv[0]);
                    exit(0);
                }
                break;
            case '1':
                repeating = 0;
                break;
            case 'u':
                update_time_s = parse_fractional_time(optarg);
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

    if (!repeating)
        update_time_s.tv_sec = update_time_s.tv_nsec = 0;

    if(dumpfile && !printtimings)
        read_from_dumpfile(dumpfile, force, ops);
    else
    {
        if (getuid() != 0 && geteuid() != 0) {
            fprintf(stderr, "Program must be run as root.\n");
            exit(-2);
        }

        ret = smu_init(&obj);
        if (ret != SMU_Return_OK) {
            fprintf(stderr, "%s\n", smu_return_to_str(ret));
            exit(-2);
        }

        if(printtimings) print_memory_timings();
        else start_pm_monitor(force, repeating, ops);
    }

    return 0;
}
