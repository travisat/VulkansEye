#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace tat
{

// Entry is parent class for classes stored in collection
// At this time derived classes must be able to be constructed
// without args to work in Collection
class Entry
{
  public:
    // all entries must know if loaded or not
    bool loaded = false;
    // load should set loaded to true in derived class after loading
    virtual void load(){};
    // name is set on construction of the collection
    std::string name{};
};

// Collection is a lazy loading container
// Objects in collection must be derived from Entry
// Until get or load is called the entry is not loaded
// get will load and return the entry or return an already loaded entry
// type: this should be the type that holds T in state
// ie backdrops for Backdrop
template <class T> class Collection
{
    // make sure this template only is used for Entries
    static_assert(std::is_base_of<Entry, T>::value, "T must derive from tat::Entry");

  public:
    // type of collection, this is in state json (backdrops/materials/meshes/models)
    void create(const std::string &type)
    {
        // get state
        auto &state = State::instance();

        int32_t index = 0;
        for (auto &[key, _] : state.at(type).items())
        { // iterate through type in state
            T t{};
            t.name = key;
            collection.push_back(t);
            // store name,index in map for getting index later
            names.insert(std::make_pair(key, index));
            ++index;
        }
    };

    // returns ptr to loaded entry
    // loads entry if it is not loaded
    auto get(const std::string &name) -> T *
    {
        int32_t index = getIndex(name);
        return load(index);
    }

    // return index of entry if exists, -1 otherwise
    // does not load entry
    auto getIndex(const std::string &name) -> int32_t
    {
        auto it = names.find(name);
        if (it != names.end())
        { // if name is found return value of map (index)
            return it->second;
        }
        // else return -1
        return -1;
    }

    // loads entry at index if not loaded and return ptr to it
    // if index does not exist it returns item at index 0
    auto load(int32_t index) -> T *
    {
        if (index > collection.size() || index < 0)
        { // return null if index out of range
            return nullptr;
        }
        // load default entry so we always have an entry
        auto entry = &collection[index];

        if (entry->loaded == true)
        { // if already loaded return
            return entry;
        }
        // otherwise load the material
        entry->load();
        return entry;
    }

    // destroys collection
    void destroy()
    {
        collection.clear();
        names.clear();
    }

  private:
    std::vector<T> collection{};
    // map of names to index
    std::map<std::string, int32_t> names{};
};

} // namespace tat