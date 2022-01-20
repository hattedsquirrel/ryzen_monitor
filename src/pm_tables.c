/**
 * Ryzen SMU Userspace Sensor Monitor
 * Copyright (C) 2021
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

/**
 * This file contains the mapping of every known PM Table version
 **/

#include "pm_tables.h"

//Calculate memory address of element
#define pm_element(i) ((float*) (base_addr + (i)*4))

//Assign 2 given elements to a array of size 2
#define assign_pm_elements_2(arr,e0,e1) arr[0]=pm_element(e0); arr[1]=pm_element(e1);

//Assign 4 given elements to a array of size 4
#define assign_pm_elements_4(arr,e0,e1,e2,e3) \
    arr[0]=pm_element(e0); arr[1]=pm_element(e1); arr[2]=pm_element(e2); arr[3]=pm_element(e3);

//Assign 4 consecutive elements to a array of size 4
#define assign_pm_elements_4_consec(arr,e) \
    arr[0]=pm_element(e  ); arr[1]=pm_element(e+1); arr[2]=pm_element(e+2); arr[3]=pm_element(e+3);

//Assign 8 consecutive elements to a array of size 8
#define assign_pm_elements_8_consec(arr,e) \
    arr[0]=pm_element(e  ); arr[1]=pm_element(e+1); arr[2]=pm_element(e+2); arr[3]=pm_element(e+3);\
    arr[4]=pm_element(e+4); arr[5]=pm_element(e+5); arr[6]=pm_element(e+6); arr[7]=pm_element(e+7);

//Assign 16 consecutive elements to a array of size 16
#define assign_pm_elements_16_consec(arr,e) \
    arr[ 0]=pm_element(e   ); arr[ 1]=pm_element(e+ 1); arr[ 2]=pm_element(e+ 2); arr[ 3]=pm_element(e+ 3);\
    arr[ 4]=pm_element(e+ 4); arr[ 5]=pm_element(e+ 5); arr[ 6]=pm_element(e+ 6); arr[ 7]=pm_element(e+ 7);\
    arr[ 8]=pm_element(e+ 8); arr[ 9]=pm_element(e+ 9); arr[10]=pm_element(e+10); arr[11]=pm_element(e+11);\
    arr[12]=pm_element(e+12); arr[13]=pm_element(e+13); arr[14]=pm_element(e+14); arr[15]=pm_element(e+15);

void pm_table_0x380804(pm_table *pmt, void* base_addr) {
    // Tested with:
    // Ryzen 5900X on Gigabyte B55M AORUS Pro-P, Bios V11p
    // Ryzen 5900X on Gigabyte B55M AORUS Pro-P, Bios V11, SMU FW v56.40.0
    // Ryzen 5900X on Gigabyte B55M AORUS Pro-P, Bios V13a, SMU FW v56.45.0, AGESA ComboV2 1.2.0.0

    pmt->version = 0x380804;
    pmt->max_cores = 16; //Number of cores supported by this PM table version
    pmt->max_l3 = 2; //Number of L3 caches supported by this PM table version
    pmt->zen_version = 3; //Zen3

    /* Legend for notes in comments:
     * o = ok. I'm confident this is the right value.
     * s = static. Does not change unter load.
     * z = always zero
     * c = changes under load. Don't know if the value is correct.
     */

    pmt->PPT_LIMIT = pm_element( 0); //o
    pmt->PPT_VALUE = pm_element( 1); //o
    pmt->TDC_LIMIT = pm_element( 2); //o
    pmt->TDC_VALUE = pm_element( 3); //o
    pmt->THM_LIMIT = pm_element( 4); //o
    pmt->THM_VALUE = pm_element( 5); //o
    pmt->FIT_LIMIT = pm_element( 6); //o
    pmt->FIT_VALUE = pm_element( 7); //o
    pmt->EDC_LIMIT = pm_element( 8); //o
    pmt->EDC_VALUE = pm_element( 9); //o
    pmt->VID_LIMIT = pm_element(10); //o
    pmt->VID_VALUE = pm_element(11); //o

    pmt->PPT_WC     = pm_element(12);
    pmt->PPT_ACTUAL = pm_element(13); //o
    pmt->TDC_WC     = pm_element(14);
    pmt->TDC_ACTUAL = pm_element(15); //o
    pmt->THM_WC     = pm_element(16);
    pmt->THM_ACTUAL = pm_element(17); //o
    pmt->FIT_WC     = pm_element(18);
    pmt->FIT_ACTUAL = pm_element(19); //o
    pmt->EDC_WC     = pm_element(20);
    pmt->EDC_ACTUAL = pm_element(21); //o
    pmt->VID_WC     = pm_element(22);
    pmt->VID_ACTUAL = pm_element(23); //o

    pmt->VDDCR_CPU_POWER = pm_element(24); //o
    pmt->VDDCR_SOC_POWER = pm_element(25); //o
    pmt->VDDIO_MEM_POWER = pm_element(26); //o
    pmt->VDD18_POWER     = pm_element(27); //o
    pmt->ROC_POWER       = pm_element(28); //s
    pmt->SOCKET_POWER    = pm_element(29); //o

    pmt->PPT_FREQUENCY     = pm_element(30);
    pmt->TDC_FREQUENCY     = pm_element(31);
    pmt->THM_FREQUENCY     = pm_element(32);
    pmt->PROCHOT_FREQUENCY = pm_element(33);
    pmt->VOLTAGE_FREQUENCY = pm_element(34);
    pmt->CCA_FREQUENCY     = pm_element(35);

    pmt->FIT_VOLTAGE            = pm_element(36);
    pmt->FIT_PRE_VOLTAGE        = pm_element(37);
    pmt->LATCHUP_VOLTAGE        = pm_element(38);
    pmt->CPU_SET_VOLTAGE        = pm_element(39); //os
    pmt->CPU_TELEMETRY_VOLTAGE  = pm_element(40);
    pmt->CPU_TELEMETRY_VOLTAGE2 = pm_element(41);
    pmt->CPU_TELEMETRY_CURRENT  = pm_element(42); //o
    pmt->CPU_TELEMETRY_POWER    = pm_element(43); //o
    pmt->SOC_SET_VOLTAGE        = pm_element(44); //os
    pmt->SOC_TELEMETRY_VOLTAGE  = pm_element(45); //o
    pmt->SOC_TELEMETRY_CURRENT  = pm_element(46); //o
    pmt->SOC_TELEMETRY_POWER    = pm_element(47); //o

    pmt->FCLK_FREQ          = pm_element(48); //o
    pmt->FCLK_FREQ_EFF      = pm_element(49); //o
    pmt->UCLK_FREQ          = pm_element(50); //o
    pmt->MEMCLK_FREQ        = pm_element(51); //o
    pmt->FCLK_DRAM_SETPOINT = pm_element(52);
    pmt->FCLK_DRAM_BUSY     = pm_element(53);
    pmt->FCLK_GMI_SETPOINT  = pm_element(54);
    pmt->FCLK_GMI_BUSY      = pm_element(55);
    pmt->FCLK_IOHC_SETPOINT = pm_element(56);
    pmt->FCLK_IOHC_BUSY     = pm_element(57);
    pmt->FCLK_XGMI_SETPOINT = pm_element(58);
    pmt->FCLK_XGMI_BUSY     = pm_element(59);

    pmt->CCM_READS     = pm_element(60);
    pmt->CCM_WRITES    = pm_element(61);
    pmt->IOMS          = pm_element(62);
    pmt->XGMI          = pm_element(63);
    pmt->CS_UMC_READS  = pm_element(64);
    pmt->CS_UMC_WRITES = pm_element(65);
    //unknown: 66 .. 69
    assign_pm_elements_4(pmt->FCLK_RESIDENCY,     70,  71,  72,  73);
    assign_pm_elements_4(pmt->FCLK_FREQ_TABLE,    74,  75,  76,  77);
    assign_pm_elements_4(pmt->UCLK_FREQ_TABLE,    78,  79,  80,  81);
    assign_pm_elements_4(pmt->MEMCLK_FREQ_TABLE,  82,  83,  84,  85);
    assign_pm_elements_4(pmt->FCLK_VOLTAGE,       86,  87,  88,  89);

    assign_pm_elements_4(pmt->LCLK_SETPOINT,      90,  98, 106, 114);
    assign_pm_elements_4(pmt->LCLK_BUSY,          91,  99, 107, 115);
    assign_pm_elements_4(pmt->LCLK_FREQ,          92, 100, 108, 116);
    assign_pm_elements_4(pmt->LCLK_FREQ_EFF,      93, 101, 109, 117);
    assign_pm_elements_4(pmt->LCLK_MAX_DPM,       94, 102, 110, 118);
    assign_pm_elements_4(pmt->LCLK_MIN_DPM,       95, 103, 111, 119);
    assign_pm_elements_4(pmt->SOCCLK_FREQ_EFF,    96, 104, 112, 120);
    assign_pm_elements_4(pmt->HUBCLK_FREQ_EFF,    97, 105, 113, 121);

    pmt->XGMI_SETPOINT   = pm_element(122);
    pmt->XGMI_BUSY       = pm_element(123);
    pmt->XGMI_LANE_WIDTH = pm_element(124);
    pmt->XGMI_DATA_RATE  = pm_element(125);

    pmt->SOC_POWER           = pm_element(126); //x
    pmt->SOC_TEMP            = pm_element(127); //o?
    pmt->DDR_VDDP_POWER      = pm_element(128);
    pmt->DDR_VDDIO_MEM_POWER = pm_element(129);
    pmt->GMI2_VDDG_POWER     = pm_element(130);
    pmt->IO_VDDCR_SOC_POWER  = pm_element(131);
    pmt->IOD_VDDIO_MEM_POWER = pm_element(132);
    pmt->IO_VDD18_POWER      = pm_element(133); 
    pmt->TDP                 = pm_element(134);
    pmt->DETERMINISM         = pm_element(135);
    pmt->V_VDDM              = pm_element(136);
    pmt->V_VDDP              = pm_element(137);
    pmt->V_VDDG_IOD          = pm_element(138);
    pmt->V_VDDG_CCD          = pm_element(139);

    pmt->PEAK_TEMP       = pm_element(140); //o
    pmt->PEAK_VOLTAGE    = pm_element(141); //o
    pmt->PEAK_CCLK_FREQ  = pm_element(142); //o
    pmt->AVG_CORE_COUNT  = pm_element(143);
    pmt->CCLK_LIMIT      = pm_element(144); //o GHz
    pmt->MAX_SOC_VOLTAGE = pm_element(145); //o
    //pmt->DC_BTC          = pm_element(146); ?? //z
    pmt->PACKAGE_POWER   = pm_element(147); //?
    //unkown 148, 149
    pmt->CSTATE_BOOST = pm_element(150);
    pmt->PROCHOT      = pm_element(151);
    pmt->PC6          = pm_element(152);
    pmt->SELF_REFRESH = pm_element(153);
    pmt->PWM          = pm_element(154);
    pmt->SOCCLK       = pm_element(155);
    pmt->SHUBCLK      = pm_element(156);
    pmt->SMNCLK       = pm_element(157);
    pmt->SMNCLK_EFF   = pm_element(158);
    pmt->MP0CLK       = pm_element(159);
    pmt->MP0CLK_EFF   = pm_element(160);
    pmt->MP1CLK       = pm_element(161);
    pmt->MP1CLK_EFF   = pm_element(162);
    pmt->MP5CLK       = pm_element(163);
    pmt->TWIXCLK      = pm_element(164);
    pmt->WAFLCLK      = pm_element(165); //0 in https://chart-studio.plotly.com/~brettdram/16/
    pmt->DPM_BUSY     = pm_element(166);
    pmt->MP1_BUSY     = pm_element(167);
    pmt->MP5_BUSY     = pm_element(168);

    assign_pm_elements_16_consec(pmt->CORE_POWER       , 169);
    assign_pm_elements_16_consec(pmt->CORE_VOLTAGE     , 185);
    assign_pm_elements_16_consec(pmt->CORE_TEMP        , 201);
    assign_pm_elements_16_consec(pmt->CORE_FIT         , 217);
    assign_pm_elements_16_consec(pmt->CORE_IDDMAX      , 233);
    assign_pm_elements_16_consec(pmt->CORE_FREQ        , 249);
    assign_pm_elements_16_consec(pmt->CORE_FREQEFF     , 265);
    assign_pm_elements_16_consec(pmt->CORE_C0          , 281);
    assign_pm_elements_16_consec(pmt->CORE_CC1         , 297);
    assign_pm_elements_16_consec(pmt->CORE_CC6         , 313);
    assign_pm_elements_16_consec(pmt->CORE_CKS_FDD     , 329);
    assign_pm_elements_16_consec(pmt->CORE_CI_FDD      , 345);
    assign_pm_elements_16_consec(pmt->CORE_IRM         , 361);
    assign_pm_elements_16_consec(pmt->CORE_PSTATE      , 377);
    assign_pm_elements_16_consec(pmt->CORE_FREQ_LIM_MAX, 393);
    assign_pm_elements_16_consec(pmt->CORE_FREQ_LIM_MIN, 409);
    assign_pm_elements_16_consec(pmt->CORE_SC_LIMIT    , 425);
    assign_pm_elements_16_consec(pmt->CORE_SC_CAC      , 441);
    assign_pm_elements_16_consec(pmt->CORE_SC_RESIDENCY, 457);
    assign_pm_elements_16_consec(pmt->CORE_UOPS_CLK    , 473);
    assign_pm_elements_16_consec(pmt->CORE_UOPS        , 489);
    assign_pm_elements_16_consec(pmt->CORE_MEM_LATECY  , 505);

    assign_pm_elements_2(pmt->L3_LOGIC_POWER   , 521, 522);
    assign_pm_elements_2(pmt->L3_VDDM_POWER    , 523, 524);
    assign_pm_elements_2(pmt->L3_TEMP          , 525, 526);
    assign_pm_elements_2(pmt->L3_FIT           , 527, 528);
    assign_pm_elements_2(pmt->L3_IDDMAX        , 529, 530);
    assign_pm_elements_2(pmt->L3_FREQ          , 531, 532);
    assign_pm_elements_2(pmt->L3_FREQ_EFF      , 533, 534);
    assign_pm_elements_2(pmt->L3_CKS_FDD       , 535, 536);
    assign_pm_elements_2(pmt->L3_CCA_THRESHOLD , 537, 538);
    assign_pm_elements_2(pmt->L3_CCA_CAC       , 539, 540);
    assign_pm_elements_2(pmt->L3_CCA_ACTIVATION, 541, 542);
    assign_pm_elements_2(pmt->L3_EDC_LIMIT     , 543, 544);
    assign_pm_elements_2(pmt->L3_EDC_CAC       , 545, 546);
    assign_pm_elements_2(pmt->L3_EDC_RESIDENCY , 547, 548);
    assign_pm_elements_2(pmt->L3_FLL_BTC       , 549, 550);
    //2 unkown values to follow. Might have to do with L3 because there are 
    //only 1 value left for units with one L3 cache.
    //[2] //c 21->31

    pmt->min_size = 551*4; //(Highest element we access + 1)*4.
                           //Needed to avoid illegal memory access
}

void pm_table_0x380805(pm_table *pmt, void* base_addr) {
    // Tested with:
    // Ryzen 5900X on Gigabyte B55M AORUS Pro-P, Bios V13i, SMU FW v56.50.0, AGESA ComboV2 1.2.0.2

    pmt->version = 0x380805;
    pmt->max_cores = 16; //Number of cores supported by this PM table version
    pmt->max_l3 = 2; //Number of L3 caches supported by this PM table version
    pmt->zen_version = 3; //Zen3

    /* Legend for notes in comments:
     * o = ok. I'm confident this is the right value.
     * s = static. Does not change unter load.
     * z = always zero
     * c = changes under load. Don't know if the value is correct.
     */

    pmt->PPT_LIMIT = pm_element( 0); //o
    pmt->PPT_VALUE = pm_element( 1); //o
    pmt->TDC_LIMIT = pm_element( 2); //o
    pmt->TDC_VALUE = pm_element( 3); //o
    pmt->THM_LIMIT = pm_element( 4); //o
    pmt->THM_VALUE = pm_element( 5); //o
    pmt->FIT_LIMIT = pm_element( 6); //o
    pmt->FIT_VALUE = pm_element( 7); //o
    pmt->EDC_LIMIT = pm_element( 8); //o
    pmt->EDC_VALUE = pm_element( 9); //o
    pmt->VID_LIMIT = pm_element(10); //o
    pmt->VID_VALUE = pm_element(11); //o

    pmt->PPT_WC     = pm_element(12);
    pmt->PPT_ACTUAL = pm_element(13); //o
    pmt->TDC_WC     = pm_element(14);
    pmt->TDC_ACTUAL = pm_element(15); //o
    pmt->THM_WC     = pm_element(16);
    pmt->THM_ACTUAL = pm_element(17); //o
    pmt->FIT_WC     = pm_element(18);
    pmt->FIT_ACTUAL = pm_element(19); //o
    pmt->EDC_WC     = pm_element(20);
    pmt->EDC_ACTUAL = pm_element(21); //o
    pmt->VID_WC     = pm_element(22);
    pmt->VID_ACTUAL = pm_element(23); //o

    pmt->VDDCR_CPU_POWER = pm_element(24); //o
    pmt->VDDCR_SOC_POWER = pm_element(25); //o
    pmt->VDDIO_MEM_POWER = pm_element(26); //o
    pmt->VDD18_POWER     = pm_element(27); //o
    pmt->ROC_POWER       = pm_element(28); //s
    pmt->SOCKET_POWER    = pm_element(29); //o

    pmt->PPT_FREQUENCY     = pm_element(30);
    pmt->TDC_FREQUENCY     = pm_element(31);
    pmt->THM_FREQUENCY     = pm_element(32);
    pmt->PROCHOT_FREQUENCY = pm_element(33);
    pmt->VOLTAGE_FREQUENCY = pm_element(34);
    pmt->CCA_FREQUENCY     = pm_element(35);

    pmt->FIT_VOLTAGE            = pm_element(36);
    pmt->FIT_PRE_VOLTAGE        = pm_element(37);
    pmt->LATCHUP_VOLTAGE        = pm_element(38);
    pmt->CPU_SET_VOLTAGE        = pm_element(39); //os
    pmt->CPU_TELEMETRY_VOLTAGE  = pm_element(40);
    pmt->CPU_TELEMETRY_VOLTAGE2 = pm_element(41);
    pmt->CPU_TELEMETRY_CURRENT  = pm_element(42); //o
    pmt->CPU_TELEMETRY_POWER    = pm_element(43); //o
    pmt->SOC_SET_VOLTAGE        = pm_element(44); //os
    pmt->SOC_TELEMETRY_VOLTAGE  = pm_element(45); //o
    pmt->SOC_TELEMETRY_CURRENT  = pm_element(46); //o
    pmt->SOC_TELEMETRY_POWER    = pm_element(47); //o

    pmt->FCLK_FREQ          = pm_element(48); //o
    pmt->FCLK_FREQ_EFF      = pm_element(49); //o
    pmt->UCLK_FREQ          = pm_element(50); //o
    pmt->MEMCLK_FREQ        = pm_element(51); //o
    pmt->FCLK_DRAM_SETPOINT = pm_element(52);
    pmt->FCLK_DRAM_BUSY     = pm_element(53);
    pmt->FCLK_GMI_SETPOINT  = pm_element(54);
    pmt->FCLK_GMI_BUSY      = pm_element(55);
    pmt->FCLK_IOHC_SETPOINT = pm_element(56);
    pmt->FCLK_IOHC_BUSY     = pm_element(57);
    pmt->FCLK_XGMI_SETPOINT = pm_element(58);
    pmt->FCLK_XGMI_BUSY     = pm_element(59);

    pmt->CCM_READS     = pm_element(60);
    pmt->CCM_WRITES    = pm_element(61);
    pmt->IOMS          = pm_element(62);
    pmt->XGMI          = pm_element(63);
    pmt->CS_UMC_READS  = pm_element(64);
    pmt->CS_UMC_WRITES = pm_element(65);
    //unknown: 66 .. 69
    assign_pm_elements_4(pmt->FCLK_RESIDENCY,     70,  71,  72,  73);
    assign_pm_elements_4(pmt->FCLK_FREQ_TABLE,    74,  75,  76,  77);
    assign_pm_elements_4(pmt->UCLK_FREQ_TABLE,    78,  79,  80,  81);
    assign_pm_elements_4(pmt->MEMCLK_FREQ_TABLE,  82,  83,  84,  85);
    assign_pm_elements_4(pmt->FCLK_VOLTAGE,       86,  87,  88,  89);

    assign_pm_elements_4(pmt->LCLK_SETPOINT,      90,  98, 106, 114);
    assign_pm_elements_4(pmt->LCLK_BUSY,          91,  99, 107, 115);
    assign_pm_elements_4(pmt->LCLK_FREQ,          92, 100, 108, 116);
    assign_pm_elements_4(pmt->LCLK_FREQ_EFF,      93, 101, 109, 117);
    assign_pm_elements_4(pmt->LCLK_MAX_DPM,       94, 102, 110, 118);
    assign_pm_elements_4(pmt->LCLK_MIN_DPM,       95, 103, 111, 119);
    assign_pm_elements_4(pmt->SOCCLK_FREQ_EFF,    96, 104, 112, 120);
    assign_pm_elements_4(pmt->HUBCLK_FREQ_EFF,    97, 105, 113, 121);

    pmt->XGMI_SETPOINT   = pm_element(122);
    pmt->XGMI_BUSY       = pm_element(123);
    pmt->XGMI_LANE_WIDTH = pm_element(124);
    pmt->XGMI_DATA_RATE  = pm_element(125);

    pmt->SOC_POWER           = pm_element(126); //x
    pmt->SOC_TEMP            = pm_element(127); //o?
    pmt->DDR_VDDP_POWER      = pm_element(128);
    pmt->DDR_VDDIO_MEM_POWER = pm_element(129);
    pmt->GMI2_VDDG_POWER     = pm_element(130);
    pmt->IO_VDDCR_SOC_POWER  = pm_element(131);
    pmt->IOD_VDDIO_MEM_POWER = pm_element(132);
    pmt->IO_VDD18_POWER      = pm_element(133); 
    pmt->TDP                 = pm_element(134);
    pmt->DETERMINISM         = pm_element(135);
    pmt->V_VDDM              = pm_element(136);
    pmt->V_VDDP              = pm_element(137);
    pmt->V_VDDG_IOD          = pm_element(138);
    pmt->V_VDDG_CCD          = pm_element(139);

    pmt->PEAK_TEMP       = pm_element(140); //??
    pmt->PEAK_VOLTAGE    = pm_element(141); //o
    pmt->PEAK_CCLK_FREQ  = pm_element(142); //o
    //143 peak (requested?) voltage?
    //144 temp? power? 34->58
    //145 temp? power? 49->65
    pmt->AVG_CORE_COUNT  = pm_element(146); //o
    pmt->CCLK_LIMIT      = pm_element(147); //o
    pmt->MAX_SOC_VOLTAGE = pm_element(148); //o
    //149 -> //z
    pmt->PACKAGE_POWER   = pm_element(150); //? some power at least. 22->86 in 64W eco mode. PPT maybe?
    //unknown 151, 152
    pmt->CSTATE_BOOST = pm_element(153);
    pmt->PROCHOT      = pm_element(154);
    pmt->PC6          = pm_element(155); //o
    pmt->SELF_REFRESH = pm_element(156); //?
    pmt->PWM          = pm_element(157); //?
    pmt->SOCCLK       = pm_element(158);
    pmt->SHUBCLK      = pm_element(159);
    pmt->SMNCLK       = pm_element(160);
    pmt->SMNCLK_EFF   = pm_element(161);
    pmt->MP0CLK       = pm_element(162);
    pmt->MP0CLK_EFF   = pm_element(163);
    pmt->MP1CLK       = pm_element(164);
    pmt->MP1CLK_EFF   = pm_element(165);
    pmt->MP5CLK       = pm_element(166);
    pmt->TWIXCLK      = pm_element(167);
    pmt->WAFLCLK      = pm_element(168); //0 in https://chart-studio.plotly.com/~brettdram/16/
    pmt->DPM_BUSY     = pm_element(169);
    pmt->MP1_BUSY     = pm_element(170);
    pmt->MP5_BUSY     = pm_element(171);

    assign_pm_elements_16_consec(pmt->CORE_POWER       , 172); //o
    assign_pm_elements_16_consec(pmt->CORE_VOLTAGE     , 188); //o
    assign_pm_elements_16_consec(pmt->CORE_TEMP        , 204); //o 37->65
    assign_pm_elements_16_consec(pmt->CORE_FIT         , 220); //o
    assign_pm_elements_16_consec(pmt->CORE_IDDMAX      , 236); //o
    assign_pm_elements_16_consec(pmt->CORE_FREQ        , 252); //o
    assign_pm_elements_16_consec(pmt->CORE_FREQEFF     , 268); //o
    assign_pm_elements_16_consec(pmt->CORE_C0          , 284); //o
    assign_pm_elements_16_consec(pmt->CORE_CC1         , 300); //o
    assign_pm_elements_16_consec(pmt->CORE_CC6         , 316); //o
    assign_pm_elements_16_consec(pmt->CORE_CKS_FDD     , 332); //z
    assign_pm_elements_16_consec(pmt->CORE_CI_FDD      , 348); //z
    assign_pm_elements_16_consec(pmt->CORE_IRM         , 364); //o
    assign_pm_elements_16_consec(pmt->CORE_PSTATE      , 380); //o
    assign_pm_elements_16_consec(pmt->CORE_FREQ_LIM_MAX, 396); //o
    assign_pm_elements_16_consec(pmt->CORE_FREQ_LIM_MIN, 412); //o
    assign_pm_elements_16_consec(pmt->CORE_unk         , 428); //z
    assign_pm_elements_16_consec(pmt->CORE_SC_LIMIT    , 444); //55->21
    assign_pm_elements_16_consec(pmt->CORE_SC_CAC      , 460); //0->13
    assign_pm_elements_16_consec(pmt->CORE_SC_RESIDENCY, 476); //z
    assign_pm_elements_16_consec(pmt->CORE_UOPS_CLK    , 492); //0->6
    assign_pm_elements_16_consec(pmt->CORE_UOPS        , 508); //200->12000
    assign_pm_elements_16_consec(pmt->CORE_MEM_LATECY  , 524); //15->350

    assign_pm_elements_2(pmt->L3_LOGIC_POWER   , 540, 541);
    assign_pm_elements_2(pmt->L3_VDDM_POWER    , 542, 543);
    assign_pm_elements_2(pmt->L3_TEMP          , 544, 545);
    assign_pm_elements_2(pmt->L3_FIT           , 546, 547);
    assign_pm_elements_2(pmt->L3_IDDMAX        , 548, 549);
    assign_pm_elements_2(pmt->L3_FREQ          , 550, 551);
    assign_pm_elements_2(pmt->L3_FREQ_EFF      , 552, 553);
    assign_pm_elements_2(pmt->L3_CKS_FDD       , 554, 555);
    assign_pm_elements_2(pmt->L3_CCA_THRESHOLD , 556, 557);
    assign_pm_elements_2(pmt->L3_CCA_CAC       , 558, 559);
    assign_pm_elements_2(pmt->L3_CCA_ACTIVATION, 560, 561);
    assign_pm_elements_2(pmt->L3_EDC_LIMIT     , 562, 563);
    assign_pm_elements_2(pmt->L3_EDC_CAC       , 564, 565);
    assign_pm_elements_2(pmt->L3_EDC_RESIDENCY , 566, 567);
    assign_pm_elements_2(pmt->L3_FLL_BTC       , 568, 569);
    //2 unkown values to follow. Might have to do with L3 because there are 
    //only 1 value left for units with one L3 cache.
    //[2] //c 19->27

    pmt->min_size = 570*4; //(Highest element we access + 1)*4.
                           //Needed to avoid illegal memory access
}

void pm_table_0x380904(pm_table *pmt, void* base_addr) {
    // most guesswork done by spektren 
    // https://github.com/hattedsquirrel/ryzen_monitor/issues/1
    // loosely tested with:
    // Ryzen 5600X on ASUS B550-I, bios 1803, SMU FW v56.45.0, AGESA 1.2.0.0


    pmt->version = 0x380904;
    pmt->max_cores = 8; //Number of cores supported by this PM table version
    pmt->max_l3 = 1; //Number of L3 caches supported by this PM table version
    pmt->zen_version = 3; //Zen3


	pmt->PPT_LIMIT =                    pm_element(0);
	pmt->PPT_VALUE =                    pm_element(1);
	pmt->TDC_LIMIT =                    pm_element(2);
	pmt->TDC_VALUE =                    pm_element(3);
	pmt->THM_LIMIT =                    pm_element(4);
	pmt->THM_VALUE =                    pm_element(5);
	pmt->FIT_LIMIT =                    pm_element(6);
	pmt->FIT_VALUE =                    pm_element(7);
	pmt->EDC_LIMIT =                    pm_element(8);
	pmt->EDC_VALUE =                    pm_element(9);
	pmt->VID_LIMIT =                    pm_element(10); 
	pmt->VID_VALUE =                    pm_element(11); 

	pmt->PPT_WC =                       pm_element(12);
	pmt->PPT_ACTUAL =                   pm_element(13); 
	pmt->TDC_WC =                       pm_element(14);
	pmt->TDC_ACTUAL =                   pm_element(15); 
	pmt->THM_WC =                       pm_element(16);
	pmt->THM_ACTUAL =                   pm_element(17); 
	pmt->FIT_WC =                       pm_element(18);
	pmt->FIT_ACTUAL =                   pm_element(19); 
	pmt->EDC_WC =                       pm_element(20);
	pmt->EDC_ACTUAL =                   pm_element(21); 
	pmt->VID_WC =                       pm_element(22);
	pmt->VID_ACTUAL =                   pm_element(23); 
	
    pmt->VDDCR_CPU_POWER =              pm_element(24); 
	pmt->VDDCR_SOC_POWER =              pm_element(25); 
	pmt->VDDIO_MEM_POWER =              pm_element(26); 
	pmt->VDD18_POWER =                  pm_element(27); 
	pmt->ROC_POWER =                    pm_element(28); 
	pmt->SOCKET_POWER =                 pm_element(29); 
	
    pmt->PPT_FREQUENCY =                pm_element(30);
	pmt->TDC_FREQUENCY =                pm_element(31);
	pmt->THM_FREQUENCY =                pm_element(32);
	pmt->PROCHOT_FREQUENCY =            pm_element(33);
	pmt->VOLTAGE_FREQUENCY =            pm_element(34);
	pmt->CCA_FREQUENCY =                pm_element(35);
	
    pmt->FIT_VOLTAGE =                  pm_element(36);
	pmt->FIT_PRE_VOLTAGE =              pm_element(37);
	pmt->LATCHUP_VOLTAGE =              pm_element(38);
	pmt->CPU_SET_VOLTAGE =              pm_element(39); 
	pmt->CPU_TELEMETRY_VOLTAGE =        pm_element(40);
	pmt->CPU_TELEMETRY_VOLTAGE2 =       pm_element(41);
	pmt->CPU_TELEMETRY_CURRENT =        pm_element(42); 
	pmt->CPU_TELEMETRY_POWER =          pm_element(43); 
	pmt->SOC_SET_VOLTAGE =              pm_element(44); 
	pmt->SOC_TELEMETRY_VOLTAGE =        pm_element(45); 
	pmt->SOC_TELEMETRY_CURRENT =        pm_element(46); 
	pmt->SOC_TELEMETRY_POWER =          pm_element(47); 
	
    pmt->FCLK_FREQ =                    pm_element(48); 
	pmt->FCLK_FREQ_EFF =                pm_element(49); 
	pmt->UCLK_FREQ =                    pm_element(50); 
	pmt->MEMCLK_FREQ =                  pm_element(51); 
	pmt->FCLK_DRAM_SETPOINT =           pm_element(52);
	pmt->FCLK_DRAM_BUSY =               pm_element(53);
	pmt->FCLK_GMI_SETPOINT =            pm_element(54);
	pmt->FCLK_GMI_BUSY =                pm_element(55);
	pmt->FCLK_IOHC_SETPOINT =           pm_element(56);
	pmt->FCLK_IOHC_BUSY =               pm_element(57);
	pmt->FCLK_XGMI_SETPOINT =           pm_element(58);
	pmt->FCLK_XGMI_BUSY =               pm_element(59);
	
    pmt->CCM_READS =                    pm_element(60);
	pmt->CCM_WRITES =                   pm_element(61);
	pmt->IOMS =                         pm_element(62);
	pmt->XGMI =                         pm_element(63);
	pmt->CS_UMC_READS =                 pm_element(64);
	pmt->CS_UMC_WRITES =                pm_element(65);
	//unknown: 66 .. 69
	//unknown: 66 .. 69
	//unknown: 66 .. 69
	//unknown: 66 .. 69
    assign_pm_elements_4(pmt->FCLK_RESIDENCY,     70,  71,  72,  73);
    assign_pm_elements_4(pmt->FCLK_FREQ_TABLE,    74,  75,  76,  77);
    assign_pm_elements_4(pmt->UCLK_FREQ_TABLE,    78,  79,  80,  81);
    assign_pm_elements_4(pmt->MEMCLK_FREQ_TABLE,  82,  83,  84,  85);
    assign_pm_elements_4(pmt->FCLK_VOLTAGE,       86,  87,  88,  89);

    assign_pm_elements_4(pmt->LCLK_SETPOINT,      90,  98, 106, 114);
    assign_pm_elements_4(pmt->LCLK_BUSY,          91,  99, 107, 115);
    assign_pm_elements_4(pmt->LCLK_FREQ,          92, 100, 108, 116);
    assign_pm_elements_4(pmt->LCLK_FREQ_EFF,      93, 101, 109, 117);
    assign_pm_elements_4(pmt->LCLK_MAX_DPM,       94, 102, 110, 118);
    assign_pm_elements_4(pmt->LCLK_MIN_DPM,       95, 103, 111, 119);
    assign_pm_elements_4(pmt->SOCCLK_FREQ_EFF,    96, 104, 112, 120);
    assign_pm_elements_4(pmt->HUBCLK_FREQ_EFF,    97, 105, 113, 121); 

	pmt->XGMI_SETPOINT =                pm_element(122);
	pmt->XGMI_BUSY =                    pm_element(123);
	pmt->XGMI_LANE_WIDTH =              pm_element(124);
	pmt->XGMI_DATA_RATE =               pm_element(125);

	pmt->SOC_POWER =                    pm_element(126); 
	pmt->SOC_TEMP =                     pm_element(127); 
	pmt->DDR_VDDP_POWER =               pm_element(128);
	pmt->DDR_VDDIO_MEM_POWER =          pm_element(129);
	pmt->GMI2_VDDG_POWER =              pm_element(130);
	pmt->IO_VDDCR_SOC_POWER =           pm_element(131);
	pmt->IOD_VDDIO_MEM_POWER =          pm_element(132);
	pmt->IO_VDD18_POWER =               pm_element(133);
	pmt->TDP =                          pm_element(134);
	pmt->DETERMINISM =                  pm_element(135);
	pmt->V_VDDM =                       pm_element(136);
	pmt->V_VDDP =                       pm_element(137);
	pmt->V_VDDG_IOD =                   pm_element(138);
	pmt->V_VDDG_CCD =                   pm_element(139);
	
    pmt->PEAK_TEMP =                    pm_element(140);
    pmt->PEAK_VOLTAGE =                 pm_element(141);
    pmt->PEAK_CCLK_FREQ =               pm_element(142);
    pmt->AVG_CORE_COUNT =               pm_element(143);
    pmt->CCLK_LIMIT =                   pm_element(144);
    pmt->MAX_SOC_VOLTAGE =              pm_element(145);
    //pmt->DC_BTC =                     pm_element(146); ??
    pmt->PACKAGE_POWER =                pm_element(147);
	//unkown 148, 149
	//unkown 148, 149
    pmt->CSTATE_BOOST = pm_element(150);
    pmt->PROCHOT      = pm_element(151);
    pmt->PC6          = pm_element(152);
    pmt->SELF_REFRESH = pm_element(153);
    pmt->PWM          = pm_element(154);
    pmt->SOCCLK       = pm_element(155);
    pmt->SHUBCLK      = pm_element(156);
    pmt->SMNCLK       = pm_element(157);
    pmt->SMNCLK_EFF   = pm_element(158);
    pmt->MP0CLK       = pm_element(159);
    pmt->MP0CLK_EFF   = pm_element(160);
    pmt->MP1CLK       = pm_element(161);
    pmt->MP1CLK_EFF   = pm_element(162);
    pmt->MP5CLK       = pm_element(163);
    pmt->TWIXCLK      = pm_element(164);
    pmt->WAFLCLK      = pm_element(165);
    pmt->DPM_BUSY     = pm_element(166);
    pmt->MP1_BUSY     = pm_element(167);
    pmt->MP5_BUSY     = pm_element(168);
	
    assign_pm_elements_8_consec(pmt->CORE_POWER         , 169);
	assign_pm_elements_8_consec(pmt->CORE_VOLTAGE       , 177);
	assign_pm_elements_8_consec(pmt->CORE_TEMP          , 185);
	assign_pm_elements_8_consec(pmt->CORE_FIT           , 193);
	assign_pm_elements_8_consec(pmt->CORE_IDDMAX        , 201);
	assign_pm_elements_8_consec(pmt->CORE_FREQ          , 209);
	assign_pm_elements_8_consec(pmt->CORE_FREQEFF       , 217);
	assign_pm_elements_8_consec(pmt->CORE_C0            , 225);
	assign_pm_elements_8_consec(pmt->CORE_CC1           , 233);
	assign_pm_elements_8_consec(pmt->CORE_CC6           , 241);
	assign_pm_elements_8_consec(pmt->CORE_CKS_FDD       , 249);
	assign_pm_elements_8_consec(pmt->CORE_CI_FDD        , 257);
	assign_pm_elements_8_consec(pmt->CORE_IRM           , 265);
	assign_pm_elements_8_consec(pmt->CORE_PSTATE        , 273);
	assign_pm_elements_8_consec(pmt->CORE_FREQ_LIM_MAX  , 281);
	assign_pm_elements_8_consec(pmt->CORE_FREQ_LIM_MIN  , 289);
	assign_pm_elements_8_consec(pmt->CORE_SC_LIMIT      , 297);
	assign_pm_elements_8_consec(pmt->CORE_SC_CAC        , 305);
	assign_pm_elements_8_consec(pmt->CORE_SC_RESIDENCY  , 313);
	assign_pm_elements_8_consec(pmt->CORE_UOPS_CLK      , 321);
	assign_pm_elements_8_consec(pmt->CORE_UOPS          , 329);
	assign_pm_elements_8_consec(pmt->CORE_MEM_LATECY    , 337);
	
    pmt->L3_LOGIC_POWER[0] =                pm_element(345);
	pmt->L3_VDDM_POWER[0] =                 pm_element(346);
	pmt->L3_TEMP[0] =                       pm_element(347);
	pmt->L3_FIT[0] =                        pm_element(348);
	pmt->L3_IDDMAX[0] =                     pm_element(349);
	pmt->L3_FREQ[0] =                       pm_element(350);
	pmt->L3_FREQ_EFF[0] =                   pm_element(351);
	pmt->L3_CKS_FDD[0] =                    pm_element(352);
	pmt->L3_CCA_THRESHOLD[0] =              pm_element(353);
	pmt->L3_CCA_CAC[0] =                    pm_element(354);
	pmt->L3_CCA_ACTIVATION[0] =             pm_element(355);
	pmt->L3_EDC_LIMIT[0] =                  pm_element(356);
	pmt->L3_EDC_CAC[0] =                    pm_element(357);
	pmt->L3_EDC_RESIDENCY[0] =              pm_element(358);
    pmt->L3_FLL_BTC[0] =                    pm_element(359);
	//unkown 360
	
	pmt->min_size = 361*4; //(Highest element we access + 1)*4.
	//Needed to avoid illegal memory access
}

void pm_table_0x380905(pm_table *pmt, void* base_addr) {
    // Pure guess. Derived from 0x380805 and 0x380904
    // Ryzen 5600X

    pmt->version = 0x380905;
    pmt->max_cores = 8; //Number of cores supported by this PM table version
    pmt->max_l3 = 1; //Number of L3 caches supported by this PM table version
    pmt->zen_version = 3; //Zen3


	pmt->PPT_LIMIT =                    pm_element(0);
	pmt->PPT_VALUE =                    pm_element(1);
	pmt->TDC_LIMIT =                    pm_element(2);
	pmt->TDC_VALUE =                    pm_element(3);
	pmt->THM_LIMIT =                    pm_element(4);
	pmt->THM_VALUE =                    pm_element(5);
	pmt->FIT_LIMIT =                    pm_element(6);
	pmt->FIT_VALUE =                    pm_element(7);
	pmt->EDC_LIMIT =                    pm_element(8);
	pmt->EDC_VALUE =                    pm_element(9);
	pmt->VID_LIMIT =                    pm_element(10); 
	pmt->VID_VALUE =                    pm_element(11); 

	pmt->PPT_WC =                       pm_element(12);
	pmt->PPT_ACTUAL =                   pm_element(13); 
	pmt->TDC_WC =                       pm_element(14);
	pmt->TDC_ACTUAL =                   pm_element(15); 
	pmt->THM_WC =                       pm_element(16);
	pmt->THM_ACTUAL =                   pm_element(17); 
	pmt->FIT_WC =                       pm_element(18);
	pmt->FIT_ACTUAL =                   pm_element(19); 
	pmt->EDC_WC =                       pm_element(20);
	pmt->EDC_ACTUAL =                   pm_element(21); 
	pmt->VID_WC =                       pm_element(22);
	pmt->VID_ACTUAL =                   pm_element(23); 
	
    pmt->VDDCR_CPU_POWER =              pm_element(24); 
	pmt->VDDCR_SOC_POWER =              pm_element(25); 
	pmt->VDDIO_MEM_POWER =              pm_element(26); 
	pmt->VDD18_POWER =                  pm_element(27); 
	pmt->ROC_POWER =                    pm_element(28); 
	pmt->SOCKET_POWER =                 pm_element(29); 
	
    pmt->PPT_FREQUENCY =                pm_element(30);
	pmt->TDC_FREQUENCY =                pm_element(31);
	pmt->THM_FREQUENCY =                pm_element(32);
	pmt->PROCHOT_FREQUENCY =            pm_element(33);
	pmt->VOLTAGE_FREQUENCY =            pm_element(34);
	pmt->CCA_FREQUENCY =                pm_element(35);
	
    pmt->FIT_VOLTAGE =                  pm_element(36);
	pmt->FIT_PRE_VOLTAGE =              pm_element(37);
	pmt->LATCHUP_VOLTAGE =              pm_element(38);
	pmt->CPU_SET_VOLTAGE =              pm_element(39); 
	pmt->CPU_TELEMETRY_VOLTAGE =        pm_element(40);
	pmt->CPU_TELEMETRY_VOLTAGE2 =       pm_element(41);
	pmt->CPU_TELEMETRY_CURRENT =        pm_element(42); 
	pmt->CPU_TELEMETRY_POWER =          pm_element(43); 
	pmt->SOC_SET_VOLTAGE =              pm_element(44); 
	pmt->SOC_TELEMETRY_VOLTAGE =        pm_element(45); 
	pmt->SOC_TELEMETRY_CURRENT =        pm_element(46); 
	pmt->SOC_TELEMETRY_POWER =          pm_element(47); 
	
    pmt->FCLK_FREQ =                    pm_element(48); 
	pmt->FCLK_FREQ_EFF =                pm_element(49); 
	pmt->UCLK_FREQ =                    pm_element(50); 
	pmt->MEMCLK_FREQ =                  pm_element(51); 
	pmt->FCLK_DRAM_SETPOINT =           pm_element(52);
	pmt->FCLK_DRAM_BUSY =               pm_element(53);
	pmt->FCLK_GMI_SETPOINT =            pm_element(54);
	pmt->FCLK_GMI_BUSY =                pm_element(55);
	pmt->FCLK_IOHC_SETPOINT =           pm_element(56);
	pmt->FCLK_IOHC_BUSY =               pm_element(57);
	pmt->FCLK_XGMI_SETPOINT =           pm_element(58);
	pmt->FCLK_XGMI_BUSY =               pm_element(59);
	
    pmt->CCM_READS =                    pm_element(60);
	pmt->CCM_WRITES =                   pm_element(61);
	pmt->IOMS =                         pm_element(62);
	pmt->XGMI =                         pm_element(63);
	pmt->CS_UMC_READS =                 pm_element(64);
	pmt->CS_UMC_WRITES =                pm_element(65);
	//unknown: 66 .. 69
	//unknown: 66 .. 69
	//unknown: 66 .. 69
	//unknown: 66 .. 69
    assign_pm_elements_4(pmt->FCLK_RESIDENCY,     70,  71,  72,  73);
    assign_pm_elements_4(pmt->FCLK_FREQ_TABLE,    74,  75,  76,  77);
    assign_pm_elements_4(pmt->UCLK_FREQ_TABLE,    78,  79,  80,  81);
    assign_pm_elements_4(pmt->MEMCLK_FREQ_TABLE,  82,  83,  84,  85);
    assign_pm_elements_4(pmt->FCLK_VOLTAGE,       86,  87,  88,  89);

    assign_pm_elements_4(pmt->LCLK_SETPOINT,      90,  98, 106, 114);
    assign_pm_elements_4(pmt->LCLK_BUSY,          91,  99, 107, 115);
    assign_pm_elements_4(pmt->LCLK_FREQ,          92, 100, 108, 116);
    assign_pm_elements_4(pmt->LCLK_FREQ_EFF,      93, 101, 109, 117);
    assign_pm_elements_4(pmt->LCLK_MAX_DPM,       94, 102, 110, 118);
    assign_pm_elements_4(pmt->LCLK_MIN_DPM,       95, 103, 111, 119);
    assign_pm_elements_4(pmt->SOCCLK_FREQ_EFF,    96, 104, 112, 120);
    assign_pm_elements_4(pmt->HUBCLK_FREQ_EFF,    97, 105, 113, 121); 

	pmt->XGMI_SETPOINT =                pm_element(122);
	pmt->XGMI_BUSY =                    pm_element(123);
	pmt->XGMI_LANE_WIDTH =              pm_element(124);
	pmt->XGMI_DATA_RATE =               pm_element(125);

	pmt->SOC_POWER =                    pm_element(126); 
	pmt->SOC_TEMP =                     pm_element(127); 
	pmt->DDR_VDDP_POWER =               pm_element(128);
	pmt->DDR_VDDIO_MEM_POWER =          pm_element(129);
	pmt->GMI2_VDDG_POWER =              pm_element(130);
	pmt->IO_VDDCR_SOC_POWER =           pm_element(131);
	pmt->IOD_VDDIO_MEM_POWER =          pm_element(132);
	pmt->IO_VDD18_POWER =               pm_element(133);
	pmt->TDP =                          pm_element(134);
	pmt->DETERMINISM =                  pm_element(135);
	pmt->V_VDDM =                       pm_element(136);
	pmt->V_VDDP =                       pm_element(137);
	pmt->V_VDDG_IOD =                   pm_element(138);
	pmt->V_VDDG_CCD =                   pm_element(139);
	
    pmt->PEAK_TEMP       = pm_element(140);
    pmt->PEAK_VOLTAGE    = pm_element(141);
    pmt->PEAK_CCLK_FREQ  = pm_element(142);
    //143 peak (requested?) voltage?
    //144 temp? power?
    //145 temp? power?
    pmt->AVG_CORE_COUNT  = pm_element(146);
    pmt->CCLK_LIMIT      = pm_element(147);
    pmt->MAX_SOC_VOLTAGE = pm_element(148);
    //149 -> //z
    pmt->PACKAGE_POWER   = pm_element(150);
    //unknown 151, 152
    pmt->CSTATE_BOOST = pm_element(153);
    pmt->PROCHOT      = pm_element(154);
    pmt->PC6          = pm_element(155);
    pmt->SELF_REFRESH = pm_element(156);
    pmt->PWM          = pm_element(157);
    pmt->SOCCLK       = pm_element(158);
    pmt->SHUBCLK      = pm_element(159);
    pmt->SMNCLK       = pm_element(160);
    pmt->SMNCLK_EFF   = pm_element(161);
    pmt->MP0CLK       = pm_element(162);
    pmt->MP0CLK_EFF   = pm_element(163);
    pmt->MP1CLK       = pm_element(164);
    pmt->MP1CLK_EFF   = pm_element(165);
    pmt->MP5CLK       = pm_element(166);
    pmt->TWIXCLK      = pm_element(167);
    pmt->WAFLCLK      = pm_element(168);
    pmt->DPM_BUSY     = pm_element(169);
    pmt->MP1_BUSY     = pm_element(170);
    pmt->MP5_BUSY     = pm_element(171);
	
    assign_pm_elements_8_consec(pmt->CORE_POWER         , 172);
	assign_pm_elements_8_consec(pmt->CORE_VOLTAGE       , 180);
	assign_pm_elements_8_consec(pmt->CORE_TEMP          , 188);
	assign_pm_elements_8_consec(pmt->CORE_FIT           , 196);
	assign_pm_elements_8_consec(pmt->CORE_IDDMAX        , 204);
	assign_pm_elements_8_consec(pmt->CORE_FREQ          , 212);
	assign_pm_elements_8_consec(pmt->CORE_FREQEFF       , 220);
	assign_pm_elements_8_consec(pmt->CORE_C0            , 228);
	assign_pm_elements_8_consec(pmt->CORE_CC1           , 236);
	assign_pm_elements_8_consec(pmt->CORE_CC6           , 244);
	assign_pm_elements_8_consec(pmt->CORE_CKS_FDD       , 252);
	assign_pm_elements_8_consec(pmt->CORE_CI_FDD        , 260);
	assign_pm_elements_8_consec(pmt->CORE_IRM           , 268);
	assign_pm_elements_8_consec(pmt->CORE_PSTATE        , 276);
	assign_pm_elements_8_consec(pmt->CORE_FREQ_LIM_MAX  , 284);
	assign_pm_elements_8_consec(pmt->CORE_FREQ_LIM_MIN  , 292);
	assign_pm_elements_8_consec(pmt->CORE_SC_LIMIT      , 300);
	assign_pm_elements_8_consec(pmt->CORE_SC_CAC        , 308);
	assign_pm_elements_8_consec(pmt->CORE_SC_RESIDENCY  , 316);
	assign_pm_elements_8_consec(pmt->CORE_UOPS_CLK      , 324);
	assign_pm_elements_8_consec(pmt->CORE_UOPS          , 332);
	assign_pm_elements_8_consec(pmt->CORE_MEM_LATECY    , 340);
	
    pmt->L3_LOGIC_POWER[0] =                pm_element(348);
	pmt->L3_VDDM_POWER[0] =                 pm_element(349);
	pmt->L3_TEMP[0] =                       pm_element(350);
	pmt->L3_FIT[0] =                        pm_element(351);
	pmt->L3_IDDMAX[0] =                     pm_element(352);
	pmt->L3_FREQ[0] =                       pm_element(353);
	pmt->L3_FREQ_EFF[0] =                   pm_element(354);
	pmt->L3_CKS_FDD[0] =                    pm_element(355);
	pmt->L3_CCA_THRESHOLD[0] =              pm_element(356);
	pmt->L3_CCA_CAC[0] =                    pm_element(357);
	pmt->L3_CCA_ACTIVATION[0] =             pm_element(358);
	pmt->L3_EDC_LIMIT[0] =                  pm_element(359);
	pmt->L3_EDC_CAC[0] =                    pm_element(360);
	pmt->L3_EDC_RESIDENCY[0] =              pm_element(361);
    pmt->L3_FLL_BTC[0] =                    pm_element(362);
	//unkown 363
	
	pmt->min_size = 363*4; //(Highest element we access + 1)*4.
	//Needed to avoid illegal memory access
}


void pm_table_0x240903(pm_table *pmt, void* base_addr) {
    // Order of elements extracted from
    // https://gitlab.com/leogx9r/ryzen_smu/-/blob/master/userspace/monitor_cpu.c
    // Credit to Leonardo Gates <leogatesx9r@protonmail.com> under GPL V3
    //
    // Could not test myself, as I dont have the hardware. But it should work just fine.
    // Ryzen 3700X / 3800X

    pmt->version = 0x240903;
    pmt->max_cores = 8; //Number of cores supported by this PM table version
    pmt->zen_version = 2; //Zen2

    pmt->PPT_LIMIT = pm_element( 0);
    pmt->PPT_VALUE = pm_element( 1);
    pmt->TDC_LIMIT = pm_element( 2);
    pmt->TDC_VALUE = pm_element( 3);
    pmt->THM_LIMIT = pm_element( 4);
    pmt->THM_VALUE = pm_element( 5);
    pmt->FIT_LIMIT = pm_element( 6);
    pmt->FIT_VALUE = pm_element( 7);
    pmt->EDC_LIMIT = pm_element( 8);
    pmt->EDC_VALUE = pm_element( 9);
    pmt->VID_LIMIT = pm_element(10);
    pmt->VID_VALUE = pm_element(11);

    pmt->PPT_WC     = pm_element(12);
    pmt->PPT_ACTUAL = pm_element(13);
    pmt->TDC_WC     = pm_element(14);
    pmt->TDC_ACTUAL = pm_element(15);
    pmt->THM_WC     = pm_element(16);
    pmt->THM_ACTUAL = pm_element(17);
    pmt->FIT_WC     = pm_element(18);
    pmt->FIT_ACTUAL = pm_element(19);
    pmt->EDC_WC     = pm_element(20);
    pmt->EDC_ACTUAL = pm_element(21);
    pmt->VID_WC     = pm_element(22);
    pmt->VID_ACTUAL = pm_element(23);

    pmt->VDDCR_CPU_POWER = pm_element(24);
    pmt->VDDCR_SOC_POWER = pm_element(25);
    pmt->VDDIO_MEM_POWER = pm_element(26);
    pmt->VDD18_POWER     = pm_element(27);
    pmt->ROC_POWER       = pm_element(28);
    pmt->SOCKET_POWER    = pm_element(29);
    //pmt->PACKAGE_POWER   = pm_element(29); //PACKAGE_POWER does not exist in this table. It
                                           // *should* be somewhat similar to SOCKET_POWER.
                                           // However, since we show both fields in the output
                                           // there is no use in having it show the exact same
                                           // value two times. So don't set it.

    pmt->PPT_FREQUENCY     = pm_element(30);
    pmt->TDC_FREQUENCY     = pm_element(31);
    pmt->THM_FREQUENCY     = pm_element(32);
    pmt->PROCHOT_FREQUENCY = pm_element(33);
    pmt->VOLTAGE_FREQUENCY = pm_element(34);
    pmt->CCA_FREQUENCY     = pm_element(35);

    pmt->FIT_VOLTAGE            = pm_element(36);
    pmt->FIT_PRE_VOLTAGE        = pm_element(37);
    pmt->LATCHUP_VOLTAGE        = pm_element(38);
    pmt->CPU_SET_VOLTAGE        = pm_element(39);
    pmt->CPU_TELEMETRY_VOLTAGE  = pm_element(40);
    pmt->CPU_TELEMETRY_CURRENT  = pm_element(41);
    pmt->CPU_TELEMETRY_POWER    = pm_element(42);
    //CPU_TELEMETRY_POWER_ALT = pm_element(43);
    pmt->SOC_SET_VOLTAGE        = pm_element(44);
    pmt->SOC_TELEMETRY_VOLTAGE  = pm_element(45);
    pmt->SOC_TELEMETRY_CURRENT  = pm_element(46);
    pmt->SOC_TELEMETRY_POWER    = pm_element(47);

    pmt->FCLK_FREQ          = pm_element(48);
    pmt->FCLK_FREQ_EFF      = pm_element(49);
    pmt->UCLK_FREQ          = pm_element(50);
    pmt->MEMCLK_FREQ        = pm_element(51);
    pmt->FCLK_DRAM_SETPOINT = pm_element(52);
    pmt->FCLK_DRAM_BUSY     = pm_element(53);
    pmt->FCLK_GMI_SETPOINT  = pm_element(54);
    pmt->FCLK_GMI_BUSY      = pm_element(55);
    pmt->FCLK_IOHC_SETPOINT = pm_element(56);
    pmt->FCLK_IOHC_BUSY     = pm_element(57);
    pmt->FCLK_XGMI_SETPOINT = pm_element(58);
    pmt->FCLK_XGMI_BUSY     = pm_element(59);

    pmt->CCM_READS     = pm_element(60);
    pmt->CCM_WRITES    = pm_element(61);
    pmt->IOMS          = pm_element(62);
    pmt->XGMI          = pm_element(63);
    pmt->CS_UMC_READS  = pm_element(64);
    pmt->CS_UMC_WRITES = pm_element(65);
    assign_pm_elements_4(pmt->FCLK_RESIDENCY,     66,  67,  68,  69);
    assign_pm_elements_4(pmt->FCLK_FREQ_TABLE,    70,  71,  72,  73);
    assign_pm_elements_4(pmt->UCLK_FREQ_TABLE,    74,  75,  76,  77);
    assign_pm_elements_4(pmt->MEMCLK_FREQ_TABLE,  78,  79,  80,  81);
    assign_pm_elements_4(pmt->FCLK_VOLTAGE,       82,  83,  84,  85);

    assign_pm_elements_4(pmt->LCLK_SETPOINT,  86,  92,  98, 104);
    assign_pm_elements_4(pmt->LCLK_BUSY,      87,  93,  99, 105);
    assign_pm_elements_4(pmt->LCLK_FREQ,      88,  94, 100, 106);
    assign_pm_elements_4(pmt->LCLK_FREQ_EFF,  89,  95, 101, 107);
    assign_pm_elements_4(pmt->LCLK_MAX_DPM,   90,  96, 102, 108);
    assign_pm_elements_4(pmt->LCLK_MIN_DPM,   91,  97, 103, 109);

    pmt->XGMI_SETPOINT   = pm_element(110);
    pmt->XGMI_BUSY       = pm_element(111);
    pmt->XGMI_LANE_WIDTH = pm_element(112);
    pmt->XGMI_DATA_RATE  = pm_element(113);

    pmt->SOC_POWER           = pm_element(114);
    pmt->SOC_TEMP            = pm_element(115);
    pmt->DDR_VDDP_POWER      = pm_element(116);
    pmt->DDR_VDDIO_MEM_POWER = pm_element(117);
    pmt->GMI2_VDDG_POWER     = pm_element(118);
    pmt->IO_VDDCR_SOC_POWER  = pm_element(119);
    pmt->IOD_VDDIO_MEM_POWER = pm_element(120);
    pmt->IO_VDD18_POWER      = pm_element(121);
    pmt->TDP                 = pm_element(122);
    pmt->DETERMINISM         = pm_element(123);
    pmt->V_VDDM              = pm_element(124);
    pmt->V_VDDP              = pm_element(125);
    pmt->V_VDDG              = pm_element(126);

    pmt->PEAK_TEMP       = pm_element(127);
    pmt->PEAK_VOLTAGE    = pm_element(128);
    pmt->AVG_CORE_COUNT  = pm_element(129);
    pmt->CCLK_LIMIT      = pm_element(130);
    pmt->MAX_SOC_VOLTAGE = pm_element(131);
    pmt->DC_BTC          = pm_element(132);
    //pmt->PACKAGE_POWER   = pm_element(xxx); //Does not exist. Use SOCKET_POWER
    pmt->CSTATE_BOOST    = pm_element(133);
    pmt->PROCHOT         = pm_element(134);
    pmt->PC6             = pm_element(135);
    pmt->PWM             = pm_element(136);

    pmt->SOCCLK   = pm_element(137);
    pmt->SHUBCLK  = pm_element(138);
    pmt->MP0CLK   = pm_element(139);
    pmt->MP1CLK   = pm_element(140);
    pmt->MP5CLK   = pm_element(141);
    pmt->SMNCLK   = pm_element(142);
    pmt->TWIXCLK  = pm_element(143);
    pmt->WAFLCLK  = pm_element(144);

    pmt->DPM_BUSY = pm_element(145);
    pmt->MP1_BUSY = pm_element(146);
    pmt->MP5_BUSY = pm_element(325); //For some reason this is at the end of the table

    assign_pm_elements_8_consec(pmt->CORE_POWER       , 147);
    assign_pm_elements_8_consec(pmt->CORE_VOLTAGE     , 155);
    assign_pm_elements_8_consec(pmt->CORE_TEMP        , 163);
    assign_pm_elements_8_consec(pmt->CORE_FIT         , 171);
    assign_pm_elements_8_consec(pmt->CORE_IDDMAX      , 179);
    assign_pm_elements_8_consec(pmt->CORE_FREQ        , 187);
    assign_pm_elements_8_consec(pmt->CORE_FREQEFF     , 195);
    assign_pm_elements_8_consec(pmt->CORE_C0          , 203);
    assign_pm_elements_8_consec(pmt->CORE_CC1         , 211);
    assign_pm_elements_8_consec(pmt->CORE_CC6         , 219);
    assign_pm_elements_8_consec(pmt->CORE_CKS_FDD     , 227);
    assign_pm_elements_8_consec(pmt->CORE_CI_FDD      , 235);
    assign_pm_elements_8_consec(pmt->CORE_IRM         , 243);
    assign_pm_elements_8_consec(pmt->CORE_PSTATE      , 251);
    assign_pm_elements_8_consec(pmt->CORE_FREQ_LIM_MAX, 259);
    assign_pm_elements_8_consec(pmt->CORE_FREQ_LIM_MIN, 267);
    assign_pm_elements_8_consec(pmt->CORE_SC_LIMIT    , 275);
    assign_pm_elements_8_consec(pmt->CORE_SC_CAC      , 283);
    assign_pm_elements_8_consec(pmt->CORE_SC_RESIDENCY, 291);

    assign_pm_elements_2(pmt->L3_LOGIC_POWER   , 299, 300);
    assign_pm_elements_2(pmt->L3_VDDM_POWER    , 301, 302);
    assign_pm_elements_2(pmt->L3_TEMP          , 303, 304);
    assign_pm_elements_2(pmt->L3_FIT           , 305, 306);
    assign_pm_elements_2(pmt->L3_IDDMAX        , 307, 308);
    assign_pm_elements_2(pmt->L3_FREQ          , 309, 310);
    assign_pm_elements_2(pmt->L3_CKS_FDD       , 311, 312);
    assign_pm_elements_2(pmt->L3_CCA_THRESHOLD , 313, 314);
    assign_pm_elements_2(pmt->L3_CCA_CAC       , 315, 316);
    assign_pm_elements_2(pmt->L3_CCA_ACTIVATION, 317, 318);
    assign_pm_elements_2(pmt->L3_EDC_LIMIT     , 319, 320);
    assign_pm_elements_2(pmt->L3_EDC_CAC       , 321, 322);
    assign_pm_elements_2(pmt->L3_EDC_RESIDENCY , 323, 324);

    //MP5_BUSY is at 325

    pmt->min_size = 326*4; //(Highest element we access + 1)*4.
                           //Needed to avoid illegal memory access
}

void pm_table_0x240803(pm_table *pmt, void* base_addr) {
    // Ryzen 3950X
    // Could not test myself, as I dont have the hardware. But most values seem reasonable.
    // (Re)constructed using the pm table dumps from 
    // https://github.com/hattedsquirrel/ryzen_monitor/issues/2#

    pmt->version = 0x240803;
    pmt->max_cores = 16; //Number of cores supported by this PM table version
    pmt->max_l3 = 4; //Number of L3 caches supported by this PM table version
    pmt->zen_version = 2; //Zen2

    pmt->PPT_LIMIT = pm_element( 0);
    pmt->PPT_VALUE = pm_element( 1);
    pmt->TDC_LIMIT = pm_element( 2);
    pmt->TDC_VALUE = pm_element( 3);
    pmt->THM_LIMIT = pm_element( 4);
    pmt->THM_VALUE = pm_element( 5);
    pmt->FIT_LIMIT = pm_element( 6);
    pmt->FIT_VALUE = pm_element( 7);
    pmt->EDC_LIMIT = pm_element( 8);
    pmt->EDC_VALUE = pm_element( 9);
    pmt->VID_LIMIT = pm_element(10);
    pmt->VID_VALUE = pm_element(11);

    pmt->PPT_WC     = pm_element(12);
    pmt->PPT_ACTUAL = pm_element(13);
    pmt->TDC_WC     = pm_element(14);
    pmt->TDC_ACTUAL = pm_element(15);
    pmt->THM_WC     = pm_element(16);
    pmt->THM_ACTUAL = pm_element(17);
    pmt->FIT_WC     = pm_element(18);
    pmt->FIT_ACTUAL = pm_element(19);
    pmt->EDC_WC     = pm_element(20);
    pmt->EDC_ACTUAL = pm_element(21);
    pmt->VID_WC     = pm_element(22);
    pmt->VID_ACTUAL = pm_element(23);

    pmt->VDDCR_CPU_POWER = pm_element(24);
    pmt->VDDCR_SOC_POWER = pm_element(25);
    pmt->VDDIO_MEM_POWER = pm_element(26);
    pmt->VDD18_POWER     = pm_element(27);
    pmt->ROC_POWER       = pm_element(28);
    pmt->SOCKET_POWER    = pm_element(29);
    //pmt->PACKAGE_POWER   = pm_element(29); //PACKAGE_POWER does not exist in this table. It
                                           // *should* be somewhat similar to SOCKET_POWER.
                                           // However, since we show both fields in the output
                                           // there is no use in having it show the exact same
                                           // value two times. So don't set it.

    pmt->PPT_FREQUENCY     = pm_element(30);
    pmt->TDC_FREQUENCY     = pm_element(31);
    pmt->THM_FREQUENCY     = pm_element(32);
    pmt->PROCHOT_FREQUENCY = pm_element(33);
    pmt->VOLTAGE_FREQUENCY = pm_element(34);
    pmt->CCA_FREQUENCY     = pm_element(35);

    pmt->FIT_VOLTAGE            = pm_element(37);
    pmt->FIT_PRE_VOLTAGE        = pm_element(38);
    pmt->LATCHUP_VOLTAGE        = pm_element(39);
    pmt->CPU_SET_VOLTAGE        = pm_element(40);
    pmt->CPU_TELEMETRY_VOLTAGE  = pm_element(41);
    pmt->CPU_TELEMETRY_CURRENT  = pm_element(42);
    pmt->CPU_TELEMETRY_POWER    = pm_element(43);
    pmt->SOC_SET_VOLTAGE        = pm_element(44);
    pmt->SOC_TELEMETRY_VOLTAGE  = pm_element(45);
    pmt->SOC_TELEMETRY_CURRENT  = pm_element(46);
    pmt->SOC_TELEMETRY_POWER    = pm_element(47);

    pmt->FCLK_FREQ          = pm_element(48);
    pmt->FCLK_FREQ_EFF      = pm_element(49);
    pmt->UCLK_FREQ          = pm_element(50);
    pmt->MEMCLK_FREQ        = pm_element(51);
    pmt->FCLK_DRAM_SETPOINT = pm_element(52);
    pmt->FCLK_DRAM_BUSY     = pm_element(53);
    pmt->FCLK_GMI_SETPOINT  = pm_element(54);
    pmt->FCLK_GMI_BUSY      = pm_element(55);
    pmt->FCLK_IOHC_SETPOINT = pm_element(56);
    pmt->FCLK_IOHC_BUSY     = pm_element(57);
    pmt->FCLK_XGMI_SETPOINT = pm_element(58);
    pmt->FCLK_XGMI_BUSY     = pm_element(59);

    pmt->CCM_READS     = pm_element(60);
    pmt->CCM_WRITES    = pm_element(61);
    pmt->IOMS          = pm_element(62);
    pmt->XGMI          = pm_element(63);
    pmt->CS_UMC_READS  = pm_element(64);
    pmt->CS_UMC_WRITES = pm_element(65);
    assign_pm_elements_4(pmt->FCLK_RESIDENCY,     66,  67,  68,  69);
    assign_pm_elements_4(pmt->FCLK_FREQ_TABLE,    70,  71,  72,  73);
    assign_pm_elements_4(pmt->UCLK_FREQ_TABLE,    74,  75,  76,  77);
    assign_pm_elements_4(pmt->MEMCLK_FREQ_TABLE,  78,  79,  80,  81);
    assign_pm_elements_4(pmt->FCLK_VOLTAGE,       82,  83,  84,  85);

    assign_pm_elements_4(pmt->LCLK_SETPOINT,  86,  92,  98, 104);
    assign_pm_elements_4(pmt->LCLK_BUSY,      87,  93,  99, 105);
    assign_pm_elements_4(pmt->LCLK_FREQ,      88,  94, 100, 106);
    assign_pm_elements_4(pmt->LCLK_FREQ_EFF,  89,  95, 101, 107);
    assign_pm_elements_4(pmt->LCLK_MAX_DPM,   90,  96, 102, 108);
    assign_pm_elements_4(pmt->LCLK_MIN_DPM,   91,  97, 103, 109);

    pmt->XGMI_SETPOINT   = pm_element(110);
    pmt->XGMI_BUSY       = pm_element(111);
    pmt->XGMI_LANE_WIDTH = pm_element(112);
    pmt->XGMI_DATA_RATE  = pm_element(113);

    pmt->SOC_POWER           = pm_element(114);
    pmt->SOC_TEMP            = pm_element(115);
    pmt->DDR_VDDP_POWER      = pm_element(116);
    pmt->DDR_VDDIO_MEM_POWER = pm_element(117);
    pmt->GMI2_VDDG_POWER     = pm_element(118);
    pmt->IO_VDDCR_SOC_POWER  = pm_element(119);
    pmt->IOD_VDDIO_MEM_POWER = pm_element(120);
    pmt->IO_VDD18_POWER      = pm_element(121);
    pmt->TDP                 = pm_element(122);
    pmt->DETERMINISM         = pm_element(123);
    pmt->V_VDDM              = pm_element(124);
    pmt->V_VDDP              = pm_element(125);
    pmt->V_VDDG              = pm_element(126);

    pmt->PEAK_TEMP       = pm_element(127);
    pmt->PEAK_VOLTAGE    = pm_element(128);
    pmt->AVG_CORE_COUNT  = pm_element(129);
    pmt->CCLK_LIMIT      = pm_element(130);
    pmt->MAX_SOC_VOLTAGE = pm_element(131);
    pmt->DC_BTC          = pm_element(132);
    //pmt->PACKAGE_POWER   = pm_element(xxx); //Does not exist. Use SOCKET_POWER
    pmt->CSTATE_BOOST    = pm_element(133);
    pmt->PROCHOT         = pm_element(134);
    pmt->PC6             = pm_element(135);
    pmt->PWM             = pm_element(136);

    pmt->SOCCLK   = pm_element(137); //elem 137 -> 457.14
    pmt->SHUBCLK  = pm_element(138); //elem 138 -> 457.14
    pmt->MP0CLK   = pm_element(139); //elem 139 -> 457.14
    pmt->MP1CLK   = pm_element(140); //elem 140 -> 457.14
    pmt->MP5CLK   = pm_element(141); //elem 141 -> 400.00
    pmt->SMNCLK   = pm_element(142); //elem 142 -> 500.00
    pmt->TWIXCLK  = pm_element(143); //elem 143 -> 400.00
    pmt->WAFLCLK  = pm_element(144);

    pmt->DPM_BUSY = pm_element(145);
    pmt->MP1_BUSY = pm_element(146);

    assign_pm_elements_16_consec(pmt->CORE_POWER       , 147);
    assign_pm_elements_16_consec(pmt->CORE_VOLTAGE     , 163);
    assign_pm_elements_16_consec(pmt->CORE_TEMP        , 179);
    assign_pm_elements_16_consec(pmt->CORE_FIT         , 195);
    assign_pm_elements_16_consec(pmt->CORE_IDDMAX      , 211);
    assign_pm_elements_16_consec(pmt->CORE_FREQ        , 227);
    assign_pm_elements_16_consec(pmt->CORE_FREQEFF     , 243);
    assign_pm_elements_16_consec(pmt->CORE_C0          , 259);
    assign_pm_elements_16_consec(pmt->CORE_CC1         , 275);
    assign_pm_elements_16_consec(pmt->CORE_CC6         , 291);
    assign_pm_elements_16_consec(pmt->CORE_CKS_FDD     , 307);
    assign_pm_elements_16_consec(pmt->CORE_CI_FDD      , 323);
    assign_pm_elements_16_consec(pmt->CORE_IRM         , 339);
    assign_pm_elements_16_consec(pmt->CORE_PSTATE      , 355);
    assign_pm_elements_16_consec(pmt->CORE_FREQ_LIM_MAX, 371);
    assign_pm_elements_16_consec(pmt->CORE_FREQ_LIM_MIN, 387);
    assign_pm_elements_16_consec(pmt->CORE_SC_LIMIT    , 403);
    assign_pm_elements_16_consec(pmt->CORE_SC_CAC      , 419);
    assign_pm_elements_16_consec(pmt->CORE_SC_RESIDENCY, 435);

    assign_pm_elements_4_consec(pmt->L3_LOGIC_POWER   , 451);
    assign_pm_elements_4_consec(pmt->L3_VDDM_POWER    , 455);
    assign_pm_elements_4_consec(pmt->L3_TEMP          , 459);
    assign_pm_elements_4_consec(pmt->L3_FIT           , 463);
    assign_pm_elements_4_consec(pmt->L3_IDDMAX        , 467);
    assign_pm_elements_4_consec(pmt->L3_FREQ          , 471);
    assign_pm_elements_4_consec(pmt->L3_CKS_FDD       , 475);
    assign_pm_elements_4_consec(pmt->L3_CCA_THRESHOLD , 479);
    assign_pm_elements_4_consec(pmt->L3_CCA_CAC       , 483);
    assign_pm_elements_4_consec(pmt->L3_CCA_ACTIVATION, 487);
    assign_pm_elements_4_consec(pmt->L3_EDC_LIMIT     , 491);
    assign_pm_elements_4_consec(pmt->L3_EDC_CAC       , 495);
    assign_pm_elements_4_consec(pmt->L3_EDC_RESIDENCY , 499);
    assign_pm_elements_2(pmt->MP5_BUSY, 503, 504);

    pmt->min_size = 505*4; //(Highest element we access + 1)*4.
                           //Needed to avoid illegal memory access
}
