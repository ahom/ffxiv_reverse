#ifndef XIV_DAT_INDEX_H
#define XIV_DAT_INDEX_H

#include <xiv/dat/SqPack.h>

#include <unordered_map>

#include <boost/filesystem.hpp>

namespace xiv
{
namespace dat
{

struct IndexBlockRecord;

class Index : public SqPack
{
public:
    // Full path to the index file
    Index(const boost::filesystem::path& i_path);
    virtual ~Index();

    // An entry in the hash table, representing a file in a given dat
    struct HashTableEntry
    {
        uint32_t dat_nb;
        uint32_t dir_hash;
        uint32_t filename_hash;
        uint32_t dat_offset;
    };

    // HashTable has dir hashes -> filename hashes -> HashTableEntry
    typedef std::unordered_map<uint32_t, HashTableEntry> DirHashTable;
    typedef std::unordered_map<uint32_t, DirHashTable> HashTable;

    // Get the number of dat files the index is linked to
    uint32_t get_dat_count() const;

    bool check_file_existence(uint32_t dir_hash, uint32_t filename_hash) const;
    bool check_dir_existence(uint32_t dir_hash) const;

    // Returns the whole HashTable
    const HashTable& get_hash_table() const;
    // Returns the hash table for a specific dir
    const DirHashTable& get_dir_hash_table(uint32_t dir_hash) const;
    // Returns the HashTableEntry for a given file given its hashes
    const HashTableEntry& get_hash_table_entry(uint32_t dir_hash, uint32_t filename_hash) const;

protected:
    // Checks that the block is valid with regards to its hash
    void is_index_block_valid(const IndexBlockRecord& i_index_block_record);

    uint32_t _dat_count;
    HashTable _hash_table;
};

}
}

#endif // XIV_DAT_INDEX_H
