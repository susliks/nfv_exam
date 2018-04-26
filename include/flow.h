#ifndef FLOW_H
#define FLOW_H

#include <vector>

//#include "flow_manager.h"
#include "flow_node.h"
#include "log.h"

namespace nfv_exam {

class Flow {
public:
    int get_flow_node(int function_id, FlowNode &flow_node);

    int get_length();


private:
    int id;
    int chain_id;
    int lifetime_left;
    int length;
    std::vector<int> flow_nodes;
    int bandwidth_cost; 
};

}

#endif /* FLOW_H */
