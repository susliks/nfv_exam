#ifndef PHYSICAL_NODE_MANAGER_H
#define PHYSICAL_NODE_MANAGER_H

//#include <cstdio>
#include <vector>
#include <map>

#include "physical_node.h"

namespace nfv_exam {

class PhysicalNodeManager {
public:
    ~PhysicalNodeManager();

    static PhysicalNodeManager *get_instance();

    int build_3_level_topo(int level0_son_num, int level1_son_num, int level2_son_num, \
            int server_cpu, int server_memory, int server_up_bandwidth, \
            double level0_bandwidth_decay_rate, double level1_bandwidth_decay_rate);

    int get_bandwidth_hop_count(int physical_node_id_1, int physical_node_id_2, int &hop_count);

    int assign_bandwidth(int physical_node_id_1, int physical_node_id_2, int bandwidth);
    int assign_host_resource(int physical_node_id, int cpu_cost, int memory_cost);
    int release_bandwidth(int physical_node_id_1, int physical_node_id_2, int bandwidth);
    int release_host_resource(int physical_node_id, int cpu_cost, int memory_cost);

    int get_physical_node(int node_id, PhysicalNode &physical_node);

    int get_total_cpu_statistics(int &total_cpu_used, int &total_cpu);
    int get_total_memory_statistics(int &total_memory_used, int &total_memory);
    int get_total_bandwidth_statistics(int &total_bandwidth_used, int &total_bandwidth);

private:
    int add_physical_node(int parent_id);

    int assign_total_host_resource(int cpu_cost, int memory_cost);
    int assign_total_bandwidth(int bandwidth_cost);
    int release_total_host_resource(int cpu_cost, int memory_cost);
    int release_total_bandwidth(int bandwidth_cost);

    int calculate_total_cpu_cost(); // for check
    int calculate_total_memory_cost();
    int calculate_total_bandwidth_cost();

private:
    std::map<int, PhysicalNode*> physical_node;
    int total_cpu_used;
    int total_memory_used;
    int total_bandwidth_used;
    int total_cpu;
    int total_memory;
    int total_bandwidth;
};

}

#endif

