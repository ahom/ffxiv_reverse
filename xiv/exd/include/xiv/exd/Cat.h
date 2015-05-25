#ifndef XIV_EXD_CAT_H
#define XIV_EXD_CAT_H

#include <string>
#include <memory>
#include <unordered_map>

#include <boost/filesystem.hpp>

#include <xiv/utils/bparse.h>

// Language in the exd files - note: chs/chinese is present in the languages array but not in the data files
XIV_ENUM((xiv)(exd), Language, uint16_t,
         XIV_VALUE(none, 0)
         XIV_VALUE(ja,   1)
         XIV_VALUE(en,   2)
         XIV_VALUE(de,   3)
         XIV_VALUE(fr,   4)
         XIV_VALUE(chs,  5));

namespace xiv
{
namespace dat
{
class GameData;
}
namespace exd
{

class Exh;
class Exd;

// A category repesent a several data sheets in the dats all under the same category
class Cat
{
public:
    // i_name: name of the category
    // i_game_data: used to fetch the files needed
    Cat(dat::GameData& i_game_data, const std::string& i_name);
    ~Cat();

    // Returns the name of the category
    const std::string& get_name() const;

    // Returns the header
    const Exh& get_header() const;

    // Returns data for a specific language
    const Exd& get_data_ln(Language i_language = Language::none) const;

    // Export in csv in base flder i_ouput_path
    void export_as_csvs(const boost::filesystem::path& i_output_path) const;

protected:
    const std::string _name;

    // The header file of the category *.exh
    std::unique_ptr<Exh> _header;
    // The data files of the category, indexed by language *.exd
    // Note that if we have multiple files for different range of IDs, they are merged here
    std::unordered_map<Language, std::unique_ptr<Exd>> _data;
};

}
}

#endif // XIV_EXD_CAT_H
