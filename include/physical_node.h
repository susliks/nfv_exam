#ifndef PHYSICAL_NODE_H
#define PHYSICAL_NODE_H

#include <vector>
#include <cmath>

namespace nfv_exam {

class PhysicalNode {
public:
    PhysicalNode(int id, int depth, int cpu, int memory, int up_bandwidth);
    PhysicalNode() {} ;

    int add_son(int son_id);
    int set_parent(int parent_id);

    int get_host_cost(double &host_cost);
    int get_pre_host_cost(int cur_cpu_cost, int cur_memory_cost, double &host_cost);

    int get_up_bandwidth();
    int get_parent_id();
    int get_depth();
    int get_cpu_used();
    int get_memory_used();
    int get_up_bandwidth_used();

    int assign_bandwidth(int bandwidth_cost);
    int release_bandwidth(int bandwidth_cost);
    int assign_host_resource(int cpu_cost, int memory_cost);
    int release_host_resource(int cpu_cost, int memory_cost);

private:
    int id;
    int depth;
    int cpu_used;
    int memory_used;
    int up_bandwidth_used;
    int parent_id;
    std::vector<int> son_id;

    int cpu;
    int memory;
    int up_bandwidth;
};

}

#endif
