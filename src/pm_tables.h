/**
 * Ryzen SMU Userspace Sensor Monitor
 * Copyright (C) 2021
 *    Florian Huehn <hattedsquirrel@gmail.com> (https://hattedsquirrel.net)
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

#ifndef pm_tables_h
#define pm_tables_h

typedef struct {
    unsigned int version;  //PM table version
    int max_cores;         //Number of cores supported by the PM table
    int zen_version;       //Zen, Zen2 or Zen3?
    unsigned int min_size; //Size of the selected PM table version

    float *PPT_LIMIT;
    float *PPT_VALUE;
    float *TDC_LIMIT;
    float *TDC_VALUE;
    float *THM_LIMIT;
    float *THM_VALUE;
    float *FIT_LIMIT;
    float *FIT_VALUE;
    float *EDC_LIMIT;
    float *EDC_VALUE;
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
    float *PPT_FREQUENCY;
    float *TDC_FREQUENCY;
    float *THM_FREQUENCY;
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
    float *LCLK_unk1[4];
    float *LCLK_unk2[4];
    float *LCLK_FREQ[4];
    float *LCLK_FREQ_EFF[4];
    float *LCLK_MAX_DPM[4];
    float *LCLK_MIN_DPM[4];
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
    float *V_unk1;
    float *PEAK_TEMP;
    float *PEAK_VOLTAGE;
    float *unk_power;
    float *AVG_CORE_COUNT;
    float *CCLK_LIMIT;
    float *MAX_VOLTAGE;
    float *DC_BTC;
    float *PACKAGE_POWER;
    float *CSTATE_BOOST;
    float *PROCHOT;
    float *PC6;
    float *PWM;
    float *SOCCLK;
    float *SHUBCLK;
    float *MP0CLK;
    float *MP1CLK;
    float *MP5CLK;
    float *SMNCLK;
    float *TWIXCLK;
    float *clk_0290;
    float *WAFLCLK;
    float *DPM_BUSY;
    float *MP1_BUSY;
    float *MP5_BUSY;
    float *CORE_POWER[16];
    float *CORE_VOLTAGE[16];
    float *CORE_TEMP[16];
    float *CORE_FIT[16];
    float *CORE_IDDMAX[16];
    float *CORE_FREQ[16];
    float *CORE_FREQEFF[16];
    float *CORE_C0[16];
    float *CORE_CC1[16];
    float *CORE_CC6[16];
    float *CORE_CKS_FDD[16];
    float *CORE_CI_FDD[16];
    float *CORE_IRM[16];
    float *CORE_PSTATE[16];
    float *CORE_FREQ_LIM_MAX[16];
    float *CORE_FREQ_LIM_MIN[16];
    float *CORE_CPPC_MAX[16];
    float *CORE_CPPC_MIN[16];
    float *CORE_unk[16];
    float *CORE_SC_LIMIT[16];
    float *CORE_SC_CAC[16];
    float *CORE_SC_RESIDENCY[16];
    float *L3_LOGIC_POWER[2];
    float *L3_VDDM_POWER[2];
    float *L3_TEMP[2];
    float *L3_FIT[2];
    float *L3_IDDMAX[2];
    float *L3_FREQ[2];
    float *L3_CKS_FDD[2];
    float *L3_CCA_THRESHOLD[2];
    float *L3_CCA_CAC[2];
    float *L3_CCA_ACTIVATION[2];
    float *L3_EDC_LIMIT[2];
    float *L3_EDC_CAC[2];
    float *L3_EDC_RESIDENCY[2];
} pm_table;

void pm_table_0x380804(pm_table *pmt, void* base_addr);

#endif