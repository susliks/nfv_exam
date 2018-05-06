#include "flow_node.h"
#include "log.h"

namespace nfv_exam {

int FlowNode::get_id()
{
    return this->id;
}

int FlowNode::get_flow_id()
{
    return this->flow_id;
}

int FlowNode::get_function_id()
{
    return this->function_id;
}

int FlowNode::settle(int vi_id)
{
    if (this->flow_node_location != -1) {
        warning_log("try to put flow_node %d at vi_id = %d, while it is on vi_id = %d", this->id, vi_id, this->flow_node_location);
        return -1;
    } else {
        this->flow_node_location = vi_id;
        return 0;
    }
}

int FlowNode::remove()
{
    if (this->flow_node_location == -1) {
        warning_log("try to remove a unplaced flow_node %d", this->id);
        return -1;
    } else {
        this->flow_node_location = -1;
        return 0;
    }
}

bool FlowNode::is_settled()
{
    if (this->flow_node_location != -1) {
        return true;
    } else {
        return false;
    }
}

bool FlowNode::has_pre_node()
{
    if (this->pre_flow_node_id != -1) {
        return true;
    } else {
        return false;
    }
}

bool FlowNode::has_next_node()
{
    if (this->next_flow_node_id != -1) {
        return true;
    } else {
        return false;
    }
}

FlowNode *get_pre_node()
{
    FlowManager *flow_manager(NULL);
    if ((flow_manager = FlowManager::get_instance()) == NULL) {
        warning_log("get flow manager failed");
        return NULL;
    }

    FlowNode *pre_node(NULL);
    if (flow_manager->get_flow_node(this->pre_flow_node_id, pre_node) != 0) {
        warning_log("get pre flow node failed");
        return NULL;
    }

    return pre_node;
}

FlowNode *get_next_node()
{
    FlowManager *flow_manager(NULL);
    if ((flow_manager = FlowManager::get_instance()) == NULL) {
        warning_log("get flow manager failed");
        return NULL;
    }

    FlowNode *next_node(NULL);
    if (flow_manager->get_flow_node(this->next_flow_node_id, next_node) != 0) {
        warning_log("get next flow node failed");
        return NULL;
    }

    return next_node;
}

int FlowNode::get_pre_node_id()
{
    if (has_pre_node() == true) {
        return this->pre_flow_node_id;
    } else {
        return -1;
    }
}

int FlowNode::get_next_node_id()
{
    if (has_next_node() == true) {
        return this->next_flow_node_id;
    } else {
        return -1;
    }
}

int FlowNode::get_location()
{
    if (this->flow_node_location == -1) {
        warning_log("get location while not located");
        return -1;
    } else {
        return this->flow_node_location;
    }
}

int FlowNode::init(int id, int flow_id, int function_id, int cpu_cost, int memory_cost)
{
    this->id = id;
    this->flow_id = flow_id;
    this->function_id = function_id;
    this->flow_node_location = -1;
    this->cpu_cost = cpu_cost; 
    this->memory_cost = memory_cost;
    this->pre_flow_node_id = -1;
    this->next_flow_node_id = -1;
    return 0;
}

int FlowNode::set_pre_flow_node_id(int pre_flow_node_id)
{
    this->pre_flow_node_id = pre_flow_node_id;
    return 0;
}

int FlowNode::set_next_flow_node_id(int next_flow_node_id)
{
    this->next_flow_node_id = next_flow_node_id;
    return 0;
}

int FlowNode::get_cpu_cost()
{
    return this->cpu_cost;
}

int FlowNode::get_memory_cost()
{
    return this->memory_cost;
}

int FlowNode::adjust_cost(int new_cpu_cost, int new_memory_cost)
{
    this->cpu_cost = new_cpu_cost;
    this->memory_cost = new_memory_cost;
    return 0;
}

int FlowNode::get_flow_bandwidth();
{
    FlowManager *flow_manager(NULL);
    if ((flow_manager = FlowManager::get_instance()) == NULL) {
        warning_log("get instance failed");
        return -1;
    }

    Flow *flow(NULL);
    if (flow_manager->get_flow(this->flow_id, flow) != 0) {
        warning_log("get flow failed");
        return -1;
    }

    return flow->get_flow_bandwidth();
}

}

