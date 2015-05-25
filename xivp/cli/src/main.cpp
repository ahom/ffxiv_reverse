#include <iostream>
#include <thread>
#include <mutex>
#include <queue>

#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/format.hpp>

#include <zlib.h>

#include <xiv/utils/log.h>
#include <xiv/utils/crc32.h>

#include <xiv/dat/GameData.h>
#include <xiv/dat/File.h>
#include <xiv/dat/Cat.h>
#include <xiv/dat/Index.h>

#include <xiv/mdl/Model.h>
#include <xiv/mdl/logger.h>

void search_models(xiv::dat::GameData& i_game_data);

int main(int argc, char* argv [])
{
    boost::log::add_file_log("cli.log");

    boost::log::core::get()->set_filter(
        xiv::utils::log::severity_level >= xiv::utils::log::Severity::trace
    );

    auto game_data = xiv::dat::GameData("G:/SquareEnix/FINAL FANTASY XIV - A Realm Reborn/game/sqpack/ffxiv/");

    if (false)
    {
        for (auto cat_nb : game_data.get_cat_nbs())
        {
            auto& cat = game_data.get_category(cat_nb);
            for (auto& hash_table_entry : cat.get_index().get_hash_table())
            {
                for (auto& dir_hash_table_entry : hash_table_entry.second)
                {
                    auto file = cat.get_file(hash_table_entry.first, dir_hash_table_entry.first);

                    if (file->get_type() == xiv::dat::FileType::model)
                    {
                        //xiv::mdl::Model aModel(game_data, *file);
                    }
                }
            }
        }
    }
    else if (true)
    {
        search_models(game_data);
    }
    else if (true)
    {
        std::string model_strings [] = { "chara/equipment/e0044/model/c0101e0044_top.mdl" };

        for (auto& model_string : model_strings)
        {
            xiv::mdl::Model aModel(game_data, model_string);
            aModel.export_as_json("G:/projects/output_mv");
        }
    }
    else
    {

        std::string file_strings [] = {
            "chara/monster/m0096/obj/body/b0001/material/v0001/mt_m0096b0001_b.mtrl"
        };

        for (auto& file_string : file_strings)
        {
            auto file = game_data.get_file(file_string);
            file->export_as_bin("G:/test.bin");
        }

    }

    return 0;
}

void search_models(xiv::dat::GameData& i_game_data)
{
    auto& chara_cat = i_game_data.get_category("chara");
    auto& cat_hash_table = chara_cat.get_index().get_hash_table();

    std::vector<std::thread> producer_thread_pool;
    std::vector<std::thread> consumer_thread_pool;
    std::mutex queue_mutex;
    std::queue<std::string> models_queue;

    bool work_done = false;

    // Creating consumers, 2 * hardware because these threads will spend a lot of time writing to disk
    for (auto i = 0; i < 2 * std::thread::hardware_concurrency(); ++i)
    {
        consumer_thread_pool.emplace_back([&] {
            // Serve forever
            while (true)
            {
                if (models_queue.empty())
                {
                    if (work_done)
                    {
                        break;
                    }
                    // Sleeps for 20 ms if nothing in the queue
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                }
                else
                {
                    std::string model_path;
                    {
                        std::lock_guard<std::mutex> lock(queue_mutex);
                        if (!models_queue.empty())
                        {
                            model_path = models_queue.front();
                            models_queue.pop();
                        }
                    }
                    if (!model_path.empty())
                    {
                        try
                        {
                            xiv::mdl::Model(i_game_data, model_path).export_as_json("G:/projects/output_mv");
                        }
                        catch (std::exception& e)
                        {
                            XIV_ERROR(xiv_mdl_logger, "ERROR on file: " << model_path << " - " << e.what());
                        }
                    }
                }
            }
        });
    }

    // producer threads
    producer_thread_pool.emplace_back([&] {
        std::vector<uint32_t> crc_values;
        std::string parts [] = { "face", "hair", "tail", "body" };
        std::string suffixes [] = { "fac", "hir", "til", "top" };
        std::string dir_str_format = "chara/human/c%04d/obj/%s/%s%04d/model";

        for (uint32_t i = 0; i < 4; ++i)
        {
            std::string part_str_format_in = boost::str(boost::format(dir_str_format) % 0 % parts[i] % parts[i][0] % 0);

            xiv::utils::crc32::generate_hashes_2(part_str_format_in, 13, 28, crc_values);

            for (uint32_t c = 0; c < 10000; ++c)
            {
                for (uint32_t p = 0; p < 10000; ++p)
                {
                    if (cat_hash_table.find(crc_values[c * 10000 + p]) != cat_hash_table.end())
                    {
                        std::string full_path = boost::str(boost::format(dir_str_format + "/c%04d%s%04d_%s.mdl") % c % parts[i] % parts[i][0] % p % c % parts[i][0] % p % suffixes[i]);

                        if (i_game_data.check_file_existence(full_path))
                        {
                            XIV_INFO(xiv_mdl_logger, "Found human: " << full_path);
                            queue_mutex.lock();
                            models_queue.push(full_path);
                            queue_mutex.unlock();
                        }
                    }
                }
            }
        }
    });

    producer_thread_pool.emplace_back([&] {
        std::vector<uint32_t> dir_crc_values;
        std::vector<uint32_t> file_crc_values;
        std::string suffixes [] = { "met", "top", "glv", "dwn", "sho" };
        std::string dir_str_format = "chara/equipment/e%04d/model";
        std::string dir_str_format_in = boost::str(boost::format(dir_str_format) % 0);

        xiv::utils::crc32::generate_hashes_1(dir_str_format_in, 17, dir_crc_values);

        for (uint32_t e = 0; e < 10000; ++e)
        {
            auto cat_it = cat_hash_table.find(dir_crc_values[e]);
            if (cat_it != cat_hash_table.end())
            {
                for (auto& suffix : suffixes)
                {
                    std::string file_str_format = "c%04d" + boost::str(boost::format("e%04d_%s.mdl") % e % suffix);
                    std::string file_str_format_in = boost::str(boost::format(file_str_format) % 0);

                    xiv::utils::crc32::generate_hashes_1(file_str_format_in, 1, file_crc_values);

                    for (uint32_t c = 0; c < 10000; ++c)
                    {
                        if (cat_it->second.find(file_crc_values[c]) != cat_it->second.end())
                        {
                            std::string full_path = boost::str(boost::format(dir_str_format + "/" + file_str_format) % e % c);

                            XIV_INFO(xiv_mdl_logger, "Found equipment: " << full_path);
                            queue_mutex.lock();
                            models_queue.push(full_path);
                            queue_mutex.unlock();
                        }
                    }
                }
            }
        }
    });

    producer_thread_pool.emplace_back([&] {
        std::vector<uint32_t> dir_crc_values;
        std::vector<uint32_t> file_crc_values;
        std::string suffixes [] = { "met", "top", "glv", "dwn", "sho" };
        std::string dir_str_format = "chara/demihuman/d%04d/obj/equipment/e%04d/model";
        std::string dir_str_format_in = boost::str(boost::format(dir_str_format) % 0 % 0);

        xiv::utils::crc32::generate_hashes_2(dir_str_format_in, 17, 37, dir_crc_values);

        for (uint32_t d = 0; d < 10000; ++d)
        {
            for (uint32_t e = 0; e < 10000; ++e)
            {
                auto cat_it = cat_hash_table.find(dir_crc_values[d * 10000 + e]);
                if (cat_it != cat_hash_table.end())
                {
                    for (auto& suffix : suffixes)
                    {
                        std::string full_path = boost::str(boost::format(dir_str_format + "/d%04de%04d_%s.mdl") % d % e % d % e % suffix);

                        if (i_game_data.check_file_existence(full_path))
                        {
                            XIV_INFO(xiv_mdl_logger, "Found demihuman: " << full_path);
                            queue_mutex.lock();
                            models_queue.push(full_path);
                            queue_mutex.unlock();
                        }
                    }
                }
            }
        }
    });

    producer_thread_pool.emplace_back([&] {
        std::vector<uint32_t> dir_crc_values;
        std::vector<uint32_t> file_crc_values;
        std::string suffixes [] = { "ril", "rir", "wrs", "nek", "ear" };
        std::string dir_str_format = "chara/accessory/a%04d/model";
        std::string dir_str_format_in = boost::str(boost::format(dir_str_format) % 0);

        xiv::utils::crc32::generate_hashes_1(dir_str_format_in, 17, dir_crc_values);

        for (uint32_t a = 0; a < 10000; ++a)
        {
            auto cat_it = cat_hash_table.find(dir_crc_values[a]);
            if (cat_it != cat_hash_table.end())
            {
                for (auto& suffix : suffixes)
                {
                    std::string file_str_format = "c%04d" + boost::str(boost::format("a%04d_%s.mdl") % a % suffix);
                    std::string file_str_format_in = boost::str(boost::format(file_str_format) % 0);

                    xiv::utils::crc32::generate_hashes_1(file_str_format_in, 1, file_crc_values);

                    for (uint32_t c = 0; c < 10000; ++c)
                    {
                        if (cat_it->second.find(file_crc_values[c]) != cat_it->second.end())
                        {
                            std::string full_path = boost::str(boost::format(dir_str_format + "/" + file_str_format) % a % c);

                            XIV_INFO(xiv_mdl_logger, "Found acessory: " << full_path);
                            queue_mutex.lock();
                            models_queue.push(full_path);
                            queue_mutex.unlock();
                        }
                    }
                }
            }
        }
    });

    producer_thread_pool.emplace_back([&] {
        std::vector<uint32_t> crc_values;
        std::string dir_str_format = "chara/weapon/w%04d/obj/body/b%04d/model";
        std::string dir_str_format_in = boost::str(boost::format(dir_str_format) % 0 % 0);

        xiv::utils::crc32::generate_hashes_2(dir_str_format_in, 14, 29, crc_values);

        for (uint32_t w = 0; w < 10000; ++w)
        {
            for (uint32_t b = 0; b < 10000; ++b)
            {
                if (cat_hash_table.find(crc_values[w * 10000 + b]) != cat_hash_table.end())
                {
                    std::string full_path = boost::str(boost::format(dir_str_format + "/w%04db%04d.mdl") % w % b % w % b);

                    if (i_game_data.check_file_existence(full_path))
                    {
                        XIV_INFO(xiv_mdl_logger, "Found weapon: " << full_path);
                        queue_mutex.lock();
                        models_queue.push(full_path);
                        queue_mutex.unlock();
                    }
                }
            }
        }
    });

    producer_thread_pool.emplace_back([&] {
        std::vector<uint32_t> crc_values;
        std::string dir_str_format = "chara/monster/m%04d/obj/body/b%04d/model";
        std::string dir_str_format_in = boost::str(boost::format(dir_str_format) % 0 % 0);

        xiv::utils::crc32::generate_hashes_2(dir_str_format_in, 15, 30, crc_values);

        for (uint32_t m = 0; m < 10000; ++m)
        {
            for (uint32_t b = 0; b < 10000; ++b)
            {
                if (cat_hash_table.find(crc_values[m * 10000 + b]) != cat_hash_table.end())
                {
                    std::string full_path = boost::str(boost::format(dir_str_format + "/m%04db%04d.mdl") % m % b % m % b);

                    if (i_game_data.check_file_existence(full_path))
                    {
                        XIV_INFO(xiv_mdl_logger, "Found monster: " << full_path);
                        queue_mutex.lock();
                        models_queue.push(full_path);
                        queue_mutex.unlock();
                    }
                }
            }
        }
    });

    for (auto& thread : producer_thread_pool)
    {
        thread.join();
    }

    work_done = true;

    for (auto& thread : consumer_thread_pool)
    {
        thread.join();
    }
}


