#ifndef VNF_INSTANCE_H
#define VNF_INSTANCE_H

#include <vector>
#include <set>

namespace nfv_exam {

class VnfInstance {
public:
    int init(int id, int location, bool disable_scale_up_down, int vi_cpu_used, 
            int vi_memory_used, int vi_cpu_cost, int vi_memory_cost);
    int set_vi_resource_used(int cpu_used, int memory_used);
    int set_pre_vi_id(int pre_vi_id);
    int set_next_vi_id(int next_vi_id);

    int get_id();
    int get_cpu_cost();
    int get_memory_cost();
    int get_location();
    

    int add_settled_flow_node(int flow_node_id);
    int remove_settled_flow_node(int flow_node_id);

    int settle(in pn_id);
    int remove();
    bool is_settled();


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
