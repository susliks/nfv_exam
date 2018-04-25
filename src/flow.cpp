#include "flow.h"
#include "flow_manager.h"

namespace nfv_exam {

int Flow::get_flow_node(int function_id, FlowNode &flow_node)
{
    FlowManager *flow_manager = FlowManager::get_instance();

    if (function_id > this->flow_nodes.size()) {
        warning_log("invalid function_id %d. chain length = %d", function_id, flow_nodes.size());
        return -1;
    }

    int flow_node_id = flow_nodes[function_id];
    if (flow_manager->get_flow_node(flow_node_id, flow_node) != 0) {
        warning_log("get flow node failed");
        return -1;
    }

    return 0;
}


}
