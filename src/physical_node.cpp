#include "physical_node.h"
#include "log.h"

namespace nfv_exam {

PhysicalNode::PhysicalNode(int id, int depth, int cpu, int memory, int up_bandwidth)
{
    this->id = id;
    this->depth = depth;
    this->cpu = cpu;
    this->memory = memory;
    this->up_bandwidth = up_bandwidth;

    this->parent_id = -1;
    this->cpu_used  = this->memory_used = this->up_bandwidth_used = 0;
}

int PhysicalNode::set_parent(int parent_id)
{
    this->parent_id = parent_id;
    return 0;
}

int PhysicalNode::add_son(int son_id)
{
    this->son_id.push_back(son_id);
    return 0;
}

int PhysicalNode::get_host_cost(double &host_cost)
{
    host_cost = fmax((double)cpu_used / cpu, (double)memory_used / memory);
    return 0;
}

int PhysicalNode::get_pre_host_cost(int cur_cpu_cost, int cur_memory_cost, double &host_cost)
{
    if (cur_cpu_cost > cpu_used || cur_memory_cost > cpu_used) {
        warning_log("resource error when sub some cpu or memory");
        return -1;
    }

    host_cost = fmax((double)(cpu_used - cur_cpu_cost) / cpu, (double)(memory_used - cur_memory_cost) / memory);
    return 0;
}

int PhysicalNode::get_up_bandwidth()
{
    return this->up_bandwidth;
}

int PhysicalNode::get_parent_id()
{
    return this->parent_id;
}

int PhysicalNode::get_depth()
{
    return this->depth;
}

int PhysicalNode::get_cpu_used()
{
    return this->cpu_used;
}

int PhysicalNode::get_memory_used()
{
    return this->memory_used;
}

int PhysicalNode::get_up_bandwidth_used()
{
    return this->up_bandwidth_used;
}

int PhysicalNode::get_total_cpu_available()
{
    return this->cpu;
}

int PhysicalNode::get_total_memory_available()
{
    return this->memory;
}

int PhysicalNode::get_total_bandwidth_available()
{
    return this->up_bandwidth;
}

int PhysicalNode::assign_bandwidth(int bandwidth_cost)
{
    this->up_bandwidth_used += bandwidth_cost;
    return 0;
}

int PhysicalNode::release_bandwidth(int bandwidth_cost)
{
    if (bandwidth_cost > this->up_bandwidth_used) {
        warning_log("release more than used. node_id = %d, bandwidth_used = %d, want to release bandwidth = %d",
                this->id, this->up_bandwidth_used, bandwidth_cost);
        return -1;
    }
    this->up_bandwidth_used -= bandwidth_cost;
    return 0;
}

int PhysicalNode::assign_host_resource(int cpu_cost, int memory_cost)
{
    this->cpu_used += cpu_cost;
    this->memory_used += memory_cost;
    return 0;
}

int PhysicalNode::release_host_resource(int cpu_cost, int memory_cost)
{
    if (cpu_cost > this->cpu_used) {
        warning_log("release more than used. node_id = %d, cpu_used = %d, want to release cpu = %d",
                this->id, this->cpu_used, cpu_cost);
        return -1;
    }
    if (memory_cost > this->memory_used) {
        warning_log("release more than used. node_id = %d, memory_used = %d, want to release memory = %d",
                this->id, this->memory_used, memory_cost);
        return -1;
    }
    this->cpu_used -= cpu_cost;
    this->memory_used -= memory_cost;
    return 0;
}

}
