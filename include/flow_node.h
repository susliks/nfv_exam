#ifndef FLOW_NODE_H
#define FLOW_NODE_H

#include <vector>

#include "log.h"
//#include "flow.h"
#include "flow_manager.h"

namespace nfv_exam {

class FlowNode {
public:

    int init(int id, int flow_id, int function_id, int cpu_cost, int memory_cost);
    int set_pre_flow_node_id(int pre_flow_node_id);
    int set_next_flow_node_id(int next_flow_node_id);

    int get_id();
    int get_flow_id();
    int get_function_id();
    int get_cpu_cost();
    int get_memory_cost();

    int get_flow_bandwidth();

    int settle(int vi_id);
    //int remove();
    bool is_settled();
    bool has_pre_node();
    bool has_next_node();
    FlowNode *get_pre_node();
    FlowNode *get_next_node();
    int get_pre_node_id();
    int get_next_node_id();

    int get_location();
    int remove();

    int adjust_cost(int new_cpu_cost, int new_memory_cost);


private:
    int id;
    int flow_id;
    int function_id;
    int flow_node_location;
    int cpu_cost;
    int memory_cost;
    int pre_flow_node_id;
    int next_flow_node_id;
};

}

#endif /* FLOW_NODE_H */
