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

enum group {
    GROUP_SYSINFO,
    GROUP_CORES,
    GROUP_CORE_STATS_CALC,
    GROUP_CORE_STATS_SMU,
    GROUP_LIMITS,
    GROUP_MEMORY,
    GROUP_GRAPHICS,
    GROUP_POWER,
    GROUP_POWER_REPORTS,
};

enum datum {
    DATUM_MODEL,
    DATUM_CODENAME,
    DATUM_CORES,
    DATUM_CCDS,
    DATUM_CCXS,
    DATUM_CORES_PER_CCX,
    DATUM_CORES_PER_CCD,
    DATUM_SMU_FW_VERSION,
    DATUM_MP1_IF_VERSION,

    DATUM_PEAK_CORE_FREQ,
    DATUM_PEAK_CORE_TEMP,
    DATUM_PEAK_CORE_VOLTAGE,
    DATUM_AVG_CORE_VOLTAGE,
    DATUM_AVG_CORE_C6,
    DATUM_TOTAL_CORE_POWER,

    DATUM_PEAK_CORE_VOLTAGE_SMU,
    DATUM_PACKAGE_C6_SMU,

    DATUM_PEAK_TEMP,
    DATUM_SOC_TEMP,
    DATUM_GFX_TEMP,
    DATUM_CORE_VRM_VOLTAGE_LIMIT,
    DATUM_PPT_LIMIT,
    DATUM_PPT_APU_LIMIT,
    DATUM_TDC_LIMIT_NOMINAL,
    DATUM_TDC_LIMIT_ACTUAL,
    DATUM_TDC_SOC_LIMIT_NOMINAL,
    DATUM_EDC_LIMIT,
    DATUM_EDC_SOC_LIMIT,
    DATUM_THM_LIMIT,
    DATUM_THM_SOC_LIMIT,
    DATUM_THM_GFX_LIMIT,
    DATUM_FIT_LIMIT,

    DATUM_MEMORY_COUPLED,
    DATUM_FCLK_AVG,
    DATUM_FCLK,
    DATUM_UCLK,
    DATUM_MCLK,
    DATUM_VDDM,
    DATUM_VDDP,
    DATUM_VDDG,
    DATUM_VDDG_IOD,
    DATUM_VDDG_CCD,

    DATUM_GFX_VOLTAGE_ROC_POWER,
    DATUM_GFX_FREQ_REAL_EFF,
    DATUM_GFX_BUSY,
    DATUM_GFX_EDC_LIMIT_RESIDENCY,
    DATUM_GFX_DISPLAY_COUNT_FPS,
    DATUM_GFX_DGPU_POWER_FREQ_TARGET_BUSY,

    DATUM_VDDR_SOC_POWER,
    DATUM_IO_VDDR_SOC_POWER,
    DATUM_GMI2_VDDG_POWER,
    DATUM_ROC_POWER,
    DATUM_L3_LOGIC_POWER,
    DATUM_L3_VDDM_POWER,
    DATUM_VDDIO_MEM_POWER,
    DATUM_IOD_VDDIO_MEM_POWER,
    DATUM_DDR_VDDP_POWER,
    DATUM_DDR_PHY_POWER,
    DATUM_VDD18_POWER,
    DATUM_IO_DISPLAY_POWER,
    DATUM_IO_USB_POWER,
    DATUM_CALC_TOTAL_POWER,

    DATUM_SVI2_SOC_POWER,
    DATUM_SVI2_CORE_POWER,
    DATUM_SMU_CORE_POWER,
    DATUM_SMU_SOCKET_POWER,
    DATUM_SMU_PACKAGE_POWER,
};

enum unit {
    UNIT_COUNT,
    UNIT_RATIO,
    // Most frequencies in the tables are MHz, except for core frequency which
    // is in GHz.  There's no point in converting to Hz and back.
    UNIT_MEGAHERTZ,
    UNIT_VOLTS,
    UNIT_AMPS,
    UNIT_WATTS,
    UNIT_CELSIUS,
};

struct output_ops {
    void (*init)(_Bool repeating, _Bool interactive);
    void (*cleanup)();

    void (*begin)();
    void (*end)();

    void (*begin_group)(enum group group);
    void (*end_group)(enum group group);

    void (*datum_string)(enum datum datum, const char* value);
    void (*datum_bool)(enum datum datum, _Bool value);
    void (*datum_int)(enum datum datum, int value, enum unit unit);
    void (*datum_float)(enum datum datum, float value, enum unit unit);
    void (*datum_float2)(enum datum datum, float value1, enum unit unit1, float value2, enum unit unit2);
    void (*datum_float3)(enum datum datum, float value1, enum unit unit1, float value2, enum unit unit2, float value3, enum unit unit3);
    void (*datum_limit)(enum datum datum, float value, float limit, enum unit unit);
    void (*datum_sum)(enum datum datum, const float* begin, const float* end, enum unit unit);

    void (*core)(int number, _Bool disabled, _Bool sleeping, float frequency, float power, float voltage, float temperature, float c0_percent, float c1_percent, float c6_percent);
};

extern const struct output_ops box_drawing_ops;
extern const struct output_ops json_ops;
extern const struct output_ops ndjson_ops;