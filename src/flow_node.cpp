#include "flow_node.h"
#include "log.h"

namespace nfv_exam {

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

int FlowNode::get_pre_node()
{
    if (has_pre_node() == true) {
        return this->pre_flow_node_id;
    } else {
        return -1;
    }
}

int FlowNode::get_next_node()
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

}

