#ifndef XIV_DAT_CAT_H
#define XIV_DAT_CAT_H

#include <memory>
#include <vector>

#include <boost/filesystem.hpp>

namespace xiv
{
namespace dat
{

class Index;
class Dat;
class File;

// A category represents an .index and its associated .datX
class Cat
{
public:
    // i_base_path: Path to the folder containingthe datfiles
    // i_cat_nb: The number of the category
    // i_name: The name of the category, empty if not known
    Cat(const boost::filesystem::path& i_base_path, uint32_t i_cat_nb, const std::string& i_name);
    ~Cat();

    // Returns .index of the category
    const Index& get_index() const;

    // Retrieve a file from the category given its hashes
    std::unique_ptr<File> get_file(uint32_t dir_hash, uint32_t filename_hash) const;


    bool check_file_existence(uint32_t dir_hash, uint32_t filename_hash) const;
    bool check_dir_existence(uint32_t dir_hash) const;


    // Returns thename of the category
    const std::string& get_name() const;

    // Returns the number of the category
    uint32_t get_nb() const;

protected:
    const std::string _name;
    const uint32_t _nb;

    // The .index
    std::unique_ptr<Index> _index;

    // The .datXs such as dat nb X => _dats[X]
    std::vector<std::unique_ptr<Dat>> _dats;
};

}
}

#endif // XIV_DAT_CAT_H
