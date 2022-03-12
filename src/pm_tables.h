/**
 * Ryzen SMU Userspace Sensor Monitor
 * Copyright (C) 2021-2022
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

#ifndef pm_tables_h
#define pm_tables_h

#define PMT_MAX_NUM_L3      4
#define PMT_MAX_NUM_CORES   16

typedef struct {
    unsigned int version;  //PM table version
    int max_cores;         //Number of cores supported by the PM table
    int max_l3;            //Number of L3 caches supported by the PM table
    int zen_version;       //Zen, Zen2 or Zen3?
    unsigned int min_size; //Size of the selected PM table version
    int experimental;      //1 = Print experimental note
    int powersum_unclear;  //1 = No idea how to calculate the total power
    int has_graphics;      //1 = Has internal graphics

    float *STAPM_LIMIT;
    float *STAPM_VALUE;
    float *PPT_LIMIT;
    float *PPT_VALUE;
    float *PPT_LIMIT_FAST;
    float *PPT_VALUE_FAST;
    float *PPT_LIMIT_APU;
    float *PPT_VALUE_APU;
    float *TDC_LIMIT;
    float *TDC_VALUE;
    float *TDC_LIMIT_SOC;
    float *TDC_VALUE_SOC;
    float *THM_LIMIT;
    float *THM_VALUE;
    float *THM_LIMIT_SOC;
    float *THM_VALUE_SOC;
    float *THM_LIMIT_GFX;
    float *THM_VALUE_GFX;
    float *FIT_LIMIT;
    float *FIT_VALUE;
    float *EDC_LIMIT;
    float *EDC_VALUE;
    float *EDC_LIMIT_SOC;
    float *EDC_VALUE_SOC;
    float *VID_LIMIT;
    float *VID_VALUE;
    float *PPT_WC;
    float *PPT_ACTUAL;
    float *TDC_WC;
    float *TDC_ACTUAL;
    float *THM_WC;
    float *THM_ACTUAL;
    float *FIT_WC;
    float *FIT_ACTUAL;
    float *EDC_WC;
    float *EDC_ACTUAL;
    float *VID_WC;
    float *VID_ACTUAL;
    float *VDDCR_CPU_POWER;
    float *VDDCR_SOC_POWER;
    float *VDDIO_MEM_POWER;
    float *VDD18_POWER;
    float *ROC_POWER;
    float *SOCKET_POWER;
    float *CCLK_GLOBAL_FREQ;
    float *GLOB_FREQUENCY;
    float *STAPM_FREQUENCY;
    float *PPT_FREQUENCY;
    float *PPT_FREQUENCY_FAST;
    float *PPT_FREQUENCY_APU;
    float *TDC_FREQUENCY;
    float *THM_FREQUENCY;
    float *HTFMAX_FREQUENCY;
    float *PROCHOT_FREQUENCY;
    float *VOLTAGE_FREQUENCY;
    float *CCA_FREQUENCY;
    float *FIT_VOLTAGE;
    float *FIT_PRE_VOLTAGE;
    float *LATCHUP_VOLTAGE;
    float *CPU_SET_VOLTAGE;
    float *CPU_TELEMETRY_VOLTAGE;
    float *CPU_TELEMETRY_VOLTAGE2; 
    float *CPU_TELEMETRY_CURRENT;
    float *CPU_TELEMETRY_POWER;
    float *SOC_SET_VOLTAGE;
    float *SOC_TELEMETRY_VOLTAGE;
    float *SOC_TELEMETRY_CURRENT;
    float *SOC_TELEMETRY_POWER;
    float *FCLK_FREQ;
    float *FCLK_FREQ_EFF;
    float *UCLK_FREQ;
    float *MEMCLK_FREQ;
    float *FCLK_DRAM_SETPOINT;
    float *FCLK_DRAM_BUSY;
    float *FCLK_GMI_SETPOINT;
    float *FCLK_GMI_BUSY;
    float *FCLK_IOHC_SETPOINT;
    float *FCLK_IOHC_BUSY;
    float *FCLK_MEM_LATENCY_SETPOINT;
    float *FCLK_MEM_LATENCY;
    float *FCLK_CCLK_SETPOINT;
    float *FCLK_CCLK_FREQ;
    float *FCLK_XGMI_SETPOINT;
    float *FCLK_XGMI_BUSY;
    float *CCM_READS;
    float *CCM_WRITES;
    float *IOMS;
    float *XGMI;
    float *CS_UMC_READS;
    float *CS_UMC_WRITES;
    float *FCLK_RESIDENCY[4];
    float *FCLK_FREQ_TABLE[4];
    float *UCLK_FREQ_TABLE[4];
    float *MEMCLK_FREQ_TABLE[4];
    float *FCLK_VOLTAGE[4];
    float *LCLK_SETPOINT[4];
    float *LCLK_BUSY[4];
    float *LCLK_FREQ[4];
    float *LCLK_FREQ_EFF[4];
    float *LCLK_MAX_DPM[4];
    float *LCLK_MIN_DPM[4];
    float *SOCCLK_FREQ_EFF[4];
    float *SHUBCLK_FREQ_EFF[4];
    float *XGMI_SETPOINT;
    float *XGMI_BUSY;
    float *XGMI_LANE_WIDTH;
    float *XGMI_DATA_RATE;
    float *SOC_POWER;
    float *SOC_TEMP;
    float *DDR_VDDP_POWER;
    float *DDR_VDDIO_MEM_POWER;
    float *GMI2_VDDG_POWER;
    float *IO_VDDCR_SOC_POWER;
    float *IOD_VDDIO_MEM_POWER;
    float *IO_VDD18_POWER; 
    float *TDP;
    float *DETERMINISM;
    float *V_VDDM;
    float *V_VDDP;
    float *V_VDDG;
    float *V_VDDG_IOD;
    float *V_VDDG_CCD;
    float *PEAK_TEMP;
    float *PEAK_VOLTAGE;
    float *PEAK_CCLK_FREQ;
    float *unk_power;
    float *AVG_CORE_COUNT;
    float *CCLK_LIMIT;
    float *MAX_SOC_VOLTAGE;
    float *DVO_VOLTAGE;
    float *APML_POWER;
    float *CPU_DC_BTC;
    float *SOC_DC_BTC;
    float *DC_BTC;
    float *PACKAGE_POWER;
    float *CSTATE_BOOST;
    float *PROCHOT;
    float *PC6;
    float *SELF_REFRESH;
    float *PWM;
    float *SOCCLK;
    float *SHUBCLK;
    float *SMNCLK;
    float *SMNCLK_EFF;
    float *MP0CLK;
    float *MP0CLK_EFF;
    float *MP1CLK;
    float *MP1CLK_EFF;
    float *MP2CLK;
    float *MP2CLK_EFF;
    float *MP5CLK;
    float *TWIXCLK;
    float *WAFLCLK;
    float *DPM_BUSY;
    float *MP1_BUSY;
    float *DPM_Skipped;
    float *CORE_POWER[PMT_MAX_NUM_CORES];
    float *CORE_VOLTAGE[PMT_MAX_NUM_CORES];
    float *CORE_TEMP[PMT_MAX_NUM_CORES];
    float *CORE_FIT[PMT_MAX_NUM_CORES];
    float *CORE_IDDMAX[PMT_MAX_NUM_CORES];
    float *CORE_FREQ[PMT_MAX_NUM_CORES];
    float *CORE_FREQEFF[PMT_MAX_NUM_CORES];
    float *CORE_C0[PMT_MAX_NUM_CORES];
    float *CORE_CC1[PMT_MAX_NUM_CORES];
    float *CORE_CC6[PMT_MAX_NUM_CORES];
    float *CORE_CKS_FDD[PMT_MAX_NUM_CORES];
    float *CORE_CI_FDD[PMT_MAX_NUM_CORES];
    float *CORE_IRM[PMT_MAX_NUM_CORES];
    float *CORE_PSTATE[PMT_MAX_NUM_CORES];
    float *CORE_FREQ_LIM_MAX[PMT_MAX_NUM_CORES];
    float *CORE_FREQ_LIM_MIN[PMT_MAX_NUM_CORES];
    //float *CORE_CPPC_MAX[PMT_MAX_NUM_CORES];
    //float *CORE_CPPC_MIN[PMT_MAX_NUM_CORES];
    float *CORE_unk[PMT_MAX_NUM_CORES];
    float *CORE_SC_LIMIT[PMT_MAX_NUM_CORES];
    float *CORE_SC_CAC[PMT_MAX_NUM_CORES];
    float *CORE_SC_RESIDENCY[PMT_MAX_NUM_CORES];
    float *CORE_UOPS_CLK[PMT_MAX_NUM_CORES];
    float *CORE_UOPS[PMT_MAX_NUM_CORES];
    float *CORE_MEM_LATECY[PMT_MAX_NUM_CORES];
    float *L3_LOGIC_POWER[PMT_MAX_NUM_L3];
    float *L3_VDDM_POWER[PMT_MAX_NUM_L3];
    float *L3_TEMP[PMT_MAX_NUM_L3];
    float *L3_FIT[PMT_MAX_NUM_L3];
    float *L3_IDDMAX[PMT_MAX_NUM_L3];
    float *L3_FREQ[PMT_MAX_NUM_L3];
    float *L3_FREQ_EFF[PMT_MAX_NUM_L3];
    float *L3_CKS_FDD[PMT_MAX_NUM_L3];
    float *L3_CCA_THRESHOLD[PMT_MAX_NUM_L3];
    float *L3_CCA_CAC[PMT_MAX_NUM_L3];
    float *L3_CCA_ACTIVATION[PMT_MAX_NUM_L3];
    float *L3_EDC_LIMIT[PMT_MAX_NUM_L3];
    float *L3_EDC_CAC[PMT_MAX_NUM_L3];
    float *L3_EDC_RESIDENCY[PMT_MAX_NUM_L3];
    float *L3_FLL_BTC[PMT_MAX_NUM_L3];
  
    // MP5_BUSY seems to be always at the end of the table
    // It can be an array from 1 up to 4 values
    // What is currently assigned to MP5_BUSY seems to be called DPM_Skipped
    float *MP5_BUSY[PMT_MAX_NUM_L3];

    float *GFX_GLOB_FREQUENCY;
    float *GFX_STAPM_FREQUENCY;
    float *GFX_PPT_FREQUENCY_FAST;
    float *GFX_PPT_FREQUENCY;
    float *GFX_PPT_FREQUENCY_APU;
    float *GFX_TDC_FREQUENCY;
    float *GFX_THM_FREQUENCY;
    float *GFX_PROCHOT_FREQUENCY;
    float *GFX_VOLTAGE_FREQUENCY;
    float *GFX_CCA_FREQUENCY;
    float *GFX_DEM_FREQUENCY;
    float *GFX_VOLTAGE;
    float *GFX_TEMP;
    float *GFX_IDDMAX;
    float *GFX_FREQ;
    float *GFX_FREQEFF;
    float *GFX_BUSY;
    float *GFX_CGPG;
    float *GFX_EDC_LIM;
    float *GFX_EDC_RESIDENCY;
    float *GFX_DEM_RESIDENCY;
} pm_table;

void pm_table_0x380904(pm_table *pmt, void* base_addr); //5900X: Zen3, 16 cores, version 4
void pm_table_0x380905(pm_table *pmt, void* base_addr); //5900X: Zen3, 16 cores, version 5
void pm_table_0x380804(pm_table *pmt, void* base_addr); //5600X: Zen3,  8 cores, version 4
void pm_table_0x380805(pm_table *pmt, void* base_addr); //5600X: Zen3,  8 cores, version 5
void pm_table_0x400005(pm_table *pmt, void* base_addr); //5700G: Zen3,  8 cores, GPU, version 5
void pm_table_0x240903(pm_table *pmt, void* base_addr); //3700X: Zen2,  8 cores, version 3
void pm_table_0x240803(pm_table *pmt, void* base_addr); //3950X: Zen2, 16 cores, version 3

#endif
