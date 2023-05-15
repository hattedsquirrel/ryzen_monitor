/**
 * Ryzen SMU Userspace Sensor Monitor
 * Copyright (C) 2020-2021
 *    Jeffrey Bosboom <jbosboom@jeffreybosboom.com>
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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "output_ops.h"

static void print_line(const char* label, const char* value_format, ...) {
    static char buffer[1024];
    va_list list;

    va_start(list, value_format);
    vsnprintf(buffer, sizeof(buffer), value_format, list);
    va_end(list);

    fprintf(stdout, "│ %45s │ %46s │\n", label, buffer);
}

static _Bool use_control_codes = 0;
static void box_drawing_init(_Bool repeating, _Bool interactive) {
    use_control_codes = repeating && interactive;
    if (use_control_codes)
        fprintf(stdout, "\e[?25l"); // Hide Cursor
}

static void box_drawing_cleanup() {
    if (use_control_codes)
        fprintf(stdout, "\e[?25h"); // Show Cursor
}

static void box_drawing_begin() {
    if (use_control_codes)
        fprintf(stdout, "\e[1;1H\e[2J"); //Move cursor to (1,1); Clear entire screen
}

static void box_drawing_end() {
    fflush(stdout);
}

static const char* const group_begin_strings[] = {
    [GROUP_SYSINFO] = "╭───────────────────────────────────────────────┬────────────────────────────────────────────────╮\n",
    [GROUP_CORES] = "╭─────────┬────────────┬──────────┬─────────┬──────────┬─────────────┬─────────────┬─────────────╮\n",
    [GROUP_CORE_STATS_CALC] = "╭── Core Statistics (Calculated) ───────────────┬────────────────────────────────────────────────╮\n",
    [GROUP_CORE_STATS_SMU] = "├── Reported by SMU ────────────────────────────┼────────────────────────────────────────────────┤\n",
    [GROUP_LIMITS] = "╭── Electrical & Thermal Constraints ───────────┬────────────────────────────────────────────────╮\n",
    [GROUP_MEMORY] = "╭── Memory Interface ───────────────────────────┬────────────────────────────────────────────────╮\n",
    [GROUP_GRAPHICS] = "╭── Graphics Subsystem──────────────────────────┬────────────────────────────────────────────────╮\n",
    [GROUP_POWER] = "╭── Power Consumption ──────────────────────────┬────────────────────────────────────────────────╮\n",
    [GROUP_POWER_REPORTS] = "├── Additional Reports ─────────────────────────┼────────────────────────────────────────────────┤\n",
};

static const char usual_end_string[] = "╰───────────────────────────────────────────────┴────────────────────────────────────────────────╯\n";
static const char* const group_end_strings[] = {
    [GROUP_SYSINFO] = usual_end_string,
    [GROUP_CORES] = "╰─────────┴────────────┴──────────┴─────────┴──────────┴─────────────┴─────────────┴─────────────╯\n",
    [GROUP_CORE_STATS_CALC] = "",
    [GROUP_CORE_STATS_SMU] = usual_end_string,
    [GROUP_LIMITS] = usual_end_string,
    [GROUP_MEMORY] = usual_end_string,
    [GROUP_GRAPHICS] = usual_end_string,
    [GROUP_POWER] = "",
    [GROUP_POWER_REPORTS] = usual_end_string,
};

static void box_drawing_begin_group(enum group group) {
    fprintf(stdout, "%s", group_begin_strings[group]);
}

static void box_drawing_end_group(enum group group) {
    fprintf(stdout, "%s", group_end_strings[group]);
}

static const char* const datum_strings[] = {
    [DATUM_MODEL] = "CPU Model",
    [DATUM_CODENAME] = "Processor Code Name",
    [DATUM_CORES] = "Cores",
    [DATUM_CCDS] = "Core CCDs",
    [DATUM_CCXS] = "Core CCXs",
    [DATUM_CORES_PER_CCX] = "Cores Per CCX",
    [DATUM_CORES_PER_CCD] = "Cores Per CCD",
    [DATUM_SMU_FW_VERSION] = "SMU FW Version",
    [DATUM_MP1_IF_VERSION] = "MP1 IF Version",

    [DATUM_PEAK_CORE_FREQ] = "Highest Effective Core Frequency",
    [DATUM_PEAK_CORE_TEMP] = "Highest Core Temperature",
    [DATUM_PEAK_CORE_VOLTAGE] = "Highest Core Voltage",
    [DATUM_AVG_CORE_VOLTAGE] = "Average Core Voltage",
    [DATUM_AVG_CORE_C6] = "Average Core CC6",
    [DATUM_TOTAL_CORE_POWER] = "Total Core Power Sum",

    [DATUM_PEAK_CORE_VOLTAGE_SMU] = "Peak Core Voltage",
    [DATUM_PACKAGE_C6_SMU] = "Package CC6",

    [DATUM_PEAK_TEMP] = "Peak Temperature",
    [DATUM_SOC_TEMP] = "SoC Temperature",
    [DATUM_GFX_TEMP] = "GFX Temperature",
    [DATUM_CORE_VRM_VOLTAGE_LIMIT] = "Voltage from Core VRM",
    [DATUM_PPT_LIMIT] = "PPT",
    [DATUM_PPT_APU_LIMIT] = "PPT APU",
    [DATUM_TDC_LIMIT_NOMINAL] = "TDC Value",
    [DATUM_TDC_LIMIT_ACTUAL] = "TDC Actual",
    [DATUM_TDC_SOC_LIMIT_NOMINAL] = "TDC Value, SoC only",
    [DATUM_EDC_LIMIT] = "EDC",
    [DATUM_EDC_SOC_LIMIT] = "EDC, SoC only",
    [DATUM_THM_LIMIT] = "THM",
    [DATUM_THM_SOC_LIMIT] = "THM SoC",
    [DATUM_THM_GFX_LIMIT] = "THM GFX",
    [DATUM_FIT_LIMIT] = "FIT",

    [DATUM_MEMORY_COUPLED] = "Coupled Mode",
    [DATUM_FCLK_AVG] = "Fabric Clock (Average)",
    [DATUM_FCLK] = "Fabric Clock",
    [DATUM_UCLK] = "Uncore Clock",
    [DATUM_MCLK] = "Memory Clock",
    [DATUM_VDDM] = "cLDO_VDDM",
    [DATUM_VDDP] = "cLDO_VDDP",
    [DATUM_VDDG] = "cLDO_VDDG",
    [DATUM_VDDG_IOD] = "cLDO_VDDG_IOD",
    [DATUM_VDDG_CCD] = "cLDO_VDDG_CCD",

    [DATUM_GFX_VOLTAGE_ROC_POWER] = "GFX Voltage | ROC Power",
    [DATUM_GFX_FREQ_REAL_EFF] = "GFX Clock Real | Effective",
    [DATUM_GFX_BUSY] = "GFX Busy",
    [DATUM_GFX_EDC_LIMIT_RESIDENCY] = "GFX EDC Limit | Residency",
    [DATUM_GFX_DISPLAY_COUNT_FPS] = "Display Count | FPS",
    [DATUM_GFX_DGPU_POWER_FREQ_TARGET_BUSY] = "DGPU Power | Freq Target | Busy",

    [DATUM_VDDR_SOC_POWER] = "VDDCR_SOC Power",
    [DATUM_IO_VDDR_SOC_POWER] = "IO VDDCR_SOC Power",
    [DATUM_GMI2_VDDG_POWER] = "GMI2_VDDG Power",
    [DATUM_ROC_POWER] = "ROC Power",
    [DATUM_L3_LOGIC_POWER] = "L3 Logic Power",
    [DATUM_L3_VDDM_POWER] = "L3 VDDM Power",
    [DATUM_VDDIO_MEM_POWER] = "VDDIO_MEM Power",
    [DATUM_IOD_VDDIO_MEM_POWER] = "IOD_VDDIO_MEM Power",
    [DATUM_DDR_VDDP_POWER] = "DDR_VDDP Power",
    [DATUM_DDR_PHY_POWER] = "DDR Phy Power",
    [DATUM_VDD18_POWER] = "VDD18 Power",
    [DATUM_IO_DISPLAY_POWER] = "CPU Display IO Power",
    [DATUM_IO_USB_POWER] = "CPU USB IO Power",
    [DATUM_CALC_TOTAL_POWER] = "Calculated Thermal Output",

    [DATUM_SVI2_SOC_POWER] = "SoC Power (SVI2)",
    [DATUM_SVI2_CORE_POWER] = "Core Power (SVI2)",
    [DATUM_SMU_CORE_POWER] = "Core Power (SMU)",
    [DATUM_SMU_SOCKET_POWER] = "Socket Power (SMU)",
    [DATUM_SMU_PACKAGE_POWER] = "Package Power (SMU)",
};

static enum datum const blank_line_before[] = {
    DATUM_VDDIO_MEM_POWER,
    DATUM_CALC_TOTAL_POWER,
};
static void maybe_print_blank_line(enum datum datum) {
    for (int i = 0; i < sizeof(blank_line_before) / sizeof(blank_line_before[0]); ++i)
        if (blank_line_before[i] == datum)
            print_line("", "");
}

static void box_drawing_datum_string(enum datum datum, const char* value) {
    print_line(datum_strings[datum], "%s", value);
}

static void box_drawing_datum_bool(enum datum datum, _Bool value) {
    box_drawing_datum_string(datum, value ? "ON" : "OFF");
}

static void box_drawing_datum_int(enum datum datum, int value, enum unit unit) {
    assert(unit == UNIT_COUNT);
    print_line(datum_strings[datum], "%d", value);
}

static void box_drawing_datum_float(enum datum datum, float value, enum unit unit) {
    maybe_print_blank_line(datum);
    const char* label = datum_strings[datum];
    const char* format = "";
    switch (unit) {
        case UNIT_COUNT: assert(0 && "datum_float count"); break;
        case UNIT_RATIO: format = "%6.2f %%"; value *= 100.f; break;
        case UNIT_MEGAHERTZ: format = "%8.0f MHz"; break;
        case UNIT_VOLTS: format = "%7.4f V"; break;
        case UNIT_AMPS: assert(0 && "datum_float amps"); break;
        case UNIT_WATTS: format = "%7.3f W"; break;
        case UNIT_CELSIUS: format = "%8.2f C"; break;
    }
    print_line(label, format, value);
}

static void box_drawing_datum_float2(enum datum datum, float value1, enum unit unit1, float value2, enum unit unit2) {
    const char* label = datum_strings[datum];
    const char* format = "";
    if (unit1 == UNIT_VOLTS && unit2 == UNIT_WATTS) format = "%7.4f V | %8.3f W";
    else if (unit1 == UNIT_MEGAHERTZ && unit2 == UNIT_MEGAHERTZ) format = "%5.f MHz | %6.f MHz";
    else if (unit1 == UNIT_AMPS && unit2 == UNIT_RATIO) format = "%7.3f A | %8.2f %%";
    else if (unit1 == UNIT_COUNT && unit2 == UNIT_COUNT) format = "%2.f | %8.2f  ";
    else {assert(0 && "datum_float2 unhandled unit pair");}
    if (unit1 == UNIT_RATIO) value1 *= 100.f;
    if (unit2 == UNIT_RATIO) value2 *= 100.f;
    print_line(label, format, value1, value2);
}

static void box_drawing_datum_float3(enum datum datum, float value1, enum unit unit1, float value2, enum unit unit2, float value3, enum unit unit3) {
    const char* label = datum_strings[datum];
    const char* format = "";
    if (unit1 == UNIT_VOLTS && unit2 == UNIT_AMPS && unit3 == UNIT_WATTS) format = "%8.3f V | %7.3f A | %8.3f W";
    else if (unit1 == UNIT_WATTS && unit2 == UNIT_MEGAHERTZ && unit3 == UNIT_RATIO) format = "%7.3f W | %5.f MHz | %8.2f %%";
    else {assert(0 && "datum_float3 unhandled unit triple");}
    if (unit1 == UNIT_RATIO) value1 *= 100.f;
    if (unit2 == UNIT_RATIO) value2 *= 100.f;
    if (unit3 == UNIT_RATIO) value3 *= 100.f;
    print_line(label, format, value1, value2, value3);
}

static void box_drawing_datum_limit(enum datum datum, float value, float limit, enum unit unit) {
    const char* label = datum_strings[datum];
    const char* format = "";
    switch (unit) {
        case UNIT_COUNT: format = "%7.f   | %7.f   | %8.2f %%"; break;
        case UNIT_RATIO: assert(0 && "datum_limit ratio"); break;
        case UNIT_MEGAHERTZ: assert(0 && "datum_limit megahertz"); break;
        case UNIT_VOLTS: format = "%7.3f V | %7.3f V | %8.2f %%"; break;
        case UNIT_AMPS: format = "%7.3f A | %7.f A | %8.2f %%"; break;
        case UNIT_WATTS: format = "%7.3f W | %7.f W | %8.2f %%"; break;
        case UNIT_CELSIUS: format = "%7.2f C | %7.f C | %8.2f %%"; break;
    }
    print_line(label, format, value, limit, (value / limit) * 100.f);
}

static void box_drawing_datum_sum(enum datum datum, const float* begin, const float* end, enum unit unit) {
    if (end - begin == 1)
        box_drawing_datum_float(datum, *begin, UNIT_WATTS);
    else if (end - begin == 2)
        print_line(datum_strings[datum], "%7.3f W + %7.3f W = %7.3f W", begin[0], begin[1], begin[0]+begin[1]);
    else if (end - begin == 4) {
        print_line(datum_strings[datum], "%7.3f W + %7.3f W = %7s", begin[0], begin[1], "");
        print_line("", "%7.3f W + %7.3f W = %7.3f W", begin[2], begin[3], begin[0]+begin[1]+begin[2]+begin[3]);
    } else {
        assert(0 && "datum_sum unhandled length");
        print_line(datum_strings[datum], "unhandled length %d", end - begin);
    }
}

static void box_drawing_core(int number, _Bool disabled, _Bool sleeping, float frequency,
        float power, float voltage, float temperature,
        float c0_percent, float c1_percent, float c6_percent) {
    char number_str[8];
    snprintf(number_str, sizeof(number_str), "Core %d", number);

    char frequency_str_buf[10];
    const char* frequency_str = frequency_str_buf;
    if (disabled)
        frequency_str = "Disabled";
    else if (sleeping)
        frequency_str = "Sleeping";
    else
        snprintf(frequency_str_buf, sizeof(frequency_str_buf), "%4.f MHz", frequency);

    fprintf(stdout, "│ %7s │ %10s | %6.3f W | %5.3f V | %6.2f C | C0: %5.1f %% | C1: %5.1f %% | C6: %5.1f %% │\n",
            number_str, frequency_str, power, voltage, temperature, c0_percent, c1_percent, c6_percent);
}

const struct output_ops box_drawing_ops = {
    .init = box_drawing_init,
    .cleanup = box_drawing_cleanup,
    .begin = box_drawing_begin,
    .end = box_drawing_end,
    .begin_group = box_drawing_begin_group,
    .end_group = box_drawing_end_group,
    .datum_string = box_drawing_datum_string,
    .datum_bool = box_drawing_datum_bool,
    .datum_int = box_drawing_datum_int,
    .datum_float = box_drawing_datum_float,
    .datum_float2 = box_drawing_datum_float2,
    .datum_float3 = box_drawing_datum_float3,
    .datum_limit = box_drawing_datum_limit,
    .datum_sum = box_drawing_datum_sum,
    .core = box_drawing_core,
};