// Snapshot.h
#pragma once

#include <cstdint>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include "FlatVectorStore.h"

using namespace std;

bool save_snapshot(FLatVectorStore& store) {
    string tmp_path = "./vdata/snapshot.vdb.tmp";
    string final_path = "./vdata/snapshot.vdb";
    
    ofstream file(tmp_path, ios::binary);
    if (!file.is_open()) {
        cerr << "Failed to open temporary file for writing" << endl;
        return false;
    }
    
    uint32_t magic = 0x31424456;
    uint32_t version = 1;
    uint32_t dimension = store.dimension;  // Direct access (friend)
    uint32_t vector_count = store.ids.size();  // Direct access (friend)
    uint32_t cluster_count = store.ivf_built ? store.vectors_cluster_id.size() : 0;  // Direct access (friend)
    
    file.write((char*)(&magic), sizeof(magic));
    file.write((char*)(&version), sizeof(version));
    file.write((char*)(&dimension), sizeof(dimension));
    file.write((char*)(&vector_count), sizeof(vector_count));
    file.write((char*)(&cluster_count), sizeof(cluster_count));
    
    file.write((char*)(store.ids.data()), vector_count * sizeof(int64_t));
    
    file.write((char*)(store.vectors.data()), vector_count * dimension * sizeof(float));
    
    // Write centroids if index exists
    if (store.ivf_built && cluster_count > 0) {
        file.write((char*)(store.centroids.data()), cluster_count * dimension * sizeof(float));
        
        for (uint32_t i = 0; i < cluster_count; i++) {
            uint32_t cluster_size = store.vectors_cluster_id[i].size();
            file.write((char*)(&cluster_size), sizeof(cluster_size));
            if (cluster_size > 0) {
                vector<uint32_t> indices32(cluster_size);
                for (uint32_t j = 0; j < cluster_size; j++) {
                    indices32[j] = store.vectors_cluster_id[i][j];
                }
                file.write((char*)(indices32.data()), cluster_size * sizeof(uint32_t));
            }
        }
    }
    
    file.close();
    
    if (rename(tmp_path.c_str(), final_path.c_str()) != 0) {
        cerr << "Failed to rename temporary file" << endl;
        return false;
    }
    
    cout << "Saved " << vector_count << " vectors to " << final_path << endl;
    return true;
}

bool load_snapshot(FLatVectorStore& store) {
    string file_path = "./vdata/snapshot.vdb";
    ifstream file(file_path, ios::binary);
    if (!file.is_open()) {
        cout << "No snapshot file found at " << file_path << endl;
        return false;
    }
    
    // Read header
    uint32_t magic, version, dimension, vector_count, cluster_count;
    
    file.read((char*)(&magic), sizeof(magic));
    file.read((char*)(&version), sizeof(version));
    file.read((char*)(&dimension), sizeof(dimension));
    file.read((char*)(&vector_count), sizeof(vector_count));
    file.read((char*)(&cluster_count), sizeof(cluster_count));
    
    // Validate
    if (magic != 0x31424456 || version != 1 || dimension != store.dimension) {
        cerr << "Invalid snapshot file" << endl;
        return false;
    }
    
    // Clear current store
    store.ids.clear();
    store.vectors.clear();
    store.id_to_pos.clear();
    store.vectors_cluster_id.clear();
    store.centroids.clear();
    store.ivf_built = false;
    
    store.ids.resize(vector_count);
    file.read((char*)(store.ids.data()), vector_count * sizeof(int64_t));
    
    store.vectors.resize(vector_count * dimension);
    file.read((char*)(store.vectors.data()), vector_count * dimension * sizeof(float));
    
    for (size_t i = 0; i < store.ids.size(); i++) {
        store.id_to_pos[store.ids[i]] = i;
    }
    
    // Read index if present
    if (cluster_count > 0) {
        store.centroids.resize(cluster_count * dimension);
        file.read((char*)(store.centroids.data()), cluster_count * dimension * sizeof(float));
        
        store.vectors_cluster_id.resize(cluster_count);
        for (uint32_t i = 0; i < cluster_count; i++) {
            uint32_t cluster_size;
            file.read((char*)(&cluster_size), sizeof(cluster_size));
            store.vectors_cluster_id[i].resize(cluster_size);
            if (cluster_size > 0) {
                vector<uint32_t> indices32(cluster_size);
                file.read((char*)(indices32.data()), cluster_size * sizeof(uint32_t));

                for (uint32_t j = 0; j < cluster_size; j++) {
                    store.vectors_cluster_id[i][j] = indices32[j];
                }
            }
        }
        store.ivf_built = true;
    }
    
    file.close();
    cout << "Loaded " << vector_count << " vectors from " << file_path << endl;
    return true;
}