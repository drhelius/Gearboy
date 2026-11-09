//
//  VirtualFat.cpp
//  by Nolan Baker (Q*Bert Reynolds)
//

#include "VirtualFat.h"
#include "log.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <ctime>

#if !defined(_WIN32)
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

static const u32 k_part_start = 2048;
static const u32 k_part_sectors = 0x40000000; // 512GB
static const u32 k_reserved = 32;
static const u32 k_fat_count = 2; // stock GBCOS computes data start using two FATs
static const u32 k_cluster_sectors = 64;
static const u32 k_cluster_bytes = k_cluster_sectors * 512;
static const u32 k_eoc = 0x0FFFFFFF;
static const u32 k_max_chain = 0x20000; // 4GB of clusters
static const int k_lfn_offsets[13] = {1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30};

static void Put16(u8* p, u32 v)
{
    p[0] = v & 0xFF;
    p[1] = (v >> 8) & 0xFF;
}

static void Put32(u8* p, u32 v)
{
    Put16(p, v & 0xFFFF);
    Put16(p + 2, v >> 16);
}

static u32 Get16(const u8* p)
{
    return p[0] | (p[1] << 8);
}

static u32 Get32(const u8* p)
{
    return Get16(p) | (Get16(p + 2) << 16);
}

// FAT date in the high 16 bits, time in the low 16
static u32 FatTime(time_t t)
{
    struct tm tm;
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    if (tm.tm_year < 80)
        return (1 << 5 | 1) << 16; // 1980-01-01
    u32 date = ((tm.tm_year - 80) << 9) | ((tm.tm_mon + 1) << 5) | tm.tm_mday;
    u32 time = (tm.tm_hour << 11) | (tm.tm_min << 5) | (tm.tm_sec / 2);
    return (date << 16) | time;
}

static u8 Checksum(const u8* short_name)
{
    u8 sum = 0;
    for (int i = 0; i < 11; i++)
        sum = (u8)(((sum & 1) << 7) + (sum >> 1) + short_name[i]);
    return sum;
}

static std::vector<u16> Utf16(const std::string& s)
{
    std::vector<u16> out;
    for (size_t i = 0; i < s.size();)
    {
        u8 c = s[i];
        u32 code = c;
        int extra = 0;
        if (c >= 0xF0 && c < 0xF8) { code = c & 0x07; extra = 3; }
        else if (c >= 0xE0) { code = c & 0x0F; extra = 2; }
        else if (c >= 0xC0) { code = c & 0x1F; extra = 1; }
        if (extra && i + extra >= s.size())
        {
            extra = 0; // cut off, keep the byte as Latin-1
            code = c;
        }
        for (int k = 1; k <= extra; k++)
            code = (code << 6) | (s[i + k] & 0x3F);
        i += extra + 1;
        if (code >= 0x10000)
        {
            code -= 0x10000;
            out.push_back((u16)(0xD800 | (code >> 10)));
            out.push_back((u16)(0xDC00 | (code & 0x3FF)));
        }
        else
            out.push_back((u16)code);
    }
    return out;
}

static std::string Utf8(const u16* s, int count)
{
    std::string out;
    for (int i = 0; i < count && s[i]; i++)
    {
        u32 code = s[i];
        if (code >= 0xD800 && code < 0xDC00 && i + 1 < count)
            code = 0x10000 + ((code - 0xD800) << 10) + (s[++i] - 0xDC00);
        if (code < 0x80)
            out += (char)code;
        else if (code < 0x800)
        {
            out += (char)(0xC0 | (code >> 6));
            out += (char)(0x80 | (code & 0x3F));
        }
        else if (code < 0x10000)
        {
            out += (char)(0xE0 | (code >> 12));
            out += (char)(0x80 | ((code >> 6) & 0x3F));
            out += (char)(0x80 | (code & 0x3F));
        }
        else
        {
            out += (char)(0xF0 | (code >> 18));
            out += (char)(0x80 | ((code >> 12) & 0x3F));
            out += (char)(0x80 | ((code >> 6) & 0x3F));
            out += (char)(0x80 | (code & 0x3F));
        }
    }
    return out;
}

static bool ShortChar(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || strchr("!#$%&'()-@^_`{}~", c);
}

static char Upper(char c)
{
    return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;
}

static bool ItemLess(const std::string& a, const std::string& b)
{
    for (size_t i = 0; i < a.size() && i < b.size(); i++)
    {
        char x = Upper(a[i]), y = Upper(b[i]);
        if (x != y)
            return x < y;
    }
    return a.size() < b.size();
}

static bool ValidName(const std::string& name)
{
    return !name.empty() && name != "." && name != ".." && name.find('/') == std::string::npos &&
        name.find('\\') == std::string::npos;
}

VirtualFat::VirtualFat()
{
    m_open = false;
    Close();
}

bool VirtualFat::IsOpen() const
{
    return m_open;
}

void VirtualFat::Close()
{
    m_open = false;
    m_nodes.clear();
    m_owners.clear();
    m_fat.clear();
    m_sectors.clear();
    m_dirty.clear();
}

bool VirtualFat::Open(const char* root)
{
    Close();
#if defined(_WIN32)
    Log("VirtualFat: host folders aren't supported on Windows, use GEARBOY_SD_IMAGE");
    (void)root;
    return false;
#else
    struct stat st;
    if (stat(root, &st) != 0 || !S_ISDIR(st.st_mode))
    {
        Log("VirtualFat: %s isn't a folder", root);
        return false;
    }

    m_fat_sectors = ((k_part_sectors - k_reserved) / k_cluster_sectors + 2) * 4 / 512 + 1;
    m_clusters = (k_part_sectors - k_reserved - k_fat_count * m_fat_sectors) / k_cluster_sectors;
    m_data_start = k_part_start + k_reserved + k_fat_count * m_fat_sectors;
    m_gb_clusters = m_clusters / 2;
    m_next_cluster = 2;

    std::string path(root);
    while (path.size() > 1 && path[path.size() - 1] == '/')
        path.erase(path.size() - 1);
    AddNode(path, true, 0, FatTime(st.st_mtime), -1);
    Allocate(0, 1); // root folder starts at cluster 2
    m_open = true;
    List(0);
    Log("VirtualFat: SD card is %s", path.c_str());
    return true;
#endif
}

int VirtualFat::AddNode(const std::string& path, bool dir, u32 size, u32 time, int parent)
{
    Node node;
    node.path = path;
    node.dir = dir;
    node.listed = false;
    node.size = size;
    node.first = 0;
    node.time = time;
    memset(node.short_name, ' ', 11);
    node.parent = parent;
    m_nodes.push_back(node);
    int index = (int)m_nodes.size() - 1;
    if (parent >= 0)
        m_nodes[parent].children.push_back(index);
    return index;
}

bool VirtualFat::Allocate(int node, u32 count)
{
    if (m_next_cluster + count > m_gb_clusters)
        return false;
    Run run = {m_next_cluster, count};
    Owner owner = {count, node, (int)m_nodes[node].runs.size()};
    m_owners[run.start] = owner;
    m_nodes[node].runs.push_back(run);
    if (!m_nodes[node].first)
        m_nodes[node].first = run.start;
    m_next_cluster += count;
    return true;
}

void VirtualFat::MakeShortName(const std::string& name, const std::set<std::string>& used, u8* short_name, u8* case_flags, bool* needs_long_name)
{
    size_t dot = name.rfind('.');
    std::string base = dot == std::string::npos ? name : name.substr(0, dot);
    std::string ext = dot == std::string::npos ? "" : name.substr(dot + 1);

    // a plain 8.3 name, all upper or all lower case in each part, doesn't need a long name
    bool plain = !base.empty() && base.size() <= 8 && ext.size() <= 3 && base.find('.') == std::string::npos;
    *case_flags = 0;
    for (int part = 0; part < 2 && plain; part++)
    {
        const std::string& s = part ? ext : base;
        bool lower = false, upper = false;
        for (size_t i = 0; i < s.size(); i++)
        {
            plain = plain && ShortChar(Upper(s[i]));
            lower = lower || (s[i] >= 'a' && s[i] <= 'z');
            upper = upper || (s[i] >= 'A' && s[i] <= 'Z');
        }
        plain = plain && !(lower && upper);
        if (lower)
            *case_flags |= part ? 0x10 : 0x08;
    }
    memset(short_name, ' ', 11);
    if (plain)
    {
        for (size_t i = 0; i < base.size() && i < 8; i++)
            short_name[i] = Upper(base[i]);
        for (size_t i = 0; i < ext.size() && i < 3; i++)
            short_name[8 + i] = Upper(ext[i]);
        if (!used.count(std::string((char*)short_name, 11)))
        {
            *needs_long_name = false;
            return;
        }
    }

    // NAME~N.EXT
    *needs_long_name = true;
    *case_flags = 0;
    std::string b, e;
    for (size_t i = 0; i < base.size(); i++)
        if (base[i] != ' ' && base[i] != '.')
            b += ShortChar(Upper(base[i])) ? Upper(base[i]) : '_';
    for (size_t i = 0; i < ext.size() && e.size() < 3; i++)
        if (ext[i] != ' ')
            e += ShortChar(Upper(ext[i])) ? Upper(ext[i]) : '_';
    if (b.empty())
        b = "_";
    for (u32 n = 1; n < 1000000; n++)
    {
        char tail[10];
        snprintf(tail, sizeof(tail), "~%u", n);
        std::string candidate = b.substr(0, 8 - strlen(tail)) + tail;
        memset(short_name, ' ', 11);
        memcpy(short_name, candidate.c_str(), candidate.size());
        memcpy(short_name + 8, e.c_str(), e.size());
        if (!used.count(std::string((char*)short_name, 11)))
            return;
    }
}

void VirtualFat::AddEntries(std::vector<u8>& entries, const std::string& name, const u8* short_name, u8 case_flags, bool long_name, u8 attr, u32 first, u32 size, u32 time)
{
    std::vector<u16> chars = Utf16(name);
    if (long_name && chars.size() <= 255)
    {
        u8 sum = Checksum(short_name);
        int count = (int)(chars.size() + 12) / 13;
        for (int ord = count; ord >= 1; ord--)
        {
            u8 e[32];
            memset(e, 0, 32);
            e[0] = (u8)(ord | (ord == count ? 0x40 : 0));
            e[11] = 0x0F;
            e[13] = sum;
            for (int k = 0; k < 13; k++)
            {
                size_t i = (ord - 1) * 13 + k;
                Put16(e + k_lfn_offsets[k], i < chars.size() ? chars[i] : (i == chars.size() ? 0x0000 : 0xFFFF));
            }
            entries.insert(entries.end(), e, e + 32);
        }
    }
    u8 e[32];
    memset(e, 0, 32);
    memcpy(e, short_name, 11);
    e[11] = attr;
    e[12] = case_flags;
    Put16(e + 14, time & 0xFFFF);
    Put16(e + 16, time >> 16);
    Put16(e + 18, time >> 16);
    Put16(e + 20, first >> 16);
    Put16(e + 22, time & 0xFFFF);
    Put16(e + 24, time >> 16);
    Put16(e + 26, first & 0xFFFF);
    Put32(e + 28, size);
    entries.insert(entries.end(), e, e + 32);
}

// gives the folder's children clusters and makes its entries, the first time it's read
void VirtualFat::List(int index)
{
    m_nodes[index].listed = true;
#if !defined(_WIN32)
    std::string path = m_nodes[index].path;
    std::vector<Item> items;
    DIR* dir = opendir(path.c_str());
    if (dir)
    {
        struct dirent* e;
        while ((e = readdir(dir)) != NULL)
        {
            Item item;
            item.name = e->d_name;
            if (item.name == "." || item.name == "..")
                continue;
            struct stat st;
            if (stat((path + "/" + item.name).c_str(), &st) != 0)
                continue;
            item.time = FatTime(st.st_mtime);
            item.dir = S_ISDIR(st.st_mode);
            if (!item.dir && (!S_ISREG(st.st_mode) || st.st_size >= 0xFFFFFFFFLL))
                continue;
            item.size = item.dir ? 0 : (u32)st.st_size;
            items.push_back(item);
        }
        closedir(dir);
    }
    else
        Log("VirtualFat: can't list %s", path.c_str());

    std::sort(items.begin(), items.end(), [](const Item& a, const Item& b) { return ItemLess(a.name, b.name); });

    std::vector<u8> entries;
    std::set<std::string> used;
    if (index)
    {
        u8 dot[11], dotdot[11];
        memset(dot, ' ', 11);
        memset(dotdot, ' ', 11);
        dot[0] = dotdot[0] = dotdot[1] = '.';
        int parent = m_nodes[index].parent;
        AddEntries(entries, ".", dot, 0, false, 0x10, m_nodes[index].first, 0, m_nodes[index].time);
        AddEntries(entries, "..", dotdot, 0, false, 0x10, parent ? m_nodes[parent].first : 0, 0, m_nodes[index].time);
    }
    for (size_t i = 0; i < items.size(); i++)
    {
        const Item& item = items[i];
        u32 count = item.dir ? 1 : (item.size + k_cluster_bytes - 1) / k_cluster_bytes;
        if (m_next_cluster + count > m_gb_clusters)
        {
            Log("VirtualFat: out of clusters listing %s", path.c_str());
            break;
        }
        u8 short_name[11], case_flags;
        bool long_name;
        MakeShortName(item.name, used, short_name, &case_flags, &long_name);
        used.insert(std::string((char*)short_name, 11));
        int child = AddNode(path + "/" + item.name, item.dir, item.size, item.time, index);
        memcpy(m_nodes[child].short_name, short_name, 11);
        if (count)
            Allocate(child, count);
        u8 attr = item.dir ? 0x10 : 0x20;
        if (item.name[0] == '.')
            attr |= 0x02; // hidden
        AddEntries(entries, item.name, short_name, case_flags, long_name, attr, m_nodes[child].first, item.size, item.time);
    }

    // enough clusters for the entries and an empty one that ends the folder
    u32 needed = (u32)(entries.size() + 32 + k_cluster_bytes - 1) / k_cluster_bytes;
    u32 have = 0;
    for (size_t i = 0; i < m_nodes[index].runs.size(); i++)
        have += m_nodes[index].runs[i].count;
    if (needed > have && !Allocate(index, needed - have))
        needed = have;
    entries.resize(needed * k_cluster_bytes, 0);
    m_nodes[index].entries.swap(entries);
#endif
}

bool VirtualFat::FindOwner(u32 cluster, int* node, u32* index)
{
    std::map<u32, Owner>::iterator it = m_owners.upper_bound(cluster);
    if (it == m_owners.begin())
        return false;
    --it;
    if (cluster >= it->first + it->second.count)
        return false;
    *node = it->second.node;
    *index = cluster - it->first;
    const std::vector<Run>& runs = m_nodes[*node].runs;
    for (int r = 0; r < it->second.run; r++)
        *index += runs[r].count;
    return true;
}

// FAT entry for the clusters given out for host files and folders, 0 (free) for the rest
u32 VirtualFat::DerivedFat(u32 cluster)
{
    if (cluster < 2)
        return cluster ? k_eoc : 0x0FFFFFF8;
    std::map<u32, Owner>::iterator it = m_owners.upper_bound(cluster);
    if (it == m_owners.begin())
        return 0;
    --it;
    const Owner& owner = it->second;
    if (cluster >= it->first + owner.count)
        return 0;
    if (cluster + 1 < it->first + owner.count)
        return cluster + 1;
    const std::vector<Run>& runs = m_nodes[owner.node].runs;
    if (owner.run + 1 < (int)runs.size())
        return runs[owner.run + 1].start;
    return k_eoc;
}

u32 VirtualFat::FatEntry(u32 cluster)
{
    std::map<u32, u32>::iterator it = m_fat.find(cluster);
    return it != m_fat.end() ? it->second : DerivedFat(cluster);
}

void VirtualFat::ReadData(u32 cluster, u32 sector, u8* data)
{
    int node;
    u32 index;
    if (!FindOwner(cluster, &node, &index))
        return;
    if (m_nodes[node].dir)
    {
        if (!m_nodes[node].listed)
            List(node);
        size_t offset = ((size_t)index * k_cluster_sectors + sector) * 512;
        if (offset + 512 <= m_nodes[node].entries.size())
            memcpy(data, &m_nodes[node].entries[offset], 512);
        return;
    }
    FILE* file = fopen(m_nodes[node].path.c_str(), "rb");
    if (!file)
        return;
#if defined(_WIN32)
    _fseeki64(file, ((long long)index * k_cluster_sectors + sector) * 512, SEEK_SET);
#else
    fseeko(file, ((off_t)index * k_cluster_sectors + sector) * 512, SEEK_SET);
#endif
    if (fread(data, 1, 512, file) < 512)
        clearerr(file);
    fclose(file);
}

void VirtualFat::ReadSector(u32 lba, u8* data)
{
    memset(data, 0, 512);
    if (!m_open)
        return;

    std::map<u32, std::vector<u8> >::iterator written = m_sectors.find(lba);
    if (written != m_sectors.end())
    {
        memcpy(data, &written->second[0], 512);
        return;
    }

    if (lba == 0) // MBR, one FAT32 (LBA) partition
    {
        data[446 + 4] = 0x0C;
        Put32(data + 446 + 8, k_part_start);
        Put32(data + 446 + 12, k_part_sectors);
        data[510] = 0x55;
        data[511] = 0xAA;
        return;
    }
    if (lba < k_part_start)
        return;

    u32 s = lba - k_part_start;
    if (s == 0 || s == 6) // boot sector and its backup
    {
        const u8 jump[3] = {0xEB, 0x58, 0x90};
        memcpy(data, jump, 3);
        memcpy(data + 3, "MSWIN4.1", 8);
        Put16(data + 11, 512);
        data[13] = k_cluster_sectors;
        Put16(data + 14, k_reserved);
        data[16] = k_fat_count;
        data[21] = 0xF8;
        Put16(data + 24, 63);
        Put16(data + 26, 255);
        Put32(data + 28, k_part_start);
        Put32(data + 32, k_part_sectors);
        Put32(data + 36, m_fat_sectors);
        Put32(data + 44, 2); // root folder
        Put16(data + 48, 1); // FSInfo
        Put16(data + 50, 6); // backup boot sector
        data[64] = 0x80;
        data[66] = 0x29;
        Put32(data + 67, 0x47424F59);
        memcpy(data + 71, "GEARBOY    ", 11);
        memcpy(data + 82, "FAT32   ", 8);
        data[510] = 0x55;
        data[511] = 0xAA;
        return;
    }
    if (s == 1 || s == 7) // FSInfo, the Game Boy allocates from the middle of the disk
    {
        Put32(data, 0x41615252);
        Put32(data + 484, 0x61417272);
        Put32(data + 488, 0xFFFFFFFF);
        Put32(data + 492, m_gb_clusters);
        Put32(data + 508, 0xAA550000);
        return;
    }
    if (s < k_reserved)
        return;

    s -= k_reserved;
    if (s < k_fat_count * m_fat_sectors)
    {
        s %= m_fat_sectors; // both copies describe the same virtual chains
        for (u32 i = 0; i < 128; i++)
        {
            u32 cluster = s * 128 + i;
            if (cluster < m_clusters + 2)
                Put32(data + i * 4, FatEntry(cluster));
        }
        return;
    }

    s -= k_fat_count * m_fat_sectors;
    if (s < m_clusters * k_cluster_sectors)
        ReadData(2 + s / k_cluster_sectors, s % k_cluster_sectors, data);
}

void VirtualFat::WriteSector(u32 lba, const u8* data)
{
    if (!m_open)
        return;

    u32 fat_start = k_part_start + k_reserved;
    if (lba >= fat_start && lba < fat_start + k_fat_count * m_fat_sectors)
    {
        for (u32 i = 0; i < 128; i++)
        {
            u32 cluster = ((lba - fat_start) % m_fat_sectors) * 128 + i;
            if (cluster < 2 || cluster >= m_clusters + 2)
                continue;
            u32 value = Get32(data + i * 4) & 0x0FFFFFFF;
            if (value == DerivedFat(cluster))
                m_fat.erase(cluster);
            else
                m_fat[cluster] = value;
        }
        return;
    }

    m_sectors[lba].assign(data, data + 512);
    if (lba >= m_data_start)
    {
        u32 cluster = 2 + (lba - m_data_start) / k_cluster_sectors;
        int dir = DirOf(cluster);
        if (dir >= 0)
            Sync(dir);
        else
            m_dirty.insert(cluster); // written to the host when its folder entry is
    }
}

// follows the FAT from first, whole clusters, or size bytes (fails if the chain is shorter)
bool VirtualFat::ReadChain(u32 first, u32 size, bool whole_clusters, std::vector<u8>& out)
{
    out.clear();
    u32 cluster = first;
    for (u32 steps = 0; steps < k_max_chain; steps++)
    {
        if (!whole_clusters && out.size() >= size)
            break;
        if (cluster < 2 || cluster >= m_clusters + 2)
            break;
        for (u32 s = 0; s < k_cluster_sectors; s++)
        {
            u8 sector[512];
            ReadSector(m_data_start + (cluster - 2) * k_cluster_sectors + s, sector);
            out.insert(out.end(), sector, sector + 512);
        }
        cluster = FatEntry(cluster);
    }
    if (whole_clusters)
        return true;
    if (out.size() < size)
        return false;
    out.resize(size);
    return true;
}

// true if any of the file's clusters were written since it was last written to the host, forgets them
bool VirtualFat::TakeDirty(u32 first, u32 size)
{
    bool dirty = false;
    u32 cluster = first;
    for (u32 left = size; left && !m_dirty.empty() && cluster >= 2 && cluster < m_clusters + 2; cluster = FatEntry(cluster))
    {
        dirty |= m_dirty.erase(cluster) != 0;
        left = left > k_cluster_sectors * 512 ? left - k_cluster_sectors * 512 : 0;
    }
    return dirty;
}

// folder whose chain has cluster, -1 if none
int VirtualFat::DirOf(u32 cluster)
{
    int node;
    u32 index;
    if (FindOwner(cluster, &node, &index))
        return m_nodes[node].dir ? node : -1;
    for (size_t i = 0; i < m_nodes.size(); i++) // folders the Game Boy made or grew
    {
        if (!m_nodes[i].dir || !m_nodes[i].listed)
            continue;
        u32 c = m_nodes[i].first;
        for (u32 steps = 0; steps < 4096 && c >= 2 && c < m_clusters + 2; steps++)
        {
            if (c == cluster)
                return (int)i;
            c = FatEntry(c);
        }
    }
    return -1;
}

// writes new and changed files (and new folders) in a folder the Game Boy wrote to the host
void VirtualFat::Sync(int index)
{
    std::vector<u8> bytes;
    ReadChain(m_nodes[index].first, 0, true, bytes);
    u16 lfn[20 * 13];
    bool lfn_ok = false;
    u8 lfn_sum = 0;
    int lfn_count = 0;
    for (size_t pos = 0; pos + 32 <= bytes.size(); pos += 32)
    {
        const u8* e = &bytes[pos];
        if (e[0] == 0)
            break;
        if (e[0] == 0xE5)
        {
            lfn_ok = false;
            continue;
        }
        u8 attr = e[11];
        if (attr == 0x0F)
        {
            int ord = e[0] & 0x3F;
            if (e[0] & 0x40)
            {
                lfn_ok = ord >= 1 && ord <= 20;
                lfn_sum = e[13];
                lfn_count = ord * 13;
            }
            if (!lfn_ok || ord < 1 || ord > 20 || e[13] != lfn_sum)
            {
                lfn_ok = false;
                continue;
            }
            for (int k = 0; k < 13; k++)
                lfn[(ord - 1) * 13 + k] = (u16)Get16(e + k_lfn_offsets[k]);
            continue;
        }
        bool has_lfn = lfn_ok && Checksum(e) == lfn_sum;
        lfn_ok = false;
        if ((attr & 0x08) || e[0] == '.')
            continue;

        std::string name;
        if (has_lfn)
            name = Utf8(lfn, lfn_count);
        else
        {
            for (int i = 0; i < 8 && e[i] != ' '; i++)
                name += (char)((e[12] & 0x08) && e[i] >= 'A' && e[i] <= 'Z' ? e[i] + 32 : (i == 0 && e[i] == 0x05 ? 0xE5 : e[i]));
            if (e[8] != ' ')
                name += '.';
            for (int i = 8; i < 11 && e[i] != ' '; i++)
                name += (char)((e[12] & 0x10) && e[i] >= 'A' && e[i] <= 'Z' ? e[i] + 32 : e[i]);
        }
        if (!ValidName(name))
            continue;

        bool dir = (attr & 0x10) != 0;
        u32 first = (Get16(e + 20) << 16) | Get16(e + 26);
        u32 size = Get32(e + 28);
        int child = -1;
        const std::vector<int>& children = m_nodes[index].children;
        for (size_t i = 0; i < children.size() && child < 0; i++)
            if (!memcmp(m_nodes[children[i]].short_name, e, 11))
                child = children[i];

        if (child < 0)
        {
            std::string path = m_nodes[index].path + "/" + name;
            child = AddNode(path, dir, 0, 0, index);
            memcpy(m_nodes[child].short_name, e, 11);
            if (dir)
            {
#if !defined(_WIN32)
                if (mkdir(path.c_str(), 0777) == 0)
                    Log("VirtualFat: made %s", path.c_str());
#endif
                m_nodes[child].listed = true; // its entries are the Game Boy's
            }
            else if (WriteHostFile(path, first, size))
            {
                m_nodes[child].first = first;
                m_nodes[child].size = size;
            }
        }
        if (dir)
        {
            if (!m_nodes[child].runs.size())
                m_nodes[child].first = first;
            continue;
        }
        Node& node = m_nodes[child];
        if (first == node.first && size == node.size && !TakeDirty(first, size))
            continue; // same chain and size, and none of it rewritten
        if (!size && node.size)
            continue; // FatFs empties a file before writing it again, wait for the new size
        if (WriteHostFile(node.path, first, size))
        {
            m_nodes[child].first = first;
            m_nodes[child].size = size;
        }
    }
}

bool VirtualFat::WriteHostFile(const std::string& path, u32 first, u32 size)
{
    std::vector<u8> content;
    if (size && !ReadChain(first, size, false, content))
        return false; // not all of it is written yet
    std::string temp = path + ".gearboy-tmp";
    FILE* file = fopen(temp.c_str(), "wb");
    if (!file)
    {
        Log("VirtualFat: can't write %s", path.c_str());
        return false;
    }
    bool ok = content.empty() || fwrite(&content[0], 1, content.size(), file) == content.size();
    ok = fclose(file) == 0 && ok;
    if (!ok || rename(temp.c_str(), path.c_str()) != 0)
    {
        remove(temp.c_str());
        Log("VirtualFat: can't write %s", path.c_str());
        return false;
    }
    Log("VirtualFat: wrote %s (%u bytes)", path.c_str(), size);
    return true;
}
