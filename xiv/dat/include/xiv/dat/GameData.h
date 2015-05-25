#ifndef XIV_DAT_GAMEDATA_H
#define XIV_DAT_GAMEDATA_H

#include <memory>
#include <unordered_map>
#include <mutex>

#include <boost/filesystem.hpp>

namespace xiv
{
namespace dat
{

class Cat;
class File;

// Interface to all the datfiles - Main entry point
// All the paths to files/dirs inside the dats are case-insensitive
class GameData
{
public:
    // This should be the path in which the .index/.datX files are located
    GameData(const boost::filesystem::path& i_path);
    ~GameData();

    // Returns all the scanned category number available in the path
    const std::vector<uint32_t>& get_cat_nbs() const;

    // Return a specific category by its number (see get_cat_nbs() for loops)
    const Cat& get_category(uint32_t i_cat_nb);
    // Return a specific category by it's name (e.g.: "exd"/"game_script"/ etc...)
    const Cat& get_category(const std::string& i_cat_name);

    // Retrieve a file from the dats given its filename
    std::unique_ptr<File> get_file(const std::string& i_path);

    // Checks that a file exists
    bool check_file_existence(const std::string& i_path);

    // Checks that a dir exists, there must be a trailing / in the path
    // Note that it won't work for dirs that don't contain any file
    // e.g.:  - "ui/icon/" will return False
    //        - "ui/icon/000000/" will return True
    bool check_dir_existence(const std::string& i_path);

protected:
    // Return a specific category given a path (calls const Cat& get_category(const std::string& i_cat_name))
    const Cat& get_category_from_path(const std::string& i_path);

    // From a full path, returns the dir_hash and the filename_hash
    void get_hashes(const std::string& i_path, uint32_t& o_dir_hash, uint32_t& o_filename_hash) const;

    // Lazy instantiation of category
    void create_category(uint32_t i_cat_nb);

    // Path given to constructor, pointing to the folder with the .index/.datX files
    const boost::filesystem::path _path;

    // Stored categories, indexed by their number, categories are instantiated and parsed individually when they are needed
    std::unordered_map<uint32_t, std::unique_ptr<Cat>> _cats;

    // List of all the categories numbers, is equal to _cats.keys()
    std::vector<uint32_t> _cat_nbs;

    // Mutexes needed to not instantiate the categories at the same time in two different threads, indexed by category number
    std::unordered_map<uint32_t, std::unique_ptr<std::mutex>> _cat_creation_mutexes;
};

}
}

#endif // XIV_DAT_GAMEDATA_H
