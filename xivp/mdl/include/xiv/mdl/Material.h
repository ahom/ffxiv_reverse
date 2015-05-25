#ifndef XIV_MDL_MATERIAL_H
#define XIV_MDL_MATERIAL_H

#include <vector>
#include <string>
#include <ostream>

#include <boost/filesystem.hpp>

namespace xiv
{
namespace dat
{
class GameData;
class File;
}
namespace tex
{
class Texture;
}
namespace mdl
{

class Material
{
public:
    Material(dat::GameData& i_game_data, const std::string& i_name);
    Material(dat::GameData& i_game_data, const dat::File& i_file);
    ~Material();

    const std::string& get_name() const;

    const std::vector<tex::Texture>& get_texs() const;

    void export_as_json(const boost::filesystem::path& i_output_path) const;

protected:
    std::string _name;
    std::vector<tex::Texture> _texs;
    std::vector<std::string> _maps;
    std::vector<std::string> _color_sets;

private:
    void initialize(dat::GameData& i_game_data, const dat::File& i_file);
};

}
}

#endif // XIV_MDL_MATERIAL_H

