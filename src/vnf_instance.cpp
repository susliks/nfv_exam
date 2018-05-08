#include "log.h"
#include "vnf_instance.h"

namespace nfv_exam {

int VnfInstance::init(int id, int location, bool disable_scale_up_down, int vi_cpu_used, 
            int vi_memory_used, int vi_cpu_cost, int vi_memory_cost)
{
    this->id = id;
    this->location = location;
    this->disable_scale_up_down = disable_scale_up_down;
    this->vi_cpu_used = vi_cpu_used;
    this->vi_memory_used = vi_memory_used;
    this->vi_cpu_cost = vi_cpu_cost;
    this->vi_memory_cost = vi_memory_cost;

    this->pre_vi_id.clear();
    this->next_vi_id.clear();
    settled_flow_nodes.clear();

    return 0;
}

int VnfInstance::set_vi_resource_used(int cpu_used, int memory_used)
{
    if (this->disable_scale_up_down == true) {
        if (cpu_used > this->vi_cpu_cost || memory_used > this->vi_memory_cost) {
            warning_log("illegal set, cpu_used=%d, memory_used=%d", cpu_used, memory_used);
            return -1;
        }
    }
    this->vi_cpu_used = cpu_used;
    this->vi_memory_used = memory_used;

    if (this->disable_scale_up_down == false) {
        this->vi_cpu_cost = this->vi_cpu_used;
        this->vi_memory_cost = this->vi_memory_used;
    }

    return 0;
}

int VnfInstance::remove_pre_vi_id(int pre_vi_id)
{
    for (auto iter = this->pre_vi_id.begin(); iter != this->pre_vi_id.end();) {
        if (*iter == pre_vi_id) {
            iter = this->pre_vi_id.erase(iter);
            return 0;
        } else {
            ++iter;
        }
    }
    warning_log("not existed pre_vi_id: %d", pre_vi_id);
    return -1;
}

int VnfInstance::remove_next_vi_id(int next_vi_id)
{
    for (auto iter = this->next_vi_id.begin(); iter != this->next_vi_id.end();) {
        if (*iter == next_vi_id) {
            iter = this->next_vi_id.erase(iter);
            return 0;
        } else {
            ++iter;
        }
    }
    warning_log("not existed next_vi_id: %d", next_vi_id);
    return -1;
}

int VnfInstance::add_pre_vi_id(int pre_vi_id)
{
    for (auto iter = this->pre_vi_id.begin(); iter != this->pre_vi_id.end(); ++iter) {
        if (*iter == pre_vi_id) {
            warning_log("existed pre_vi_id: %d", pre_vi_id);
            return -1;
        }
    }
    this->pre_vi_id.push_back(pre_vi_id);
    return 0;
}

int VnfInstance::add_next_vi_id(int next_vi_id)
{
    for (auto iter = this->next_vi_id.begin(); iter != this->next_vi_id.end(); ++iter) {
        if (*iter == next_vi_id) {
            warning_log("existed next_vi_id: %d", next_vi_id);
            return -1;
        }
    }
    this->next_vi_id.push_back(next_vi_id);
    return 0;
}

int VnfInstance::add_settled_flow_node(int flow_node_id)
{
    if (this->settled_flow_nodes.find(flow_node_id) != this->settled_flow_nodes.end()) {
        warning_log("existed flow node id: %d", flow_node_id);
        return -1;
    }

    this->settled_flow_nodes.insert(flow_node_id);
    return 0;
}

int VnfInstance::remove_settled_flow_node(int flow_node_id)
{
    if (this->settled_flow_nodes.find(flow_node_id) == this->settled_flow_nodes.end()) {
        warning_log("not exist flow node id: %d", flow_node_id);
        return -1;
    }

    this->settled_flow_nodes.erase(flow_node_id);
    return 0;
}

bool VnfInstance::has_settled_flow_node()
{
    return this->settled_flow_nodes.size() != 0;
}

int VnfInstance::get_id()
{
    return this->id;
}

int VnfInstance::settle(int pn_id)
{
    if (this->location != -1) {
        warning_log("already settled");
        return -1;
    }
    this->location = pn_id;
    return 0;
}

int VnfInstance::remove()
{
    if (this->location == -1) {
        warning_log("already removed");
        return -1;
    }
    this->location = -1;
    return 0;
}

bool VnfInstance::get_disable_scale_up_down()
{
    return this->disable_scale_up_down;
}

int VnfInstance::get_cpu_used()
{
    return this->vi_cpu_used;
}

int VnfInstance::get_memory_used()
{
    return this->vi_memory_used;
}

int VnfInstance::get_cpu_cost()
{
    return this->vi_cpu_cost;
}

int VnfInstance::get_memory_cost()
{
    return this->vi_memory_cost;
}

int VnfInstance::get_location()
{
    return this->location;
}

bool VnfInstance::is_settled()
{
    return this->location != -1;
}

int VnfInstance::set_vi_resource_cost(int vi_cpu_cost, int vi_memory_cost)
{
    if (this->disable_scale_up_down != true) {
        warning_log("this function only for disable_scale_up_down==true");
        return -1;
    }

    this->vi_cpu_cost = vi_cpu_cost;
    this->vi_memory_cost = vi_memory_cost;

    return 0;
}

int VnfInstance::get_settled_flow_node(std::vector<int> &settled_flow_nodes)
{
    settled_flow_nodes.clear();
    for (auto iter = this->settled_flow_nodes.begin(); iter != this->settled_flow_nodes.end(); ++iter) {
        settled_flow_nodes.push_back(*iter);
    }
    return 0;
}

const std::string VnfInstance::to_string()
{
    std::string result;
    result += std::string("id:") + std::to_string(id) + std::string(" ");
    result += std::string("location:") + std::to_string(location) + std::string(" ");
    result += std::string("pre_vi_id:");
    for (auto iter = pre_vi_id.begin(); iter != pre_vi_id.end(); ++iter) {
        result += std::to_string(*iter) + std::string(" ");
    }
    result += std::string(".");
    result += std::string("next_vi_id:");
    for (auto iter = next_vi_id.begin(); iter != next_vi_id.end(); ++iter) {
        result += std::to_string(*iter) + std::string(" ");
    }
    result += std::string(".");
    if (disable_scale_up_down == true) {
        result += std::string(" true ");
    } else {
        result += std::string(" false ");
    }

    result += std::string("cpu_used:") + std::to_string(vi_cpu_used) + std::string(" ");
    result += std::string("memory_used:") + std::to_string(vi_memory_used) + std::string(" ");
    result += std::string("cpu_cost:") + std::to_string(vi_cpu_cost) + std::string(" ");
    result += std::string("memory_cost:") + std::to_string(vi_memory_cost) + std::string(" ");

    return result;
}

int VnfInstance::assign_vi_resource(int cpu_used, int memory_used)
{
    this->vi_cpu_used += cpu_used;
    this->vi_memory_used += memory_used;
    return 0;
}

int VnfInstance::release_vi_resource(int cpu_used, int memory_used)
{
    if (cpu_used > this->vi_cpu_used) {
        warning_log("release more than used. vi_id = %d, cpu_used = %d, want to release cpu = %d",
                this->id, this->vi_cpu_used, cpu_used);
        return -1;
    }
    if (memory_used > this->vi_memory_used) {
        warning_log("release more than used. vi_id = %d, memory_used = %d, want to release memory = %d",
                this->id, this->vi_memory_used, memory_used);
        return -1;
    }
    this->vi_cpu_used -= cpu_used;
    this->vi_memory_used -= memory_used;
    return 0;
}





}
