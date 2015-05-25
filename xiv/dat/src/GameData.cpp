#include <xiv/dat/GameData.h>

#include <string>
#include <sstream>
#include <algorithm>

#include <boost/assign/list_of.hpp>
#include <boost/bimap.hpp>

#include <zlib.h>

#include <xiv/utils/bparse.h>
#include <xiv/dat/logger.h>
#include <xiv/dat/Cat.h>
#include <xiv/dat/File.h>

namespace
{
// Relation between category number and category name
// These names are taken straight from the exe, it helps resolve dispatching when getting files by path
boost::bimap<std::string, uint32_t> category_map = boost::assign::list_of<boost::bimap<std::string, uint32_t>::relation>
        ("common",          0x00)
        ("bgcommon",        0x01)
        ("bg",              0x02)
        ("cut",             0x03)
        ("chara",           0x04)
        ("shader",          0x05)
        ("ui",              0x06)
        ("sound",           0x07)
        ("vfx",             0x08)
        ("ui_script",       0x09)
        ("exd",             0x0A)
        ("game_script",     0x0B)
        ("music",           0x0C);
}

namespace xiv
{
namespace dat
{

GameData::GameData(const boost::filesystem::path& i_path) try :
    _path(i_path)
{
    XIV_INFO(xiv_dat_logger, "Initializing GameData with path: " << _path);

    // Iterate over the files in i_path
    for (auto it = boost::filesystem::directory_iterator(_path); it != boost::filesystem::directory_iterator(); ++it)
    {
        // Get the filename of the current element
        auto filename = it->path().filename().string();

        // If it contains ".win32.index" this is most likely a hit for a category
        if(filename.find(".win32.index") != std::string::npos)
        {
            // Format of indexes is XX0000.win32.index, so fetch the hex number for category number
            std::istringstream iss(filename.substr(0, 2));
            uint32_t cat_nb;
            iss >> std::hex >> cat_nb;

            // Add to the list of category number
            // creates the empty category in the cats map
            // instantiate the creation mutex for this category
            _cat_nbs.push_back(cat_nb);
            _cats[cat_nb] = std::unique_ptr<Cat>();
            _cat_creation_mutexes[cat_nb] = std::unique_ptr<std::mutex>(new std::mutex());
        }
    }
}
catch(std::exception& e)
{
    // In case of failure here, client is supposed to catch the exception because it is not recoverable on our side
    XIV_FATAL(xiv_dat_logger, "GameData initialization failed: " << e.what());
    throw;
}

GameData::~GameData()
{

}

const std::vector<uint32_t>& GameData::get_cat_nbs() const
{
    return _cat_nbs;
}

std::unique_ptr<File> GameData::get_file(const std::string& i_path)
{
    XIV_INFO(xiv_dat_logger, "Get file: " << i_path);

    // Get the hashes, the category from the path then call the get_file of the category
    uint32_t dir_hash;
    uint32_t filename_hash;
    get_hashes(i_path, dir_hash, filename_hash);

    return get_category_from_path(i_path).get_file(dir_hash, filename_hash);
}

bool GameData::check_file_existence(const std::string& i_path)
{
    uint32_t dir_hash;
    uint32_t filename_hash;
    get_hashes(i_path, dir_hash, filename_hash);

    return get_category_from_path(i_path).check_file_existence(dir_hash, filename_hash);
}

bool GameData::check_dir_existence(const std::string& i_path)
{
    uint32_t dir_hash;
    uint32_t filename_hash;
    get_hashes(i_path, dir_hash, filename_hash);

    return get_category_from_path(i_path).check_dir_existence(dir_hash);
}

const Cat& GameData::get_category(uint32_t i_cat_nb)
{
    // Check that the category number exists
    auto cat_it = _cats.find(i_cat_nb);
    if (cat_it == _cats.end())
    {
        throw std::runtime_error("Category not found: " + std::to_string(i_cat_nb));
    }

    // If it exists and already instantiated return it
    if (cat_it->second)
    {
        return *(cat_it->second);
    }
    else
    {
        // Else create it and return it
        create_category(i_cat_nb);
        return *(_cats[i_cat_nb]);
    }
}

const Cat& GameData::get_category(const std::string& i_cat_name)
{
    // Find the category number from the name
    auto category_map_it = ::category_map.left.find(i_cat_name);
    if (category_map_it == ::category_map.left.end())
    {
        throw std::runtime_error("Category not found: " + i_cat_name);
    }

    // From the category number return the category
    return get_category(category_map_it->second);
}

const Cat& GameData::get_category_from_path(const std::string& i_path)
{
    // Find the first / in the string, paths are in the format CAT_NAME/..../.../../....
    auto first_slash_pos = i_path.find('/');
    if (first_slash_pos == std::string::npos)
    {
        throw std::runtime_error("Path do not have a / char: " + i_path);
    }

    // From the sub string found beforethe first / get the category
    std::string cat_name = i_path.substr(0, first_slash_pos);
    XIV_INFO(xiv_dat_logger, "get_category_from_path: " << i_path << " - " << cat_name);
    return get_category(cat_name);
}

void GameData::get_hashes(const std::string& i_path, uint32_t& o_dir_hash, uint32_t& o_filename_hash) const
{
    // Convert the path to lowercase before getting the hashes
    std::string path_lower;
    path_lower.resize(i_path.size());
    std::transform(i_path.begin(), i_path.end(), path_lower.begin(), ::tolower);

    // Find last / to separate dir from filename
    auto last_slash_pos = path_lower.rfind('/');
    if (last_slash_pos == std::string::npos)
    {
        throw std::runtime_error("Path do not have a / char: " + i_path);
    }

    std::string dir_part = path_lower.substr(0, last_slash_pos);
    std::string filename_part = path_lower.substr(last_slash_pos + 1);

    // Get the crc32 values from zlib, to compensate the final XOR 0xFFFFFFFF that isnot done in the exe we just reXOR
    o_dir_hash = crc32(0, reinterpret_cast<const uint8_t*>(dir_part.data()), dir_part.size()) ^ 0xFFFFFFFF;
    o_filename_hash = crc32(0, reinterpret_cast<const uint8_t*>(filename_part.data()), filename_part.size()) ^ 0xFFFFFFFF;
}

void GameData::create_category(uint32_t i_cat_nb)
{
    // Lock mutex in this scope
    std::lock_guard<std::mutex> lock(*(_cat_creation_mutexes[i_cat_nb]));
    // Maybe after unlocking it has already been created, so check (most likely if it blocked)
    if (!_cats[i_cat_nb])
    {
        // Get the category name if we have it
        std::string cat_name;
        auto category_map_it = ::category_map.right.find(i_cat_nb);
        if (category_map_it != ::category_map.right.end())
        {
            cat_name = category_map_it->second;
        }

        // Actually creates the category
        _cats[i_cat_nb] = std::unique_ptr<Cat>(new Cat(_path, i_cat_nb, cat_name));
    }
}

}
}
