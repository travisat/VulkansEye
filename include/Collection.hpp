#pragma once

#include <map>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>


namespace tat
{

class Entry
{
  public:
    bool loaded = false;
    std::string name{};
    virtual void load(){};
};

template <class T> class Collection
{
    static_assert(std::is_base_of<Entry, T>::value, "T must derive from tat::Entry");

  public:
    // type of collection, this is in state json (backdrops/materials/meshes/models)
    explicit Collection(const std::string &type)
    {
        auto &state = State::instance();

        int32_t index = 0;
        for (auto &[key, config] : state.at(type).items())
        {
            collection.push_back(std::make_shared<T>());
            collection[index]->name = key;
            names.insert(std::make_pair(key, index));
            ++index;
        }
        if (index == 0)
        { // load a default in if none was found
            collection.push_back(std::make_shared<T>());
            names.insert(std::make_pair("default", 0));
        }
    };

    ~Collection() = default;

    auto get(const std::string &name) -> std::shared_ptr<T>
    {
        int32_t index = getIndex(name);
        return load(index);
    }

    auto load(int32_t index) -> std::shared_ptr<T>
    {
        auto &entry = collection[index];
        // if already loaded return
        if (entry->loaded == true)
        {
            return entry;
        }
        // otherwise load the material
        entry->load();
        return entry;
    }
  private:
    // collection of shared ptrs
    // shared_ptrs objects are lazy loaded so name isn't in collection until loaded
    std::vector<std::shared_ptr<T>> collection{};
    // map of names to index of shared ptr for faster retrieval
    std::map<std::string, int32_t> names{};

    auto getIndex(const std::string &name) -> int32_t
    {
        auto it = names.find(name);
        if (it != names.end())
        { // if name is found return value of map (index)
            return it->second;
        }
        // else return 0 which should always exist
        return 0;
    }
};

} // namespace tat