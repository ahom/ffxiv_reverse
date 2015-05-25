#include <xiv/exd/ExdData.h>

#include <xiv/utils/stream.h>

#include <xiv/dat/GameData.h>
#include <xiv/dat/File.h>

#include <xiv/exd/logger.h>
#include <xiv/exd/Cat.h>

namespace xiv
{
namespace exd
{

ExdData::ExdData(dat::GameData& i_game_data) try :
    _game_data(i_game_data)
{
    XIV_INFO(xiv_exd_logger, "Initializing ExdData");

    // Fetch the root.exl and get a stream from it
    auto root_exl = i_game_data.get_file("exd/root.exl");
    auto stream_ptr = utils::stream::get_istream(root_exl->get_data_sections().front());
    auto& stream = *stream_ptr;

    // Iterates over the lines while skipping the first one
    std::string line;
    std::getline(stream, line); // extract first line EXLT,2
    std::getline(stream, line);

    // Until the EOF
    while (!line.empty())
    {
        // Format is cat_name,XX
        // XX being an internal identifier
        // Get only the cat_name
        auto sep = line.find(',');
        auto category = line.substr(0, sep);

        // Add to the list of category name
        // creates the empty category in the cats map
        // instantiate the creation mutex for this category
        _cat_names.push_back(category);
        _cats[category] = std::unique_ptr<Cat>();
        _cat_creation_mutexes[category] = std::unique_ptr<std::mutex>(new std::mutex());

        std::getline(stream, line);
    }
}
catch(std::exception& e)
{
    // In case of failure here, client is supposed to catch the exception because it is not recoverable on our side
    XIV_FATAL(xiv_exd_logger, "ExdData initialization failed: " << e.what());
    throw;
}

ExdData::~ExdData()
{

}

const std::vector<std::string>& ExdData::get_cat_names() const
{
    return _cat_names;
}

const Cat& ExdData::get_category(const std::string& i_cat_name)
{
    // Get the category from its name
    auto cat_it = _cats.find(i_cat_name);
    if (cat_it == _cats.end())
    {
        throw std::runtime_error("Category not found: " + i_cat_name);
    }

    if (cat_it->second)
    {
        // If valid return it
        return *(cat_it->second);
    }
    else
    {
        // If not, create it and return it
        create_category(i_cat_name);
        return *(_cats[i_cat_name]);
    }
}

void ExdData::create_category(const std::string& i_cat_name)
{
    // Lock mutex in this scope
    std::lock_guard<std::mutex> lock(*(_cat_creation_mutexes[i_cat_name]));
    // Maybe after unlocking it has already been created, so check (most likely if it blocked)
    if (!_cats[i_cat_name])
    {
        _cats[i_cat_name] = std::unique_ptr<Cat>(new Cat(_game_data, i_cat_name));
    }
}

void ExdData::export_as_csvs(const boost::filesystem::path& i_output_path)
{
    auto csv_output_path = i_output_path / "csv";

    for (auto& cat_name: get_cat_names())
    {
        get_category(cat_name).export_as_csvs(csv_output_path);
    }
}

}
}
