#ifndef VNF_INSTANCE_H
#define VNF_INSTANCE_H

#include <vector>
#include <set>

#include "physical_node.h"

namespace nfv_exam {

class VnfInstance {
public:
    int init(int id, int location, bool disable_scale_up_down, int vi_cpu_used, 
            int vi_memory_used, int vi_cpu_cost, int vi_memory_cost);
    int set_vi_resource_used(int cpu_used, int memory_used);
    int set_vi_resource_cost(int vi_cpu_cost, int vi_memory_cost);
    int add_pre_vi_id(int pre_vi_id);
    int add_next_vi_id(int next_vi_id);
    int remove_pre_vi_id(int pre_vi_id);
    int remove_next_vi_id(int next_vi_id);

    int get_id();
    int get_cpu_cost();
    int get_memory_cost();
    int get_location();
    bool get_disable_scale_up_down();

    

    int add_settled_flow_node(int flow_node_id);
    int remove_settled_flow_node(int flow_node_id);
    int get_settled_flow_node(std::vector<int> &settled_flow_nodes);
    bool has_settled_flow_node();

    int settle(int pn_id);
    int remove();
    bool is_settled();

    const std::string to_string();


private:
    int id;
    int location;
    std::vector<int> pre_vi_id;
    std::vector<int> next_vi_id;
    std::set<int> settled_flow_nodes;

    bool disable_scale_up_down;
    int vi_cpu_used;    //flow cost
    int vi_memory_used;
    int vi_cpu_cost;    //vi cost, when type==normal: vi_cpu_cost == vi_cpu_used
    int vi_memory_cost;

};

}

#endif /* VNF_INSTANCE_H */
