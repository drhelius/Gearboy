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

#ifndef BARCODE_BOY_CODES_H
#define BARCODE_BOY_CODES_H

struct GB_BarcodeBoyCode
{
    const char* name;
    const char* barcode;
};

// Card data: https://shonumi.github.io/dandocs.html#barcode-boy

static const GB_BarcodeBoyCode kBarcodeBoyCodes[] =
{
    { "Battle Space - Berserker", "4907981000301" },
    { "Battle Space - Valkyrie", "4908052808369" },
    { "Battle Space - Grizzly Bear", "4916911302309" },
    { "Battle Space - Magic Soldier", "4902776809367" },
    { "Battle Space - Knight", "4905672306367" },
    { "Battle Space - Wraith", "4912713004366" },
    { "Battle Space - Shaman", "4913508504399" },
    { "Battle Space - Thief", "4918156001351" },
    { "Battle Space - Sorcerer", "4911826551347" },
    { "Battle Space - Warrior", "4909062206350" },

    { "Family Jockey 2 - A1", "5893713522816" },
    { "Family Jockey 2 - A2", "2378649896765" },
    { "Family Jockey 2 - A4", "9845554422318" },
    { "Family Jockey 2 - B1", "1509843019075" },
    { "Family Jockey 2 - B2", "4232978865152" },
    { "Family Jockey 2 - B4", "3572821107673" },
    { "Family Jockey 2 - C3", "7164625542390" },
    { "Family Jockey 2 - C5", "6319537443513" },

    { "Famista 3 - Home-Run Batter", "8357933639923" },
    { "Famista 3 - Senior Batter", "7814374127798" },
    { "Famista 3 - Swift Batter", "9880692151263" },
    { "Famista 3 - Pitcher", "1414213562177" },

    { "Kattobi Road - Truck", "4902105002063" },
    { "Kattobi Road - Sedan", "4901121110004" },
    { "Kattobi Road - Racecar", "4903301160625" },
    { "Kattobi Road - Japanese Street Car", "4902888119101" },
    { "Kattobi Road - 4x4 Jeep", "4901780161157" },
    { "Kattobi Road - F1-style Racecar", "4987084410924" },

    { "Monster Maker - Archer Lorian", "9998017308336" },
    { "Monster Maker - Archer Elysice", "9447410810323" },
    { "Monster Maker - Knight Lauren", "9052091324955" },
    { "Monster Maker - Dragon Knight Haagun", "9322158686716" },
    { "Monster Maker - Warrior Diane", "9752412234900" },
    { "Monster Maker - Warrior Tamron", "9362462085911" },

    { 0, 0 }
};

#endif /* BARCODE_BOY_CODES_H */
