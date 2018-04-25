#include "flow_manager.h"
#include "log.h"

namespace nfv_exam {

FlowManager::~FlowManager()
{
    for (std::map<int, Flow *>::iterator itr = flow_pool.begin();
            itr != flow_pool.end(); ++itr) {
        if (itr->second != NULL) {
            delete itr->second;
        }
    }

    for (std::map<int, FlowNode *>::iterator itr = flow_node_pool.begin();
            itr != flow_node_pool.end(); ++itr) {
        if (itr->second != NULL) {
            delete itr->second;
        }
    }
}

FlowManager *FlowManager::get_instance()
{
    static FlowManager *instance(NULL);

    if (instance == NULL) {
        instance = new FlowManager;
    }
    return instance;
}

int FlowManager::get_flow(int flow_id, Flow &flow)
{
    if (flow_pool.find(flow_id) != flow_pool.end()) {
        flow = *flow_pool[flow_id];
        return 0;
    } else {
        return -1;
    }
}

int FlowManager::get_flow_node(int flow_node_id, FlowNode &flow_node)
{
    if (flow_node_pool.find(flow_node_id) != flow_node_pool.end()) {
        flow_node = *flow_node_pool[flow_node_id];
        return 0;
    } else {
        return -1;
    }
}


}
