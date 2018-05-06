#ifndef FLOW_H
#define FLOW_H

#include <vector>

//#include "flow_manager.h"
#include "flow_node.h"
#include "log.h"

namespace nfv_exam {

class Flow {
public:
    //int get_flow_node(int function_id, FlowNode *flow_node);

    int get_id();
    int get_chain_id();
    int get_lifetime_left();
    int get_length();
    int get_flow_bandwidth();
    const std::vector<int> &get_flow_nodes();
    
    void set_id(int id);
    void set_chain_id(int chain_id);
    void set_lifetime_left(int lifetime);
    void set_length(int length);
    void set_flow_bandwidth(int flow_bandwidth);
    void set_flow_nodes(const std::vector<int> &flow_nodes);

    int get_flow_node_id(int func_id);
    
    int aging();


private:
    int id;
    int chain_id;
    int lifetime_left;
    int length;
    int flow_bandwidth; 
    std::vector<int> flow_nodes;
};

}

#endif /* FLOW_H */
