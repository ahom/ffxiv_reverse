#ifndef XIV_TEX_TEXTURE_H
#define XIV_TEX_TEXTURE_H

#include <vector>
#include <iostream>
#include <memory>
#include <string>

#include <boost/filesystem.hpp>

#include <xiv/utils/bparse.h>

XIV_ENUM((xiv)(tex), TextureType, uint16_t,
         XIV_VALUE(RGB5A1, 0x1441)
         XIV_VALUE(RGB4A4, 0x1440)
         XIV_VALUE(RGB8A8, 0x1450)
         XIV_VALUE(DXT1,   0x3420)
         XIV_VALUE(DXT5,   0x3431)
         XIV_VALUE(RGBAF,  0x2460));

namespace xiv
{
namespace dat
{
class File;
class GameData;
}
namespace tex
{

// Texture from the dats
class Texture
{
public:
    Texture(dat::GameData& i_game_data, const std::string& i_name);
    Texture(std::unique_ptr<dat::File> i_file);
    ~Texture();

    const std::string& get_name() const;

    uint16_t get_width() const;
    uint16_t get_height() const;
    TextureType get_type() const;

    const std::vector<std::vector<char>>& get_mipmap_data() const;

    void export_as_json(const boost::filesystem::path& i_output_path) const;

protected:
    std::string _name;

    uint16_t _width;
    uint16_t _height;
    TextureType _type;

    // Mipmap data stored by level
    std::vector<std::vector<char>> _mipmap_data;

private:
    void initialize(std::unique_ptr<dat::File> i_file);
};

}
}

#endif // XIV_TEX_TEXTURE_H
