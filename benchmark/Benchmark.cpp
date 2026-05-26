#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <unordered_set>
#include <iomanip>
#include <numeric>
#include <string>
#include "../src/FlatVectorStore.h"

using namespace std;
using namespace std::chrono;

const unsigned int RANDOM_SEED = 42;
const int NUM_VECTORS = 50000;
const int DIMENSION = 64;
const int NUM_QUERIES = 100;
const int K = 10;

struct ResultRow
{
    string metric;
    string kpp;
    string mode;
    string nprobe;
    string iterations;
    double avg_time_ms;
    double avg_recall;
    double avg_scanned;
};

int main()
{
    FLatVectorStore store(DIMENSION);
    mt19937 gen(RANDOM_SEED);
    normal_distribution<float> dist(0.0f, 1.0f);

    cout << "==================================================" << endl;
    cout << "GENERATING DATASET & INITIAL INGESTION" << endl;
    cout << "==================================================" << endl;

    vector<vector<float>> dataset(NUM_VECTORS, vector<float>(DIMENSION));
    for (int i = 0; i < NUM_VECTORS; i++)
    {
        for (int d = 0; d < DIMENSION; d++)
        {
            dataset[i][d] = dist(gen);
        }
    }

    vector<vector<float>> queries(NUM_QUERIES, vector<float>(DIMENSION));
    for (int i = 0; i < NUM_QUERIES; i++)
    {
        for (int d = 0; d < DIMENSION; d++)
        {
            queries[i][d] = dist(gen);
        }
    }

    for (long long i = 0; i < NUM_VECTORS; i++)
    {
        store.insert(i, dataset[i]);
    }
    cout << "Ingested " << NUM_VECTORS << " vectors successfully." << endl;

    vector<ResultRow> performance_table;

    vector<bool> metric_modes = {false, true};     // false = Euclidean, true = Cosine
    vector<bool> kpp_modes = {false, true};         // false = Random,    true = K-Means++
    vector<bool> refinement_modes = {false, true};  // false = Standard,  true = Refined
    vector<int> nprobe_values {1, 5, 10, 25};

    vector<vector<float>> ivf_res;

    for (size_t m = 0; m < metric_modes.size(); m++)
    {
        bool use_cosine = metric_modes[m];
        string metric_label = use_cosine ? "COSINE" : "EUCLIDEAN";
        store.set_cosine(use_cosine);

        cout << "\n>>> Evaluating Metric: " << metric_label << " <<<" << endl;
        cout << "Computing Brute Force Baseline Ground Truth..." << endl;
        
        vector<unordered_set<long long>> ground_truth(NUM_QUERIES);
        double total_brute_time_ms = 0;

        for (int q = 0; q < NUM_QUERIES; q++)
        {
            vector<vector<float>> k_vectors;
            vector<pair<long long, float>> ids_dis;

            auto start_q = high_resolution_clock::now();
            store.k_nearest(K, queries[q], k_vectors, ids_dis);
            auto end_q = high_resolution_clock::now();

            total_brute_time_ms += duration<double, std::milli>(end_q - start_q).count();

            for (size_t i = 0; i < ids_dis.size(); i++)
            {
                ground_truth[q].insert(ids_dis[i].first);
            }
        }
        double avg_brute_time = total_brute_time_ms / NUM_QUERIES;

        ResultRow brute_row;
        brute_row.metric = metric_label;
        brute_row.kpp = "N/A";
        brute_row.mode = "BRUTE";
        brute_row.nprobe = "N/A";
        brute_row.iterations = "N/A";
        brute_row.avg_time_ms = avg_brute_time;
        brute_row.avg_recall = 1.000;
        brute_row.avg_scanned = (double)NUM_VECTORS;
        performance_table.push_back(brute_row);

        for (size_t k_idx = 0; k_idx < kpp_modes.size(); k_idx++)
        {
            bool use_kpp = kpp_modes[k_idx];
            string kpp_label = use_kpp ? "KPP_ON" : "KPP_OFF";
            store.set_kpp(use_kpp);

            cout << "Building IVF Index (" << kpp_label << ")... " << flush;
            auto start_build = high_resolution_clock::now();
            store.llyods_algorithm();
            auto end_build = high_resolution_clock::now();
            double build_time_sec = duration<double>(end_build - start_build).count();

            int actual_iterations = store.get_iterations();
            string iter_label = to_string(actual_iterations);

            cout << "Done (" << fixed << setprecision(2) << build_time_sec << "s). "
                 << "Iterations: " << actual_iterations << " | Clusters: " << store.get_no_clusters() << endl;

            for (size_t r = 0; r < refinement_modes.size(); r++)
            {
                bool use_refinement = refinement_modes[r];
                string mode_label = use_refinement ? "IVF_REFINED" : "IVF_STANDARD";
                store.set_multi_probe(use_refinement);

                for (size_t np = 0; np < nprobe_values.size(); np++)
                {
                    int nprobe = nprobe_values[np];
                    double total_ivf_time_ms = 0;
                    double total_recall = 0;
                    long long total_scanned_vectors = 0;

                    for (int q = 0; q < NUM_QUERIES; q++)
                    {
                        int scanned = 0;
                        auto start_q = high_resolution_clock::now();
                        ivf_res = store.IVF(nprobe, K, queries[q], scanned);
                        auto end_q = high_resolution_clock::now();

                        total_ivf_time_ms += duration<double, std::milli>(end_q - start_q).count();
                        total_scanned_vectors += scanned;

                        if (!ivf_res.empty() && !ivf_res[0].empty())
                        {
                            int matches = 0;
                            for (size_t i = 0; i < ivf_res[0].size(); i++)
                            {
                                // Safe long-term index truncation patch
                                long long ivf_id = static_cast<long long>(round(ivf_res[0][i]));
                                if (ground_truth[q].find(ivf_id) != ground_truth[q].end())
                                {
                                    matches++;
                                }
                            }
                            total_recall += (double)matches / K;
                        }
                    }

                    ResultRow ivf_row;
                    ivf_row.metric = metric_label;
                    ivf_row.kpp = kpp_label;
                    ivf_row.mode = mode_label;
                    ivf_row.nprobe = to_string(nprobe);
                    ivf_row.iterations = iter_label;
                    ivf_row.avg_time_ms = total_ivf_time_ms / NUM_QUERIES;
                    ivf_row.avg_recall = total_recall / NUM_QUERIES;
                    ivf_row.avg_scanned = (double)total_scanned_vectors / NUM_QUERIES;
                    performance_table.push_back(ivf_row);
                }
            }
        }
    }

    cout << "\n" << string(110, '=') << endl;
    cout << left << setw(12) << "METRIC"
         << setw(10) << "KPP"
         << setw(15) << "MODE"
         << setw(10) << "NPROBE"
         << setw(12) << "ITERATIONS"
         << setw(18) << "AVG TIME (ms)"
         << setw(16) << "AVG RECALL@10"
         << setw(15) << "VECS SCANNED" << endl;

    for (size_t i = 0; i < performance_table.size(); i++)
    {
        cout << left << setw(12) << performance_table[i].metric
             << setw(10) << performance_table[i].kpp
             << setw(15) << performance_table[i].mode
             << setw(10) << performance_table[i].nprobe
             << setw(12) << performance_table[i].iterations
             << fixed << setprecision(3) << setw(18) << performance_table[i].avg_time_ms
             << setprecision(3) << setw(16) << performance_table[i].avg_recall
             << setprecision(1) << setw(15) << performance_table[i].avg_scanned << endl;
    }
    cout << string(110, '=') << endl;

    return 0;
}
