#ifndef FLOW_NODE_H
#define FLOW_NODE_H

#include <vector>

#include "log.h"

namespace nfv_exam {

class FlowNode {
public:
    int settle(int vi_id);
    int remove(int vi_id);
    bool is_settled();
    bool has_pre_node();
    bool has_next_node();
    int get_pre_node();
    int get_next_node();

    int get_location();
    int remove();


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
