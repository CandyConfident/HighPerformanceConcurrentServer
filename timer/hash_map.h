#ifndef __HASH_hm_mapH__
#define __HASH_hm_mapH__

#include <unordered_map>
#include <mutex>

using namespace std;

template <typename K, typename V>
class hash_map{
public:
    void emplace(const K& key, const V& v) {
        unique_lock<mutex> lck(hm_mutex);
        hm_map[key] = v;
    }

    void emplace(const K& key, V&& v) {
        unique_lock<mutex> lck(hm_mutex);
        hm_map[key] = move(v);
    }

    void erase(const K& key) {
        unique_lock<mutex> lck(hm_mutex);
        if (hm_map.find(key) != hm_map.end()) {
            hm_map.erase(key);
        }
    }

    bool get_val(const K& key, V& value) {
        unique_lock<mutex> lck(hm_mutex);
        if (hm_map.find(key) != hm_map.end()) {
            value = hm_map[key];
            return true;
        }
        return false;
    }

    bool is_key_exist(const K& key) {
        unique_lock<mutex> lck(hm_mutex);
        return hm_map.find(key) != hm_map.end();
    }

    size_t size() {
        unique_lock<mutex> lck(hm_mutex);
        return hm_map.size();
    }

private:
    unordered_map<K, V> hm_map;
    mutex hm_mutex;
};

#endif