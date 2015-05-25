#include <xiv/exd/Cat.h>

#include <boost/assign/list_of.hpp>

#include <xiv/dat/GameData.h>

#include <xiv/exd/logger.h>
#include <xiv/exd/Exh.h>
#include <xiv/exd/Exd.h>

namespace
{
// Suffix of the filenames given a language
std::map<xiv::exd::Language, std::string> language_map = boost::assign::map_list_of
        (xiv::exd::Language::none, "")
        (xiv::exd::Language::ja, "_ja")
        (xiv::exd::Language::en, "_en")
        (xiv::exd::Language::de, "_de")
        (xiv::exd::Language::fr, "_fr")
        (xiv::exd::Language::chs, "_chs");
}

namespace xiv
{
namespace exd
{

Cat::Cat(dat::GameData& i_game_data, const std::string& i_name) :
    _name(i_name)
{
    XIV_INFO(xiv_exd_logger, "Initializing Cat with name: " << i_name);

    // creates the header .exh
    {
        auto header_file = i_game_data.get_file("exd/" + i_name + ".exh");
        _header = std::unique_ptr<Exh>(new Exh(*header_file));
    }

    for(auto language: _header->get_languages())
    {
        // chs not yet in data files
        if (language != Language::chs)
        {
            // Get all the files for a given category/language, in case of multiple range of IDs in separate files (like Quest)
            std::vector<std::unique_ptr<dat::File>> files;
            for(auto& exd_def: _header->get_exd_defs())
            {
                files.emplace_back(i_game_data.get_file("exd/" + i_name + "_" + std::to_string(exd_def.start_id) + language_map.at(language) + ".exd"));
            }
            // Instantiate the data for this language
            _data[language] = std::unique_ptr<Exd>(new Exd(*_header, files));
        }
    }
}

Cat::~Cat()
{

}

const std::string& Cat::get_name() const
{
    return _name;
}

const Exh& Cat::get_header() const
{
    return *_header;
}

const Exd& Cat::get_data_ln(Language i_language) const
{
    auto ln_it = _data.find(i_language);
    if (ln_it == _data.end())
    {
        throw std::runtime_error("No data for language: " + std::to_string(uint16_t(i_language)));
    }

    return *(ln_it->second);
}

void Cat::export_as_csvs(const boost::filesystem::path& i_output_path) const
{
    for (auto language: get_header().get_languages())
    {
        if (language != Language::chs)
        {
            auto output_file_path = i_output_path / (_name + language_map.at(language) + ".txt");

            boost::filesystem::create_directories(output_file_path.parent_path());

            std::ofstream ofs(output_file_path.string());
            get_data_ln(language).get_as_csv(ofs);
            ofs.close();
        }
    }
}

}
}
