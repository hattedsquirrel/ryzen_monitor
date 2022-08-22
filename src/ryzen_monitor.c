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

#define PROGRAM_VERSION "1.0.6"

smu_obj_t obj;
static int update_time_s = 1;
static int show_disabled_cores = 0;

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
#define pmta(elem) ((pmt->elem)?(*pmt->elem):NAN)
//Same, but with 0 as return. For summations that should not fail if one value is not present.
#define pmta0(elem) ((pmt->elem)?(*pmt->elem):0)

void draw_screen(pm_table *pmt, system_info *sysinfo) {
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

    if (sysinfo->available) {
        fprintf(stdout, "╭───────────────────────────────────────────────┬────────────────────────────────────────────────╮\n");
        print_line("CPU Model", sysinfo->cpu_name);
        print_line("Processor Code Name", sysinfo->codename);
        print_line("Cores", "%d", sysinfo->cores);
        print_line("Core CCDs", "%d", sysinfo->ccds);
        if (pmt->zen_version!=3) {
            print_line("Core CCXs", "%d", sysinfo->ccxs);
            print_line("Cores Per CCX", "%d", sysinfo->cores_per_ccx);
        }
        else
            print_line("Cores Per CCD", "%d", sysinfo->cores_per_ccx); //Zen3 does not have CCXs anymore
        print_line("SMU FW Version", "v%s", sysinfo->smu_fw_ver);
        print_line("MP1 IF Version", "v%d", sysinfo->if_ver);
        fprintf(stdout, "╰───────────────────────────────────────────────┴────────────────────────────────────────────────╯\n");
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

    fprintf(stdout, "╭─────────┬────────────┬──────────┬─────────┬──────────┬─────────────┬─────────────┬─────────────╮\n");
    for (i = 0; i < pmt->max_cores; i++) {
        core_disabled = (sysinfo->core_disable_map >> i)&0x01;
        core_frequency = pmta(CORE_FREQEFF[i]) * 1000.f;

        core_voltage = pmta(CORE_VOLTAGE[i]); // True core voltage
        // Rumours say this is how AMD calculates core voltage
        //if (pmta(CORE_FREQ[i]) != 0.f) {
            core_sleep_time = pmta(CORE_CC6[i]) / 100.f;
            core_voltage = ((1.0 - core_sleep_time) * average_voltage) + (0.2 * core_sleep_time);
        //}

        if (core_disabled) {
            if (show_disabled_cores)
                    fprintf(stdout,
                        "│ %*s %d │   Disabled | %6.3f W | %5.3f V | %6.2f C | C0: %5.1f %% | C1: %5.1f %% | C6: %5.1f %% │\n",
                    (core_number<10)+4, "Core", core_number, //Print "Core" and its number but right-justified
                        pmta(CORE_POWER[i]), core_voltage, pmta(CORE_TEMP[i]),
                        pmta(CORE_C0[i]), pmta(CORE_CC1[i]), pmta(CORE_CC6[i]));
        }
        else if (pmta(CORE_C0[i]) >= 6.f) {
            // AMD denotes a sleeping core as having spent less than 6% of the time in C0.
            // Source: Ryzen Master
                fprintf(stdout,
                    "│ %*s %d │   %4.f MHz | %6.3f W | %5.3f V | %6.2f C | C0: %5.1f %% | C1: %5.1f %% | C6: %5.1f %% │\n",
                (core_number<10)+4, "Core", core_number, //Print "Core" and its number but right-justified
                core_frequency, pmta(CORE_POWER[i]), core_voltage, pmta(CORE_TEMP[i]),
                    pmta(CORE_C0[i]), pmta(CORE_CC1[i]), pmta(CORE_CC6[i]));
            }
            else {
                fprintf(stdout,
                    "│ %*s %d │   Sleeping | %6.3f W | %5.3f V | %6.2f C | C0: %5.1f %% | C1: %5.1f %% | C6: %5.1f %% │\n",
                (core_number<10)+4, "Core", core_number, //Print "Core" and its number but right-justified
                    pmta(CORE_POWER[i]), core_voltage, pmta(CORE_TEMP[i]),
                    pmta(CORE_C0[i]), pmta(CORE_CC1[i]), pmta(CORE_CC6[i]));
        }

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

    fprintf(stdout, "╰─────────┴────────────┴──────────┴─────────┴──────────┴─────────────┴─────────────┴─────────────╯\n");

    fprintf(stdout, "╭── Core Statistics (Calculated) ───────────────┬────────────────────────────────────────────────╮\n");
    print_line("Highest Effective Core Frequency", "%8.0f MHz", peak_core_frequency);
    print_line("Highest Core Temperature", "%8.2f C", peak_core_temp);
    print_line("Highest Core Voltage", "%8.3f V", peak_core_voltage);
    print_line("Average Core Voltage", "%5.3f V", total_core_voltage/sysinfo->enabled_cores_count);
    print_line("Average Core CC6", "%6.2f %%", total_core_CC6/sysinfo->enabled_cores_count);
    print_line("Total Core Power Sum", "%7.3f W", total_core_power);

    fprintf(stdout, "├── Reported by SMU ────────────────────────────┼────────────────────────────────────────────────┤\n");
    //print_line("Package Power", "%8.3f W", pmta(SOCKET_POWER)); //Is listed below in power section
    print_line("Peak Core Voltage", "%5.3f V", pmta(CPU_TELEMETRY_VOLTAGE));
    if(pmt->PC6) print_line("Package CC6", "%6.2f %%", pmta(PC6));
    fprintf(stdout, "╰───────────────────────────────────────────────┴────────────────────────────────────────────────╯\n");

    fprintf(stdout, "╭── Electrical & Thermal Constraints ───────────┬────────────────────────────────────────────────╮\n");
    edc_value = pmta(EDC_VALUE) * (total_usage / sysinfo->cores / 100);
    if (edc_value < pmta(TDC_VALUE)) edc_value = pmta(TDC_VALUE);

    print_line("Peak Temperature", "%8.2f C", pmta(PEAK_TEMP));
    if(pmt->SOC_TEMP) print_line("SoC Temperature", "%8.2f C", pmta(SOC_TEMP));
    if(pmt->GFX_TEMP) print_line("GFX Temperature", "%8.2f C", pmta(GFX_TEMP));
    //print_line("Core Power", "%8.4f W", pmta(VDDCR_CPU_POWER));

    print_line("Voltage from Core VRM", "%7.3f V | %7.3f V | %8.2f %%", pmta(VID_VALUE), pmta(VID_LIMIT), (pmta(VID_VALUE) / pmta(VID_LIMIT) * 100));
    //if(pmt->STAPM_VALUE) print_line("STAPM", "%7.3f   | %7.f   | %8.2f %%", pmta(STAPM_VALUE), pmta(STAPM_LIMIT), (pmta(STAPM_VALUE) / pmta(STAPM_LIMIT) * 100));
    print_line("PPT", "%7.3f W | %7.f W | %8.2f %%", pmta(PPT_VALUE), pmta(PPT_LIMIT), (pmta(PPT_VALUE) / pmta(PPT_LIMIT) * 100));
    if(pmt->PPT_VALUE_APU) print_line("PPT APU", "%7.3f W | %7.f W | %8.2f %%", pmta(PPT_VALUE_APU), pmta(PPT_LIMIT_APU), (pmta(PPT_VALUE_APU) / pmta(PPT_LIMIT_APU) * 100));
    print_line("TDC Value", "%7.3f A | %7.f A | %8.2f %%", pmta(TDC_VALUE), pmta(TDC_LIMIT), (pmta(TDC_VALUE) / pmta(TDC_LIMIT) * 100));
    if(pmt->TDC_ACTUAL) print_line("TDC Actual", "%7.3f A | %7.f A | %8.2f %%", pmta(TDC_ACTUAL), pmta(TDC_LIMIT), (pmta(TDC_ACTUAL) / pmta(TDC_LIMIT) * 100));
    if(pmt->TDC_VALUE_SOC) print_line("TDC Value, SoC only", "%7.3f A | %7.f A | %8.2f %%", pmta(TDC_VALUE_SOC), pmta(TDC_LIMIT_SOC), (pmta(TDC_VALUE_SOC) / pmta(TDC_LIMIT_SOC) * 100));
    print_line("EDC", "%7.3f A | %7.f A | %8.2f %%", edc_value, pmta(EDC_LIMIT), (edc_value / pmta(EDC_LIMIT) * 100));
    if(pmt->EDC_VALUE_SOC) print_line("EDC, SoC only", "%7.3f A | %7.f A | %8.2f %%", pmta(EDC_VALUE_SOC), pmta(EDC_LIMIT_SOC), (pmta(EDC_VALUE_SOC) / pmta(EDC_LIMIT_SOC) * 100));
    print_line("THM", "%7.2f C | %7.f C | %8.2f %%", pmta(THM_VALUE), pmta(THM_LIMIT), (pmta(THM_VALUE) / pmta(THM_LIMIT) * 100));
    if(pmt->THM_VALUE_SOC) print_line("THM SoC", "%7.2f C | %7.f C | %8.2f %%", pmta(THM_VALUE_SOC), pmta(THM_LIMIT_SOC), (pmta(THM_VALUE_SOC) / pmta(THM_LIMIT_SOC) * 100));
    if(pmt->THM_VALUE_GFX) print_line("THM GFX", "%7.2f C | %7.f C | %8.2f %%", pmta(THM_VALUE_GFX), pmta(THM_LIMIT_GFX), (pmta(THM_VALUE_GFX) / pmta(THM_LIMIT_GFX) * 100));
    //if(pmt->STT_LIMIT_APU) print_line("STT APU", "%7.2f   | %7.f   | %8.2f %%", pmta(STT_VALUE_APU), pmta(STT_LIMIT_APU), (pmta(STT_VALUE_APU) / pmta(STT_LIMIT_APU) * 100)); //Always zero
    //if(pmt->STT_LIMIT_DGPU) print_line("STT DGPU", "%7.2f   | %7.f   | %8.2f %%", pmta(STT_VALUE_DGPU), pmta(STT_LIMIT_DGPU), (pmta(STT_VALUE_DGPU) / pmta(STT_LIMIT_DGPU) * 100)); //Always zero
    print_line("FIT", "%7.f   | %7.f   | %8.2f %%", pmta(FIT_VALUE), pmta(FIT_LIMIT), (pmta(FIT_VALUE) / pmta(FIT_LIMIT)) * 100.f);
    fprintf(stdout, "╰───────────────────────────────────────────────┴────────────────────────────────────────────────╯\n");

    fprintf(stdout, "╭── Memory Interface ───────────────────────────┬────────────────────────────────────────────────╮\n");
    print_line("Coupled Mode", "%8s", pmta(UCLK_FREQ) == pmta(MEMCLK_FREQ) ? "ON" : "OFF");
    print_line("Fabric Clock (Average)", "%5.f MHz", pmta(FCLK_FREQ_EFF));
    print_line("Fabric Clock", "%5.f MHz", pmta(FCLK_FREQ));
    print_line("Uncore Clock", "%5.f MHz", pmta(UCLK_FREQ));
    print_line("Memory Clock", "%5.f MHz", pmta(MEMCLK_FREQ));
    //print_line("VDDCR_Mem", "%7.3f W", pmta(VDDIO_MEM_POWER)); //Is listed below in power section
    //print_line("VDDCR_SoC", "%7.3f V", pmta(SOC_SET_VOLTAGE)); //Might be the default voltage, not the actually set one
    print_line("cLDO_VDDM", "%7.4f V", pmta(V_VDDM));
    print_line("cLDO_VDDP", "%7.4f V", pmta(V_VDDP));
    if(pmt->V_VDDG)     print_line("cLDO_VDDG", "%7.4f V", pmta(V_VDDG));
    if(pmt->V_VDDG_IOD) print_line("cLDO_VDDG_IOD", "%7.4f V", pmta(V_VDDG_IOD));
    if(pmt->V_VDDG_CCD) print_line("cLDO_VDDG_CCD", "%7.4f V", pmta(V_VDDG_CCD));
    fprintf(stdout, "╰───────────────────────────────────────────────┴────────────────────────────────────────────────╯\n");

    if(pmt->has_graphics){
    fprintf(stdout, "╭── Graphics Subsystem──────────────────────────┬────────────────────────────────────────────────╮\n");
    print_line("GFX Voltage | ROC Power", "%7.4f V | %8.3f W", pmta(GFX_VOLTAGE), pmta(ROC_POWER));
    print_line("GFX Temperature", "%8.2f C", pmta(GFX_TEMP));
    print_line("GFX Clock Real | Effective", "%5.f MHz | %6.f MHz", pmta(GFX_FREQ), pmta(GFX_FREQEFF));
    print_line("GFX Busy", "%8.2f %%", pmta(GFX_BUSY) * 100.f);
    print_line("GFX EDC Limit | Residency", "%7.3f A | %8.2f %%", pmta(GFX_EDC_LIM), pmta(GFX_EDC_RESIDENCY) * 100.f);
    print_line("Display Count | FPS", "%2.f | %8.2f  ", pmta(DISPLAY_COUNT), pmta(FPS));
    print_line("DGPU Power | Freq Target | Busy", "%7.3f W | %5.f MHz | %8.2f %%", pmta(DGPU_POWER), pmta(DGPU_FREQ_TARGET), pmta(DGPU_GFX_BUSY) * 100.f);
    fprintf(stdout, "╰───────────────────────────────────────────────┴────────────────────────────────────────────────╯\n");
    }

    fprintf(stdout, "╭── Power Consumption ──────────────────────────┬────────────────────────────────────────────────╮\n");
    //These powers are drawn via VDDCR_SOC and VDDCR_CPU and thus are pulled from the CPU power connector of the mainboard
    print_line("Total Core Power Sum", "%7.3f W", total_core_power);
    //print_line("VDDCR_CPU Power", "%7.3f W", pmta(VDDCR_CPU_POWER)); //This value doesn't correlate with what the cores
                                                                        //report, nor with what is actually consumed. but is
                                                                        //the value HWiNFO shows.
    print_line("VDDCR_SOC Power", "%7.3f W", pmta(VDDCR_SOC_POWER));
    if(pmt->IO_VDDCR_SOC_POWER) print_line("IO VDDCR_SOC Power", "%7.3f W", pmta(IO_VDDCR_SOC_POWER));
    if(pmt->GMI2_VDDG_POWER) print_line("GMI2_VDDG Power", "%7.3f W", pmta(GMI2_VDDG_POWER));
    if(pmt->ROC_POWER) print_line("ROC Power", "%7.3f W", pmta(ROC_POWER));

    //L3 caches (2 per CCD on Zen2, 1 per CCD on Zen3)
    l3_logic_power=0;
    l3_vddm_power=0;
    for (i=0; i<pmt->max_l3; i++) {
        l3_logic_power += pmta0(L3_LOGIC_POWER[i]);
        l3_vddm_power += pmta0(L3_VDDM_POWER[i]);
    }
    if (pmt->max_l3 == 1) {
        print_line("L3 Logic Power", "%7.3f W", pmta(L3_LOGIC_POWER[0]));
        print_line("L3 VDDM Power", "%7.3f W", pmta(L3_VDDM_POWER[0]));
    } else {
        for (i=0; i<pmt->max_l3; i+=2) {
            // + sign if needed and first value
            j = snprintf(strbuf, sizeof(strbuf), "%s%7.3f W", (i?"+ ":""), pmta(L3_LOGIC_POWER[i]));
            // second value if it exists
            if (pmt->max_l3-i > 1) j += snprintf(strbuf+j, sizeof(strbuf)-j, " + %7.3f W", pmta(L3_LOGIC_POWER[i+1]));
            // end of string (sum or nothing)
            if (pmt->max_l3-i > 2) j += snprintf(strbuf+j, sizeof(strbuf)-j, "            ");
            else j += snprintf(strbuf+j, sizeof(strbuf)-j, " = %7.3f W", l3_logic_power);
            // print
            print_line((i?"":"L3 Logic Power"), "%s", strbuf);
        }
        for (i=0; i<pmt->max_l3; i+=2) {
            // + sign if needed and first value
            j = snprintf(strbuf, sizeof(strbuf), "%s%7.3f W", (i?"+ ":""), pmta(L3_VDDM_POWER[i]));
            // second value if it exists
            if (pmt->max_l3-i > 1) j += snprintf(strbuf+j, sizeof(strbuf)-j, " + %7.3f W", pmta(L3_VDDM_POWER[i+1]));
            // end of string (sum or nothing)
            if (pmt->max_l3-i > 2) j += snprintf(strbuf+j, sizeof(strbuf)-j, "            ");
            else j += snprintf(strbuf+j, sizeof(strbuf)-j, " = %7.3f W", l3_vddm_power);
            // print
            print_line((i?"":"L3 VDDM Power"), "%s", strbuf);
        }
    }

    //These powers are supplied by other power lines to the CPU and are drawn from the 24 pin ATX connector on most boards
    print_line("","");
    print_line("VDDIO_MEM Power", "%7.3f W", pmta(VDDIO_MEM_POWER));
    print_line("IOD_VDDIO_MEM Power", "%7.3f W", pmta(IOD_VDDIO_MEM_POWER));
    if(pmt->DDR_VDDP_POWER) print_line("DDR_VDDP Power", "%7.3f W", pmta(DDR_VDDP_POWER));
    if(pmt->DDR_PHY_POWER) print_line("DDR Phy Power", "%7.3f W", pmta(DDR_PHY_POWER));
    print_line("VDD18 Power", "%7.3f W", pmta(VDD18_POWER)); //Same as pmta(IO_VDD18_POWER)
    if(pmt->IO_DISPLAY_POWER) print_line("CPU Display IO Power", "%7.3f W", pmta(IO_DISPLAY_POWER));
    if(pmt->IO_USB_POWER) print_line("CPU USB IO Power", "%7.3f W", pmta(IO_USB_POWER));

    if(!pmt->powersum_unclear) {
    //The sum is the thermal output of the whole package. Yes, this is higher than PPT and SOCKET_POWER.
    //Confirmed by measuring the actual current draw on the mainboard.
    print_line("","");
    print_line("Calculated Thermal Output", "%7.3f W", total_core_power + pmta0(VDDCR_SOC_POWER) + pmta0(GMI2_VDDG_POWER) 
            + l3_logic_power + l3_vddm_power
            + pmta0(VDDIO_MEM_POWER) + pmta0(IOD_VDDIO_MEM_POWER) + pmta0(DDR_VDDP_POWER) + pmta0(VDD18_POWER));
    }

    fprintf(stdout, "├── Additional Reports ─────────────────────────┼────────────────────────────────────────────────┤\n");
    //print_line("ROC_POWER", "%7.4f",pmta(ROC_POWER));
    print_line("SoC Power (SVI2)", "%8.3f V | %7.3f A | %8.3f W", pmta(SOC_TELEMETRY_VOLTAGE), pmta(SOC_TELEMETRY_CURRENT), pmta(SOC_TELEMETRY_POWER));
    print_line("Core Power (SVI2)", "%8.3f V | %7.3f A | %8.3f W", pmta(CPU_TELEMETRY_VOLTAGE), pmta(CPU_TELEMETRY_CURRENT), pmta(CPU_TELEMETRY_POWER));
    print_line("Core Power (SMU)", "%7.3f W", pmta(VDDCR_CPU_POWER));
    print_line("Socket Power (SMU)", "%7.3f W", pmta(SOCKET_POWER));
    if (pmt->PACKAGE_POWER) print_line("Package Power (SMU)", "%7.3f W", pmta(PACKAGE_POWER));
    fprintf(stdout, "╰───────────────────────────────────────────────┴────────────────────────────────────────────────╯\n");
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

void start_pm_monitor(unsigned int force) {
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

    while(1) {
        if (smu_read_pm_table(&obj, pm_buf, obj.pm_table_size) != SMU_Return_OK)
            continue;

        fprintf(stdout, "\e[1;1H\e[2J"); //Move cursor to (1,1); Clear entire screen
        draw_screen(&pmt, &sysinfo);
        fprintf(stdout, "\e[?25l"); // Hide Cursor
        fflush(stdout);

        sleep(update_time_s);
    }
}

void read_from_dumpfile(char *dumpfile, unsigned int version) {
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

    draw_screen(&pmt, &sysinfo);
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
        program
    );
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
    int c=0, force=0, core=0, printtimings=0;
    char *dumpfile=0;

    //Set up signal handlers
    if ((signal(SIGABRT, signal_interrupt) == SIG_ERR) ||
        (signal(SIGTERM, signal_interrupt) == SIG_ERR) ||
        (signal(SIGINT, signal_interrupt) == SIG_ERR)) {
        fprintf(stderr, "Can't set up signal hooks.\n");
        exit(-1);
    }

    //Parse arguments
    while ((c = getopt(argc, argv, "vmd::f:t:u:h")) != -1) {
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

    if(dumpfile && !printtimings)
        read_from_dumpfile(dumpfile, force);
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
        else start_pm_monitor(force);
    }

    return 0;
}
