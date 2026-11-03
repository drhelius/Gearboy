//
//  VirtualFat.h
//  by Nolan Baker (Q*Bert Reynolds)
//
//  A FAT32 disk built on the fly from a host folder, for the EverDrive X SD card.
//  Folders are listed from the host the first time the Game Boy reads them, files are read
//  from the host when their clusters are read. Sectors the Game Boy writes are kept in memory,
//  and when it writes a folder's entries, new and changed files (and new folders) are written
//  to the host. Host files are never deleted or truncated to nothing.
//
//  Layout: MBR, then one FAT32 partition at sector 2048 with 32KB clusters and one FAT.
//  Clusters for host files and folders are given out from cluster 3 up, FSInfo points the
//  Game Boy's allocations at the middle of the disk so they never meet.
//

#ifndef VIRTUAL_FAT_H
#define VIRTUAL_FAT_H

#include "definitions.h"
#include <map>
#include <set>
#include <string>
#include <vector>

class VirtualFat
{
public:
    VirtualFat();
    bool Open(const char* root);
    void Close();
    bool IsOpen() const;
    void ReadSector(u32 lba, u8* data);
    void WriteSector(u32 lba, const u8* data);

private:
    struct Run
    {
        u32 start;
        u32 count;
    };

    struct Owner
    {
        u32 count;
        int node;
        int run;
    };

    struct Node
    {
        std::string path;
        bool dir;
        bool listed;
        u32 size;
        u32 first;
        u32 time;
        u8 short_name[11];
        int parent;
        std::vector<Run> runs; // clusters given out for it, in chain order
        std::vector<u8> entries; // folders listed from the host
        std::vector<int> children;
    };

    struct Item
    {
        std::string name;
        bool dir;
        u32 size;
        u32 time;
    };

    int AddNode(const std::string& path, bool dir, u32 size, u32 time, int parent);
    bool Allocate(int node, u32 count);
    void List(int node);
    void MakeShortName(const std::string& name, const std::set<std::string>& used, u8* short_name, u8* case_flags, bool* needs_long_name);
    void AddEntries(std::vector<u8>& entries, const std::string& name, const u8* short_name, u8 case_flags, bool long_name, u8 attr, u32 first, u32 size, u32 time);
    bool FindOwner(u32 cluster, int* node, u32* index);
    u32 DerivedFat(u32 cluster);
    u32 FatEntry(u32 cluster);
    void ReadData(u32 cluster, u32 sector, u8* data);
    bool ReadChain(u32 first, u32 size, bool whole_clusters, std::vector<u8>& out);
    bool TakeDirty(u32 first, u32 size);
    int DirOf(u32 cluster);
    void Sync(int node);
    bool WriteHostFile(const std::string& path, u32 first, u32 size);

private:
    bool m_open;
    u32 m_clusters;
    u32 m_fat_sectors;
    u32 m_data_start;
    u32 m_next_cluster;
    u32 m_gb_clusters;
    std::vector<Node> m_nodes;
    std::map<u32, Owner> m_owners; // first cluster of each run
    std::map<u32, u32> m_fat; // FAT entries the Game Boy changed
    std::map<u32, std::vector<u8> > m_sectors; // sectors the Game Boy wrote
    std::set<u32> m_dirty; // file clusters the Game Boy wrote, not written to the host yet
};

#endif // VIRTUAL_FAT_H
