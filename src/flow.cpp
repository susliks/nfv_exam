#include "flow.h"
//#include "flow_manager.h"

namespace nfv_exam {

//int Flow::get_flow_node(int function_id, FlowNode *flow_node)
//{
//    FlowManager *flow_manager = FlowManager::get_instance();
//
//    if (function_id > this->flow_nodes.size()) {
//        warning_log("invalid function_id %d. chain length = %d", function_id, flow_nodes.size());
//        return -1;
//    }
//
//    int flow_node_id = flow_nodes[function_id];
//    if (flow_manager->get_flow_node(flow_node_id, flow_node) != 0) {
//        warning_log("get flow node failed");
//        return -1;
//    }
//    if (flow_node == NULL) {
//        warning_log("get flow_node failed, NULL ptr");
//        return -1;
//    }
//
//    return 0;
//}

int Flow::get_id()
{
    return this->id;
}

int Flow::get_chain_id()
{
    return this->chain_id;
}

int Flow::get_lifetime_left()
{
    return this->lifetime_left;
}

int Flow::get_length()
{
    return this->length;
}

int Flow::get_flow_bandwidth()
{
    return this->flow_bandwidth;
}

const std::vector<int> &Flow::get_flow_nodes()
{
    return this->flow_nodes;
}

void Flow::set_id(int id)
{
    this->id = id;
}

void Flow::set_chain_id(int chain_id)
{
    this->chain_id = chain_id;
}

void Flow::set_lifetime_left(int lifetime)
{
    this->lifetime_left = lifetime;
}

void Flow::set_length(int length)
{
    this->length = length;
}

void Flow::set_flow_bandwidth(int flow_bandwidth)
{
    this->flow_bandwidth = flow_bandwidth;
}

void Flow::set_flow_nodes(const std::vector<int> &flow_nodes)
{
    this->flow_nodes = flow_nodes;
}

int Flow::get_flow_node_id(int func_id)
{
    if (func_id >= this->flow_nodes.size()) {
        warning_log("illegal func_id. func_id=%d, flow_nodes.size=%d", func_id, this->flow_nodes.size());
        return -1;
    }
    return this->flow_nodes[func_id];
}

int Flow::aging()
{
    this->lifetime_left -= 1;
    if (this->lifetime_left == 0) {
        return 1;
    } else {
        return 0;
    }
}

const std::string Flow::to_string()
{
    std::string result;
    result += std::string("id:") + std::to_string(id) + std::string(" ");
    result += std::string("chain_id:") + std::to_string(chain_id) + std::string(" ");
    result += std::string("lifetime_left:") + std::to_string(lifetime_left) + std::string(" ");
    result += std::string("length:") + std::to_string(length) + std::string(" ");
    result += std::string("flow_bandwidth:") + std::to_string(flow_bandwidth) + std::string(" ");
    return result;
}

}
