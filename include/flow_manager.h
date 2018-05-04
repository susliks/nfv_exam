#ifndef FLOW_MANAGER_H
#define FLOW_MANAGER_H

//#include <cstdio>
#include <vector>
#include <map>
#include <string>

#include "flow_node.h"
#include "flow.h"

namespace nfv_exam {

class FlowManager {
public:
    ~FlowManager();

    static FlowManager *get_instance();
    int init();

    int get_flow(int flow_id, Flow *flow);
    int get_flow_node(int flow_node_id, FlowNode *flow_node);

    int create_a_flow(int length, int chain_id, int lifetime, int flow_bandwidth, 
            const std::vector<int> &flow_node_cpu_cost, const std::vector<int> &flow_node_memory_cost, int &flow_id);
    int create_a_flow_node(int &flow_node_id);
    int delete_a_flow(int flow_id);

    int get_flow_node(int flow_id, int func_id, FlowNode *flow_node);

    


    int random_pick_an_active_flow_id(int &flow_id, int &flow_length);

    int set_flow_evaluation_file_path(const std::string &file_path);
    int update_evaluation();
    int save_evaluation();

private:

private:
    std::map<int, Flow*> flow_pool;
    int flow_id_count;
    std::map<int, FlowNode*> flow_node_pool;
    int flow_node_id_count;

    std::string flow_evaluation_file_path;

    std::vector<int> throughput_record;
    std::vector<int> active_flow_count;


};

}

#endif /* FLOW_MANAGER_H */

