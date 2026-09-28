/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 *
 */

#ifndef GAME_DB_H
#define GAME_DB_H

#include "definitions.h"

#define GB_DB_DEFAULT_MAPPER 0
#define GB_DB_M161_MAPPER 1
#define GB_DB_MMM01_MAPPER 2
#define GB_DB_SACHEN_MMC1_MAPPER 3
#define GB_DB_SACHEN_MMC2_MAPPER 4
#define GB_DB_BUNG_EMS_MAPPER 5
#define GB_DB_POKE2IN1_MAPPER 6
#define GB_DB_PKJD_MAPPER 7
#define GB_DB_ROCKET_MAPPER 8
#define GB_DB_BHGOS_MAPPER 9
#define GB_DB_LICHENG_MAPPER 10
#define GB_DB_NTNEW_MAPPER 11
#define GB_DB_GGB81_MAPPER 12
#define GB_DB_HITEK_MAPPER 13
#define GB_DB_VF001_MAPPER 14
#define GB_DB_SINTAX_MAPPER 15
#define GB_DB_NTOLD1_MAPPER 16
#define GB_DB_NTOLD2_MAPPER 17

#define GB_DB_FEATURE_NONE 0x00
#define GB_DB_FEATURE_BARCODE_BOY 0x01
#define GB_DB_FEATURE_MMM01_MENU_AT_END 0x02
#define GB_DB_FEATURE_NO_BOOT_LOGO_SWAP 0x04

enum GB_GameDBCRCType
{
    GB_DB_CRC_FULL,
    GB_DB_CRC_HEADER
};

struct GB_GameDBEntry
{
    u32 crc;
    GB_GameDBCRCType crc_type;
    u8 mapper;
    int features;
    const char* title;
};

const GB_GameDBEntry kGameDatabase[] =
{
    { 0x0C38A775, GB_DB_CRC_FULL, GB_DB_M161_MAPPER, GB_DB_FEATURE_NONE,
      "Mani 4 in 1 - Tetris / Alleyway / Yakuman / Tennis" },
    { 0xA61F3EE1, GB_DB_CRC_HEADER, GB_DB_M161_MAPPER, GB_DB_FEATURE_NONE,
      "Mani 4 in 1 - Tetris / Alleyway / Yakuman / Tennis (header)" },

    { 0x5BFC3EF5, GB_DB_CRC_FULL, GB_DB_MMM01_MAPPER, GB_DB_FEATURE_MMM01_MENU_AT_END,
      "Mani 4 in 1 - Bubble Bobble / Elevator Action / Chase H.Q. / Sagaia" },
    { 0xC373AC09, GB_DB_CRC_FULL, GB_DB_MMM01_MAPPER, GB_DB_FEATURE_MMM01_MENU_AT_END,
      "Mani 4 in 1 - Gambaruger / Raijin-Oh / Zoids / Esparks" },
    { 0xCB48B6D0, GB_DB_CRC_FULL, GB_DB_MMM01_MAPPER, GB_DB_FEATURE_MMM01_MENU_AT_END,
      "Mani 4 in 1 - R-Type II / Saigo no Nindou / Yancha Maru / Shisenshou" },
    { 0x950773EE, GB_DB_CRC_FULL, GB_DB_MMM01_MAPPER, GB_DB_FEATURE_MMM01_MENU_AT_END,
      "Mani 4 in 1 - Adventure Island II / GB Genjin / Bomber Boy / Milon" },

    { 0x82F06E93, GB_DB_CRC_FULL, GB_DB_SACHEN_MMC1_MAPPER, GB_DB_FEATURE_NONE,
      "4 in 1 (Europe) (4B-001, Sachen-Commin)" },
    { 0x5E438DB8, GB_DB_CRC_FULL, GB_DB_SACHEN_MMC1_MAPPER, GB_DB_FEATURE_NONE, "4 in 1 (Europe) (4B-002, Sachen)" },
    { 0xC294AA21, GB_DB_CRC_FULL, GB_DB_SACHEN_MMC1_MAPPER, GB_DB_FEATURE_NONE,
      "4 in 1 (Taiwan) (4B-003, Sachen-Commin)" },
    { 0xC69A19F6, GB_DB_CRC_FULL, GB_DB_SACHEN_MMC1_MAPPER, GB_DB_FEATURE_NONE,
      "4 in 1 (Europe) (4B-004, Sachen-Commin)" },
    { 0xF4310EB3, GB_DB_CRC_FULL, GB_DB_SACHEN_MMC1_MAPPER, GB_DB_FEATURE_NONE,
      "4 in 1 (Europe) (4B-005, Sachen-Commin)" },
    { 0x95398DA5, GB_DB_CRC_FULL, GB_DB_SACHEN_MMC1_MAPPER, GB_DB_FEATURE_NONE, "4 in 1 (Europe) (4B-006, Sachen)" },
    { 0x62D9350E, GB_DB_CRC_FULL, GB_DB_SACHEN_MMC1_MAPPER, GB_DB_FEATURE_NONE, "4 in 1 (Europe) (4B-007, Sachen)" },
    { 0x740E9BC8, GB_DB_CRC_FULL, GB_DB_SACHEN_MMC1_MAPPER, GB_DB_FEATURE_NONE, "4 in 1 (Europe) (4B-008, Sachen)" },
    { 0x114E1F1E, GB_DB_CRC_FULL, GB_DB_SACHEN_MMC1_MAPPER, GB_DB_FEATURE_NONE, "4 in 1 (Europe) (4B-009, Sachen)" },

    { 0x0AF7C09A, GB_DB_CRC_HEADER, GB_DB_ROCKET_MAPPER, GB_DB_FEATURE_NONE, "ATV Racing & Karate Joe" },
    { 0x1F4954E4, GB_DB_CRC_HEADER, GB_DB_ROCKET_MAPPER, GB_DB_FEATURE_NONE,
      "Full Time Soccer / Full Time Soccer & Hang Time Basketball" },
    { 0x2C26C119, GB_DB_CRC_HEADER, GB_DB_ROCKET_MAPPER, GB_DB_FEATURE_NONE, "Karate Joe" },
    { 0x3AFBB401, GB_DB_CRC_HEADER, GB_DB_ROCKET_MAPPER, GB_DB_FEATURE_NONE, "Painter" },
    { 0x40A83DEC, GB_DB_CRC_HEADER, GB_DB_ROCKET_MAPPER, GB_DB_FEATURE_NONE, "ATV Racing" },
    { 0x6C1CFF79, GB_DB_CRC_HEADER, GB_DB_ROCKET_MAPPER, GB_DB_FEATURE_NONE, "Hang Time Basketball" },
    { 0xDA964D17, GB_DB_CRC_HEADER, GB_DB_ROCKET_MAPPER, GB_DB_FEATURE_NONE,
      "Race Time / Pocket Smash Out & Race Time" },
    { 0xFFC6A7BD, GB_DB_CRC_HEADER, GB_DB_ROCKET_MAPPER, GB_DB_FEATURE_NONE, "Pocket Smash Out" },

    { 0x2ED509D9, GB_DB_CRC_FULL, GB_DB_BUNG_EMS_MAPPER, GB_DB_FEATURE_NONE, "Green Beret" },
    { 0xF004440C, GB_DB_CRC_FULL, GB_DB_BUNG_EMS_MAPPER, GB_DB_FEATURE_NONE, "Cube Raider" },
    { 0xFDC1483A, GB_DB_CRC_FULL, GB_DB_BUNG_EMS_MAPPER, GB_DB_FEATURE_NONE, "Bugs Bunny - Crazy Castle 3" },

    { 0x519C04CB, GB_DB_CRC_HEADER, GB_DB_BHGOS_MAPPER, GB_DB_FEATURE_NONE, "BHGOS multicart menu" },

    { 0xBA03BD71, GB_DB_CRC_FULL, GB_DB_LICHENG_MAPPER, GB_DB_FEATURE_NONE,
      "Shuma Baolong - Shuijing Ban (CBA011)" },
    { 0xA477C7CE, GB_DB_CRC_FULL, GB_DB_LICHENG_MAPPER, GB_DB_FEATURE_NONE,
      "Mingzhu Koudai Guaishou 3 (CBA065)" },
    { 0x5F2D6317, GB_DB_CRC_FULL, GB_DB_LICHENG_MAPPER, GB_DB_FEATURE_NONE,
      "Yingxiong Tianxia (CBA079)" },
    { 0x23F291B3, GB_DB_CRC_FULL, GB_DB_LICHENG_MAPPER, GB_DB_FEATURE_NONE,
      "Taikong Baobei (CBA083)" },

    { 0x97280455, GB_DB_CRC_FULL, GB_DB_NTNEW_MAPPER, GB_DB_FEATURE_NONE,
      "Capcom vs SNK - Millennium Fight 2001" },
    { 0xA90061E9, GB_DB_CRC_FULL, GB_DB_NTNEW_MAPPER, GB_DB_FEATURE_NONE,
      "Digimon 02 4" },
    { 0xA43AB22B, GB_DB_CRC_FULL, GB_DB_NTNEW_MAPPER, GB_DB_FEATURE_NONE,
      "Digimon 2" },
    { 0x6791B106, GB_DB_CRC_FULL, GB_DB_NTNEW_MAPPER, GB_DB_FEATURE_NONE,
      "Digimon Pocket" },
    { 0xD0FECE32, GB_DB_CRC_FULL, GB_DB_NTNEW_MAPPER, GB_DB_FEATURE_NONE,
      "Harry Potter 3" },
    { 0x6E447A33, GB_DB_CRC_FULL, GB_DB_NTNEW_MAPPER, GB_DB_FEATURE_NONE,
      "Pokemon - Mewtwo Strikes Back" },
    { 0x1B5BEF4B, GB_DB_CRC_FULL, GB_DB_NTNEW_MAPPER, GB_DB_FEATURE_NONE,
      "Pokemon Diamond (Special Pikachu Edition)" },
    { 0xE9488E13, GB_DB_CRC_FULL, GB_DB_NTNEW_MAPPER, GB_DB_FEATURE_NONE,
      "Pokemon Jade Version (Special Pikachu Edition)" },
    { 0x2EE18AB2, GB_DB_CRC_FULL, GB_DB_NTNEW_MAPPER, GB_DB_FEATURE_NONE,
      "Shuma Baolong 02 4" },
    { 0x1A6FE765, GB_DB_CRC_FULL, GB_DB_NTNEW_MAPPER, GB_DB_FEATURE_NONE,
      "Street Fighter Zero 4 / Jieba Tianwang 4" },

    { 0xBE4C1D83, GB_DB_CRC_FULL, GB_DB_GGB81_MAPPER, GB_DB_FEATURE_NO_BOOT_LOGO_SWAP,
      "Digimon Sapphire (BC-R1616T3P)" },
    { 0x7AA7EEA5, GB_DB_CRC_FULL, GB_DB_GGB81_MAPPER, GB_DB_FEATURE_NONE,
      "Mo Jie Chuan Shuo (DSHGGB-81)" },
    { 0x416E6EFA, GB_DB_CRC_FULL, GB_DB_GGB81_MAPPER, GB_DB_FEATURE_NONE,
      "Mu Chang Wu Yu GB 6 (DSHGGB-81)" },
    { 0x9507E5D3, GB_DB_CRC_FULL, GB_DB_GGB81_MAPPER, GB_DB_FEATURE_NONE,
      "Shu Ma Bao Long - Kou Dai Ban (DSHGGB-81)" },

    { 0xA4AD3678, GB_DB_CRC_FULL, GB_DB_HITEK_MAPPER, GB_DB_FEATURE_NONE,
      "Shuihu Zhuan Zhi Qunmo Fengyun Lu" },
    { 0x188B06F8, GB_DB_CRC_FULL, GB_DB_HITEK_MAPPER, GB_DB_FEATURE_NONE,
      "Terrifying 911" },

    { 0xAFD7A0CC, GB_DB_CRC_FULL, GB_DB_VF001_MAPPER, GB_DB_FEATURE_NONE,
      "Chao Ji Ge Dou 2001 Alpha" },
    { 0x1B1C6F68, GB_DB_CRC_FULL, GB_DB_VF001_MAPPER, GB_DB_FEATURE_NONE,
      "Ge Dou Jian Shen - Soul Falchion" },
    { 0xE1668B49, GB_DB_CRC_FULL, GB_DB_VF001_MAPPER, GB_DB_FEATURE_NONE,
      "Nv Wang Ge Dou 2000" },

    { 0x973D38A8, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "2003 Crash II Advance" },
    { 0x7CFF9F0B, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "2003 Digimom Sapphii (2003 Digitmon Sapphire)" },
    { 0x74D71B0C, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "2003 Gu Huo Lang II" },
    { 0x1B3E1243, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "2003 Ha Li Bo Te 2 - Xiao Shi De Mi Shi" },
    { 0xDB3C8B95, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "2003 Ha Li Xiao Zi IV" },
    { 0x4C76D4D8, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "2003 Kou Dai Guai Shou - Lan Bao Shi" },
    { 0x0C22466D, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "2003 Lion King Advance 3 (The King Lion III 2003 Advance)" },
    { 0x3A0E9B6F, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "2003 Pocket Monster Carbuncle (Pocket Monster Ruby)" },
    { 0x1219EEC6, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "2003 Shu Ma Bao Long - Ge Dou Ban" },
    { 0xC4EBA914, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Bing Yuan Li Xian Ji II" },
    { 0x1FEAEB47, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Chao Ji Ji Qi Ren Da Zhan X - Super Robot War X (Alt)" },
    { 0x279BE0CC, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Chao Ji Ji Qi Ren Da Zhan X - Super Robot War X" },
    { 0xE1D61242, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Chao Ren Te Gong Dui" },
    { 0xA9DD9DA7, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Chaoji Yinsu Xiaozi II - Super Sonik II" },
    { 0x0B20D3AF, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Chuan Shuo" },
    { 0x685E76DF, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Digimon Crystal II" },
    { 0x96662E76, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Digimon Yellow Jade" },
    { 0x1A369DD5, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Donkey Kong 5 - The Journey of Over Time and Space" },
    { 0xDBE0F44E, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Dragon Ball Z 3 2002 Fighting" },
    { 0x8059E009, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Feng Kuang A Gei III - Chao Ji Zha Dan Ren" },
    { 0x842CB4FE, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Feng Zhi Gou II" },
    { 0x955BC6AD, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Gui Wu Zhe 2" },
    { 0x859457C8, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Ha Li Xiao Zi Di Er Bu - Mi Shi De Mi" },
    { 0xA962AD73, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Hai Zhan Qi Bing" },
    { 0xEF4BBB34, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "He Jin Zhuang Bei II" },
    { 0xF19E780C, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Jing Ling Wang III" },
    { 0xF4D63A7E, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Jue Dui Wu Li" },
    { 0x998AD382, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Kou Dai Yao Guai - Bai Jin Ban" },
    { 0x34C1A63A, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Langrsr II (Fantastic Simulated Battle)" },
    { 0xAECA45BE, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Lao Fuzi Chuanqi" },
    { 0x51F92403, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Luo Ke Ying Xiong EXE5 (Luo Ke Ren X5)" },
    { 0x55494332, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Matel Gear II" },
    { 0x366067E8, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Menghuan Moni Zhan II" },
    { 0x80C4FD52, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Mo Shou Shi Ji - Zhan Shen" },
    { 0x2C0D43A9, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Movie Version - Spider-Man 3" },
    { 0x0DB9FDFA, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Pian Wai Zhang Huang Jin Tai Yang - Feng Yin De Yuan Gu Lian Jin Shu" },
    { 0x71536B8E, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Pokemon Sapphire Version (Pocket Monster Saphire)" },
    { 0xCCFDD63A, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Qi Long Zhu Z 3 (Dragon Ball - Advance Adventure)" },
    { 0x2C922ED6, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Quan Ba Tian Xia" },
    { 0x8A64C933, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "San Guo Wu Shang 5 (Bynasty Warriors Advance 5)" },
    { 0x25A857AF, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Shao Lin Shi San Gun - Ying Xiong Jiu Zhu" },
    { 0xA321FF7D, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Shaoling Legend - Hero, the Saver (Fantasic ShaoLin Kungfu)" },
    { 0x1823E24D, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Shengui Diguo Zhi Emo Cheng" },
    { 0xB67999AE, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Shu Ma Bao Long - Shui Jing Ban II" },
    { 0x509F1BB5, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Space Baby (The Firmament Baby)" },
    { 0x031A82DC, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Super Marrio Sunshine" },
    { 0xF2D0F0AE, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Tai Kong Bao Bei" },
    { 0xC01BBC48, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "The Lord of the Rings - The Fellowship of the Ring (King of Ring)" },
    { 0xCBA2C784, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "The second edition Harry Boy - The secretx of the chamber of secrets (Harry Boy and the Chamber of Secrets)" },
    { 0x4E600093, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Wai Xing Tan Xian Zhi Xing Qiu Da Zhan" },
    { 0xDEE597FC, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Wang Zu Tian Tang" },
    { 0xE81701E8, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Xian Dan Chao Ren - Ultraman" },
    { 0x322C3816, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Xiao Tai Ji - Shen Hua Li Xian" },
    { 0x6DF86DB6, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Xing Qiu Da Zhan II - Ke Long Ren Zhan Yi" },
    { 0x45B048EE, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Yi Xing VS Tie Xue Zhan Shi" },
    { 0x8B8B84EC, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Yong Zhe Dou E Long VIII" },
    { 0x9268BB9F, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Yue Nan Zhan Yi 3" },
    { 0x602951A6, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Yuenan Zhanyi X - Shenru Dihou" },
    { 0x95C322C9, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Zhen San Guo Wu Shuang 2 - Shin Sangokumusou 2" },
    { 0x8160B3F5, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Zhi Huan Wang - Shou Bu Qu" },
    { 0xD311EFCC, GB_DB_CRC_FULL, GB_DB_SINTAX_MAPPER, GB_DB_FEATURE_NONE,
      "Zhi Zhu Xia III - Dian Ying Ban" },

    { 0xC47392AB, GB_DB_CRC_FULL, GB_DB_NTOLD1_MAPPER, GB_DB_FEATURE_NONE,
      "True Color 25 in 1 (NT-9920)" },
    { 0xABEBCB28, GB_DB_CRC_FULL, GB_DB_NTOLD1_MAPPER, GB_DB_FEATURE_NONE,
      "Rockman 8 (Multicart Rip)" },
    { 0x1E45FC45, GB_DB_CRC_FULL, GB_DB_NTOLD2_MAPPER, GB_DB_FEATURE_NONE,
      "23 in 1 (CR2011)" },
    { 0x78C0E5BC, GB_DB_CRC_FULL, GB_DB_NTOLD2_MAPPER, GB_DB_FEATURE_NONE,
      "29 in 1 (CR2020 / CY2061)" },
    { 0xDC03FB00, GB_DB_CRC_FULL, GB_DB_NTOLD2_MAPPER, GB_DB_FEATURE_NONE,
      "Caise Gedou 24 in 1 (CY2060)" },
    { 0x043A17B3, GB_DB_CRC_FULL, GB_DB_NTOLD2_MAPPER, GB_DB_FEATURE_NONE,
      "Rockman X4 (Megaman X4)" },
    { 0xC5D8B776, GB_DB_CRC_FULL, GB_DB_NTOLD2_MAPPER, GB_DB_FEATURE_NONE,
      "Sonic Adventure 8" },
    { 0xD44C9806, GB_DB_CRC_FULL, GB_DB_NTOLD2_MAPPER, GB_DB_FEATURE_NONE,
      "Super Donkey Kong 5" },
    { 0x33093C28, GB_DB_CRC_FULL, GB_DB_NTOLD2_MAPPER, GB_DB_FEATURE_NONE,
      "Super Donkey Kong 5 (Alt)" },
    { 0x5E4266A7, GB_DB_CRC_FULL, GB_DB_NTOLD2_MAPPER, GB_DB_FEATURE_NONE,
      "Super Mario Special 3 (Multicart Rip)" },

    { 0xABB17913, GB_DB_CRC_FULL, GB_DB_POKE2IN1_MAPPER, GB_DB_FEATURE_NONE, "Pokemon Red-Blue 2-in-1 (Unl) [S]" },

    { 0x30F8F86C, GB_DB_CRC_HEADER, GB_DB_PKJD_MAPPER, GB_DB_FEATURE_NONE,
      "Pokemon Jade Version (Telefang Speed bootleg)" },

    { 0xA828FD4F, GB_DB_CRC_FULL, GB_DB_DEFAULT_MAPPER, GB_DB_FEATURE_BARCODE_BOY, "Battle Space" },
    { 0x56BA6A71, GB_DB_CRC_FULL, GB_DB_DEFAULT_MAPPER, GB_DB_FEATURE_BARCODE_BOY, "Monster Maker - Barcode Saga" },
    { 0xEFBC23FC, GB_DB_CRC_FULL, GB_DB_DEFAULT_MAPPER, GB_DB_FEATURE_BARCODE_BOY, "Kattobi Road" },
    { 0x0325E729, GB_DB_CRC_FULL, GB_DB_DEFAULT_MAPPER, GB_DB_FEATURE_BARCODE_BOY,
      "Family Jockey 2 - Meiba no Kettou" },
    { 0xBDC4CCC3, GB_DB_CRC_FULL, GB_DB_DEFAULT_MAPPER, GB_DB_FEATURE_BARCODE_BOY, "Famista 3" },

    { 0, GB_DB_CRC_FULL, GB_DB_DEFAULT_MAPPER, GB_DB_FEATURE_NONE, NULL }
};

#endif /* GAME_DB_H */
