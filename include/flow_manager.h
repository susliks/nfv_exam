#ifndef FLOW_MANAGER_H
#define FLOW_MANAGER_H

//#include <cstdio>
#include <vector>
#include <map>

#include "flow_node.h"
#include "flow.h"

namespace nfv_exam {

class FlowManager {
public:
    ~FlowManager();

    static FlowManager *get_instance();

    int get_flow(int flow_id, Flow &flow);
    int get_flow_node(int flow_node_id, FlowNode &flow_node);

    int random_pick_an_active_flow_id(int &flow_id, int &flow_length);


private:

private:
    std::map<int, Flow*> flow_pool;
    int flow_id_count;
    std::map<int, FlowNode*> flow_node_pool;
    int flow_node_id_count;


};

}

#endif /* FLOW_MANAGER_H */

