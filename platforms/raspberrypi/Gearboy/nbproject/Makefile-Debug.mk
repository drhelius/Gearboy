#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Gearboy-Makefile.mk

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/1386528437/MBC2MemoryRule.o \
	${OBJECTDIR}/_ext/1386528437/Audio.o \
	${OBJECTDIR}/_ext/1386528437/MBC1MemoryRule.o \
	${OBJECTDIR}/_ext/1386528437/IORegistersMemoryRule.o \
	${OBJECTDIR}/_ext/1680311292/Gb_Apu.o \
	${OBJECTDIR}/_ext/1386528437/MultiMBC1MemoryRule.o \
	${OBJECTDIR}/_ext/1386528437/GearboyCore.o \
	${OBJECTDIR}/_ext/1680311292/Multi_Buffer.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/_ext/1680311292/Effects_Buffer.o \
	${OBJECTDIR}/_ext/1386528437/MBC5MemoryRule.o \
	${OBJECTDIR}/_ext/1680311292/Gb_Apu_State.o \
	${OBJECTDIR}/_ext/1386528437/SixteenBitRegister.o \
	${OBJECTDIR}/_ext/1680311292/Blip_Buffer.o \
	${OBJECTDIR}/_ext/1386528437/EightBitRegister.o \
	${OBJECTDIR}/_ext/1386528437/MemoryRule.o \
	${OBJECTDIR}/_ext/1386528437/Input.o \
	${OBJECTDIR}/_ext/1386528437/Processor.o \
	${OBJECTDIR}/_ext/1386528437/Video.o \
	${OBJECTDIR}/_ext/1386528437/Memory.o \
	${OBJECTDIR}/_ext/1386528437/Cartridge.o \
	${OBJECTDIR}/_ext/1386528437/MBC3MemoryRule.o \
	${OBJECTDIR}/_ext/1386528437/RomOnlyMemoryRule.o \
	${OBJECTDIR}/_ext/1386528437/CommonMemoryRule.o \
	${OBJECTDIR}/_ext/1680311292/Sound_Queue.o \
	${OBJECTDIR}/_ext/1680311292/Gb_Oscs.o \
	${OBJECTDIR}/_ext/1386528437/opcodes.o \
	${OBJECTDIR}/_ext/1386528437/opcodes_cb.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lglut -lGL -lGLU -lSDL -lGLEW

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gearboy

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gearboy: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gearboy ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/1386528437/MBC2MemoryRule.o: ../../../src/MBC2MemoryRule.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/MBC2MemoryRule.o ../../../src/MBC2MemoryRule.cpp

${OBJECTDIR}/_ext/1386528437/Audio.o: ../../../src/Audio.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Audio.o ../../../src/Audio.cpp

${OBJECTDIR}/_ext/1386528437/MBC1MemoryRule.o: ../../../src/MBC1MemoryRule.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/MBC1MemoryRule.o ../../../src/MBC1MemoryRule.cpp

${OBJECTDIR}/_ext/1386528437/IORegistersMemoryRule.o: ../../../src/IORegistersMemoryRule.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/IORegistersMemoryRule.o ../../../src/IORegistersMemoryRule.cpp

${OBJECTDIR}/_ext/1680311292/Gb_Apu.o: ../../../src/audio/Gb_Apu.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1680311292
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1680311292/Gb_Apu.o ../../../src/audio/Gb_Apu.cpp

${OBJECTDIR}/_ext/1386528437/MultiMBC1MemoryRule.o: ../../../src/MultiMBC1MemoryRule.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/MultiMBC1MemoryRule.o ../../../src/MultiMBC1MemoryRule.cpp

${OBJECTDIR}/_ext/1386528437/GearboyCore.o: ../../../src/GearboyCore.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/GearboyCore.o ../../../src/GearboyCore.cpp

${OBJECTDIR}/_ext/1680311292/Multi_Buffer.o: ../../../src/audio/Multi_Buffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1680311292
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1680311292/Multi_Buffer.o ../../../src/audio/Multi_Buffer.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/_ext/1680311292/Effects_Buffer.o: ../../../src/audio/Effects_Buffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1680311292
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1680311292/Effects_Buffer.o ../../../src/audio/Effects_Buffer.cpp

${OBJECTDIR}/_ext/1386528437/MBC5MemoryRule.o: ../../../src/MBC5MemoryRule.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/MBC5MemoryRule.o ../../../src/MBC5MemoryRule.cpp

${OBJECTDIR}/_ext/1680311292/Gb_Apu_State.o: ../../../src/audio/Gb_Apu_State.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1680311292
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1680311292/Gb_Apu_State.o ../../../src/audio/Gb_Apu_State.cpp

${OBJECTDIR}/_ext/1386528437/SixteenBitRegister.o: ../../../src/SixteenBitRegister.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/SixteenBitRegister.o ../../../src/SixteenBitRegister.cpp

${OBJECTDIR}/_ext/1680311292/Blip_Buffer.o: ../../../src/audio/Blip_Buffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1680311292
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1680311292/Blip_Buffer.o ../../../src/audio/Blip_Buffer.cpp

${OBJECTDIR}/_ext/1386528437/EightBitRegister.o: ../../../src/EightBitRegister.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/EightBitRegister.o ../../../src/EightBitRegister.cpp

${OBJECTDIR}/_ext/1386528437/MemoryRule.o: ../../../src/MemoryRule.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/MemoryRule.o ../../../src/MemoryRule.cpp

${OBJECTDIR}/_ext/1386528437/Input.o: ../../../src/Input.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Input.o ../../../src/Input.cpp

${OBJECTDIR}/_ext/1386528437/Processor.o: ../../../src/Processor.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Processor.o ../../../src/Processor.cpp

${OBJECTDIR}/_ext/1386528437/Video.o: ../../../src/Video.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Video.o ../../../src/Video.cpp

${OBJECTDIR}/_ext/1386528437/Memory.o: ../../../src/Memory.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Memory.o ../../../src/Memory.cpp

${OBJECTDIR}/_ext/1386528437/Cartridge.o: ../../../src/Cartridge.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Cartridge.o ../../../src/Cartridge.cpp

${OBJECTDIR}/_ext/1386528437/MBC3MemoryRule.o: ../../../src/MBC3MemoryRule.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/MBC3MemoryRule.o ../../../src/MBC3MemoryRule.cpp

${OBJECTDIR}/_ext/1386528437/RomOnlyMemoryRule.o: ../../../src/RomOnlyMemoryRule.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/RomOnlyMemoryRule.o ../../../src/RomOnlyMemoryRule.cpp

${OBJECTDIR}/_ext/1386528437/CommonMemoryRule.o: ../../../src/CommonMemoryRule.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/CommonMemoryRule.o ../../../src/CommonMemoryRule.cpp

${OBJECTDIR}/_ext/1680311292/Sound_Queue.o: ../../../src/audio/Sound_Queue.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1680311292
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1680311292/Sound_Queue.o ../../../src/audio/Sound_Queue.cpp

${OBJECTDIR}/_ext/1680311292/Gb_Oscs.o: ../../../src/audio/Gb_Oscs.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1680311292
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1680311292/Gb_Oscs.o ../../../src/audio/Gb_Oscs.cpp

${OBJECTDIR}/_ext/1386528437/opcodes.o: ../../../src/opcodes.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/opcodes.o ../../../src/opcodes.cpp

${OBJECTDIR}/_ext/1386528437/opcodes_cb.o: ../../../src/opcodes_cb.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/opcodes_cb.o ../../../src/opcodes_cb.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gearboy

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
