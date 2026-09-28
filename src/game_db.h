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

#define GB_DB_FEATURE_NONE 0x00
#define GB_DB_FEATURE_BARCODE_BOY 0x01
#define GB_DB_FEATURE_MMM01_MENU_AT_END 0x02

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
