#include <xiv/dat/Cat.h>

#include <xiv/dat/logger.h>
#include <xiv/dat/Index.h>
#include <xiv/dat/Dat.h>
#include <xiv/dat/File.h>

namespace xiv
{
namespace dat
{

Cat::Cat(const boost::filesystem::path& i_base_path, uint32_t i_cat_nb, const std::string& i_name) :
    _name(i_name),
    _nb(i_cat_nb)
{
    XIV_INFO(xiv_dat_logger, "Initializing Cat with path: " << i_base_path << " - nb: " << i_cat_nb << " - name: " << i_name);

    // From the category number, compute back the real filename for.index .datXs
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << std::hex << i_cat_nb;
    std::string prefix = ss.str() + "0000.win32";

    // Creates the index: XX0000.win32.index
    _index = std::unique_ptr<Index>(new Index(i_base_path / (prefix + ".index")));

    // For all dat files linked to this index, create it: XX0000.win32.datX
    for (uint32_t i = 0; i < get_index().get_dat_count(); ++i)
    {
        _dats.emplace_back(std::unique_ptr<Dat>(new Dat(i_base_path / (prefix + ".dat" + std::to_string(i)), i)));
    }
}

Cat::~Cat()
{

}

const Index& Cat::get_index() const
{
    return *_index;
}

std::unique_ptr<File> Cat::get_file(uint32_t dir_hash, uint32_t filename_hash) const
{
    XIV_DEBUG(xiv_dat_logger, "Get file cat: " << _name << " - nb: " << _nb << " - dir_hash: " << dir_hash << " - filename_hash : " << filename_hash);

    // Fetch the correct hash_table_entry for these hashes, from that request the file from the right dat file
    auto& hash_table_entry = get_index().get_hash_table_entry(dir_hash, filename_hash);
    return _dats[hash_table_entry.dat_nb]->get_file(hash_table_entry.dat_offset);
}

bool Cat::check_file_existence(uint32_t dir_hash, uint32_t filename_hash) const
{
    return get_index().check_file_existence(dir_hash, filename_hash);
}
bool Cat::check_dir_existence(uint32_t dir_hash) const
{
    return get_index().check_dir_existence(dir_hash);
}

const std::string& Cat::get_name() const
{
    return _name;
}

uint32_t Cat::get_nb() const
{
    return _nb;
}

}
}
