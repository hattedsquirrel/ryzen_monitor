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
#include <stdio.h>
#include <time.h>

#include "output_ops.h"

// floats are written with high precision to allow differentiating between
// values that are similar but different in each sample (probably measured)
// and values that are exactly the same in each sample (static or calculated).

// JSON does not allow trailing commas, so we write commas before each
// non-initial element in an object.  This requires remembering whether the
// current element in each scope is the initial element.  Our objects have only
// two scope levels (the top level and each group), so we just use two counters
// instead of a proper stack.
static int elements_in_top_level, elements_in_group;
static void maybe_comma(int* counter) {
    if ((*counter)++)
        fprintf(stdout, ",");
}

static void json_init(_Bool repeating, _Bool interactive) {}
static void json_cleanup() {}

static void json_begin() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    fprintf(stdout, "{\"timestamp\":%ld", t.tv_sec * 1000000000 + t.tv_nsec);
    elements_in_top_level = 1;
}

static void json_end() {
    fprintf(stdout, "}");
    fflush(stdout);
}

static void ndjson_end() {
    json_end();
    fprintf(stdout, "\n");
    fflush(stdout);
}

static const char* const group_keys[] = {
    [GROUP_SYSINFO] = "sysinfo",
    [GROUP_CORES] = "cores",
    [GROUP_CORE_STATS_CALC] = "core_stats_calc",
    [GROUP_CORE_STATS_SMU] = "core_stats_reports",
    [GROUP_LIMITS] = "limits",
    [GROUP_MEMORY] = "memory",
    [GROUP_GRAPHICS] = "graphics",
    [GROUP_POWER] = "power",
    [GROUP_POWER_REPORTS] = "power_reports",
};

static void json_begin_group(enum group group) {
    maybe_comma(&elements_in_top_level);
    if (group == GROUP_CORES)
        fprintf(stdout, "\"%s\":[", group_keys[group]);
    else
        fprintf(stdout, "\"%s\":{", group_keys[group]);
    elements_in_group = 0;
}

static void json_end_group(enum group group) {
    if (group == GROUP_CORES)
        fprintf(stdout, "]");
    else
        fprintf(stdout, "}");
}

static const char* const datum_keys[] = {
    [DATUM_MODEL] = "model",
    [DATUM_CODENAME] = "codename",
    [DATUM_CORES] = "cores",
    [DATUM_CCDS] = "ccds",
    [DATUM_CCXS] = "ccxs",
    [DATUM_CORES_PER_CCX] = "cores_per_ccx",
    [DATUM_CORES_PER_CCD] = "cores_per_ccd",
    [DATUM_SMU_FW_VERSION] = "smu_firmware_version",
    [DATUM_MP1_IF_VERSION] = "mp1_if_version",

    [DATUM_PEAK_CORE_FREQ] = "peak_freq",
    [DATUM_PEAK_CORE_TEMP] = "peak_temp",
    [DATUM_PEAK_CORE_VOLTAGE] = "peak_voltage",
    [DATUM_AVG_CORE_VOLTAGE] = "avg_voltage",
    [DATUM_AVG_CORE_C6] = "avg_cc6",
    [DATUM_TOTAL_CORE_POWER] = "total_power",

    [DATUM_PEAK_CORE_VOLTAGE_SMU] = "smu_peak_voltage",
    [DATUM_PACKAGE_C6_SMU] = "smu_package_c6",

    [DATUM_PEAK_TEMP] = "peak_temp",
    [DATUM_SOC_TEMP] = "soc_temp",
    [DATUM_GFX_TEMP] = "gfx_temp",
    [DATUM_CORE_VRM_VOLTAGE_LIMIT] = "core_voltage",
    [DATUM_PPT_LIMIT] = "ppt",
    [DATUM_PPT_APU_LIMIT] = "ppt_apu",
    [DATUM_TDC_LIMIT_NOMINAL] = "tdc_nominal",
    [DATUM_TDC_LIMIT_ACTUAL] = "tdc_actual",
    [DATUM_TDC_SOC_LIMIT_NOMINAL] = "tdc_soc_nominal",
    [DATUM_EDC_LIMIT] = "edc",
    [DATUM_EDC_SOC_LIMIT] = "edc_soc",
    [DATUM_THM_LIMIT] = "thm",
    [DATUM_THM_SOC_LIMIT] = "thm_soc",
    [DATUM_THM_GFX_LIMIT] = "thm_gfx",
    [DATUM_FIT_LIMIT] = "fit",

    [DATUM_MEMORY_COUPLED] = "coupled",
    [DATUM_FCLK_AVG] = "fclk_avg",
    [DATUM_FCLK] = "fclk",
    [DATUM_UCLK] = "uclk",
    [DATUM_MCLK] = "mclk",
    [DATUM_VDDM] = "cldo_vddm",
    [DATUM_VDDP] = "cldo_vddp",
    [DATUM_VDDG] = "cldo_vddg",
    [DATUM_VDDG_IOD] = "cldo_vddg_iod",
    [DATUM_VDDG_CCD] = "cldo_vddg_ccd",

    [DATUM_GFX_VOLTAGE_ROC_POWER] = "unused (split)",
    [DATUM_GFX_FREQ_REAL_EFF] = "unused (split)",
    [DATUM_GFX_BUSY] = "busy",
    [DATUM_GFX_EDC_LIMIT_RESIDENCY] = "unused (split)",
    [DATUM_GFX_DISPLAY_COUNT_FPS] = "unused (split)",
    [DATUM_GFX_DGPU_POWER_FREQ_TARGET_BUSY] = "unused (split)",

    [DATUM_VDDR_SOC_POWER] = "vddr_soc",
    [DATUM_IO_VDDR_SOC_POWER] = "io_vddr_soc",
    [DATUM_GMI2_VDDG_POWER] = "gmi2_vddg",
    [DATUM_ROC_POWER] = "roc",
    [DATUM_L3_LOGIC_POWER] = "l3_logic",
    [DATUM_L3_VDDM_POWER] = "l3_vddm",
    [DATUM_VDDIO_MEM_POWER] = "vddio_mem",
    [DATUM_IOD_VDDIO_MEM_POWER] = "iod_vddio_mem",
    [DATUM_DDR_VDDP_POWER] = "ddr_vddp",
    [DATUM_DDR_PHY_POWER] = "ddr_phy",
    [DATUM_VDD18_POWER] = "vdd18",
    [DATUM_IO_DISPLAY_POWER] = "cpu_display_io",
    [DATUM_IO_USB_POWER] = "cpu_usb_io",
    [DATUM_CALC_TOTAL_POWER] = "calc_total",

    [DATUM_SVI2_SOC_POWER] = "svi2_soc",
    [DATUM_SVI2_CORE_POWER] = "svi2_core",
    [DATUM_SMU_CORE_POWER] = "smu_core_power",
    [DATUM_SMU_SOCKET_POWER] = "smu_socket_power",
    [DATUM_SMU_PACKAGE_POWER] = "smu_package_power",
};

static const struct split_datum {
    enum datum datum;
    const char *key1, *key2, *key3;
} split_datums[] = {
    {DATUM_GFX_VOLTAGE_ROC_POWER, "voltage", "roc_power", NULL},
    {DATUM_GFX_FREQ_REAL_EFF, "freq", "freq_effective", NULL},
    {DATUM_GFX_EDC_LIMIT_RESIDENCY, "edc_limit", "edc_residency", ""},
    {DATUM_GFX_DISPLAY_COUNT_FPS, "display_count", "fps", ""},
    {DATUM_GFX_DGPU_POWER_FREQ_TARGET_BUSY, "dgpu_power", "dgpu_freq_target", "dgpu_busy"},
};

static const struct fused_datum_subkeys {
    enum datum datum;
    const char *key1, *key2, *key3;
} fused_datum_subkeys[] = {
    {DATUM_SVI2_SOC_POWER, "voltage", "current", "power"},
    {DATUM_SVI2_CORE_POWER, "voltage", "current", "power"},
};

static void json_datum_string(enum datum datum, const char* value) {
    maybe_comma(&elements_in_group);
    fprintf(stdout, "\"%s\":\"%s\"", datum_keys[datum], value);
}

static void json_datum_bool(enum datum datum, _Bool value) {
    maybe_comma(&elements_in_group);
    fprintf(stdout, "\"%s\":%s", datum_keys[datum], value ? "true" : "false");
}

static void json_datum_int(enum datum datum, int value, enum unit unit) {
    maybe_comma(&elements_in_group);
    fprintf(stdout, "\"%s\":%d", datum_keys[datum], value);
}

static void json_single_labeled_float(const char* label, float value) {
    maybe_comma(&elements_in_group);
    fprintf(stdout, "\"%s\":%.17f", label, value);
}

static void json_datum_float(enum datum datum, float value, enum unit unit) {
    json_single_labeled_float(datum_keys[datum], value);
}

static void json_datum_float3(enum datum datum, float value1, enum unit unit1, float value2, enum unit unit2, float value3, enum unit unit3) {
    for (int i = 0; i < sizeof(split_datums)/sizeof(split_datums[0]); ++i)
        if (split_datums[i].datum == datum) {
            if (split_datums[i].key1) json_single_labeled_float(split_datums[i].key1, value1);
            if (split_datums[i].key2) json_single_labeled_float(split_datums[i].key2, value2);
            if (split_datums[i].key3) json_single_labeled_float(split_datums[i].key3, value3);
            return;
        }
    for (int i = 0; i < sizeof(fused_datum_subkeys)/sizeof(fused_datum_subkeys[0]); ++i)
        if (fused_datum_subkeys[i].datum == datum) {
            maybe_comma(&elements_in_group);
            fprintf(stdout, "\"%s\":{\"%s\":%.17f,\"%s\":%.17f,\"%s\":%.17f}", datum_keys[datum],
                    fused_datum_subkeys[i].key1, value1,
                    fused_datum_subkeys[i].key2, value2,
                    fused_datum_subkeys[i].key3, value3);
            return;
        }
    assert(0);
}

static void json_datum_float2(enum datum datum, float value1, enum unit unit1, float value2, enum unit unit2) {
    json_datum_float3(datum, value1, unit1, value2, unit2, 0.f, UNIT_COUNT);
}

static void json_datum_limit(enum datum datum, float value, float limit, enum unit unit) {
    maybe_comma(&elements_in_group);
    fprintf(stdout, "\"%s\":{\"value\":%.17f,\"limit\":%.17f,\"usage\":%.17f}",
            datum_keys[datum], value, limit, value / limit);
}

static void json_datum_sum(enum datum datum, const float* begin, const float* end, enum unit unit) {
    maybe_comma(&elements_in_group);
    if (end - begin == 1)
        fprintf(stdout, "\"%s\":{\"values\":[%.17f],\"sum\":%.17f}", datum_keys[datum], begin[0], begin[0]);
    else if (end - begin == 2)
        fprintf(stdout, "\"%s\":{\"values\":[%.17f,%.17f],\"sum\":%.17f}", datum_keys[datum],
                begin[0], begin[1], begin[0]+begin[1]);
    else if (end - begin == 4)
        fprintf(stdout, "\"%s\":{\"values\":[%.17f,%.17f,%.17f,%.17f],\"sum\":%.17f}", datum_keys[datum],
                begin[0], begin[1], begin[2], begin[3], begin[0]+begin[1]+begin[2]+begin[3]);
    else
        assert(0 && "datum_sum unhandled length");
}

static void json_core(int number, _Bool disabled, _Bool sleeping, float frequency,
        float power, float voltage, float temperature,
        float c0_percent, float c1_percent, float c6_percent) {
    maybe_comma(&elements_in_group);
    fprintf(stdout, "{\"number\":%d,\"disabled\":%s,\"sleeping\":%s,\"frequency\":%.17f,"
            "\"power\":%.17f,\"voltage\":%.17f,\"temp\":%.17f,"
            "\"c0\":%.17f,\"c1\":%.17f,\"c6\":%.17f}",
            number, disabled ? "true" : "false", sleeping ? "true" : "false", frequency,
            power, voltage, temperature,
            c0_percent / 100.f, c1_percent / 100.f, c6_percent / 100.f);
}

const struct output_ops json_ops = {
    .init = json_init,
    .cleanup = json_cleanup,
    .begin = json_begin,
    .end = json_end,
    .begin_group = json_begin_group,
    .end_group = json_end_group,
    .datum_string = json_datum_string,
    .datum_bool = json_datum_bool,
    .datum_int = json_datum_int,
    .datum_float = json_datum_float,
    .datum_float2 = json_datum_float2,
    .datum_float3 = json_datum_float3,
    .datum_limit = json_datum_limit,
    .datum_sum = json_datum_sum,
    .core = json_core,
};

const struct output_ops ndjson_ops = {
    .init = json_init,
    .cleanup = json_cleanup,
    .begin = json_begin,
    .end = ndjson_end,
    .begin_group = json_begin_group,
    .end_group = json_end_group,
    .datum_string = json_datum_string,
    .datum_bool = json_datum_bool,
    .datum_int = json_datum_int,
    .datum_float = json_datum_float,
    .datum_float2 = json_datum_float2,
    .datum_float3 = json_datum_float3,
    .datum_limit = json_datum_limit,
    .datum_sum = json_datum_sum,
    .core = json_core,
};