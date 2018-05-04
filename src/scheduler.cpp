#include "scheduler.h"
#include "log.h"

namespace nfv_exam {

int Scheduler::set_strategy(const std::string &strategy)
{
    if (strategy != "normal" && strategy != "vertical_only" && strategy != "horizontal_only") {
        warning_log("illegal strategy:%s", strategy.c_str());
        return -1;
    }
    this->strategy = strategy;
    return 0;
}

int Schedule::set_alpha(double alpha)
{
    this->alpha = alpha;
    return 0;
}

int Scheduler::init()
{
    this->search_physical_node_ptr = 0;

    PhysicalNodeManager *physical_node_manager(NULL);
    if ((physical_node_manager = PhysicalNodeManager::get_instance()) == NULL) {
        warning_log("get pn manager failed");
        return -1;
    }

    this->server_count = physical_node_manager->get_server_count();
    this->physical_nodes_total_bandwidth = physical_node_manager->get_total_bandwidth();

    return 0;
}

void Schedeler::search_physical_node_ptr_add_one()
{
    this->search_physical_node_ptr = (this->search_physical_node_ptr + 1) % this->server_count;
}

void Schedeler::search_physical_node_ptr_random_jump()
{
    this->search_physical_node_ptr = (this->search_physical_node_ptr + rand()) % this->server_count;
}

int Scheduler::handle_req_list(const std::vector<Req> &req_list, std::vector<bool> &req_accepted_flag)
{
    req_accepted_flag.clear();
    bool req_result = false;
    for (auto iter = req_list.begin(); iter != req_list.end(); ++iter) {
        if (this->strategy == "normal") {
            if (handle_req(*iter, req_result) != 0) {
                warning_log("handle req failed");
                return -1;
            }
        } else if (this->strategy == "vertical_only") {
            if (handle_req_v_only(*iter, req_result) != 0) {
                warning_log("handle req failed");
                return -1;
            }
        } else if (this->strategy == "horizontal_only") {
            if (handle_req_h_only(*iter, req_result) != 0) {
                warning_log("handle req failed");
                return -1;
            }
        } else {
            warning_log("illegal strategy");
            return -1;
        }
        req_accepted_flag.push_back(req_result);
    }

    return 0;
}

int Scheduler::create_new_flow(int flow_template_id, int chain_id, int lifetime, int &flow_id)
{
    ReqManager *req_manager(NULL);
    FlowManager *flow_manager(NULL);

    if ((req_manager = ReqManager::get_instance()) == NULL) {
        warning_log("get req_manager instance failed");
        return -1;
    }

    if ((flow_manager = FlowManager::get_instance()) == NULL) {
        warning_log("get flow_manager instance failed");
        return -1;
    }
    
    int flow_length(0);
    std::vector<int> node_cpu_cost;
    std::vector<int> node_memory_cost;
    int flow_bandwidth_cost(0);
    if (get_template_info(flow_template_id, length, node_cpu_cost, node_memory_cost, flow_bandwidth_cost) != 0) {
        warning_log("get template info failed");
        return -1;
    }
    
    if (flow_manager->create_a_flow(flow_length, chain_id, lifetime, flow_bandwidth_cost, 
                node_cpu_cost, node_memory_cost, flow_id) != 0) {
        warning_log("create a flow failed");
        return -1;
    }

    return 0;
}

int Scheduler::create_new_chain(int length, bool disable_scale_up_down = false, int &chain_id)
{
    ServiceChainManager *service_chain_manager(NULL);

    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get service_chain_manager instance failed");
        return -1;
    }

    if (service_chain_manager->create_a_chain(length, disable_scale_up_down, chain_id) != 0) {
        warning_log("create a chain failed");
        return -1;
    }

    return 0;
}

int Scheduler::place_first_flow_on_a_chain(int flow_id, int chain_id)
{
    ServiceChainManager *service_chain_manager(NULL);

    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get service_chain_manager instance failed");
        return -1;
    }

    ServiceChain *service_chain(NULL);

    if (service_chain_manager->place_first_flow_on_a_chain(int flow_id, int chain_id) != 0) {
        warning_log("place first flow on a chain failed");
        return -1;
    }
    
//change into a maybe better way
/*
    if (service_chain_manager->get_service_chain(chain_id, service_chain) != 0 || service_chain == NULL) {
        warning_log("get service chain failed");
        return -1;
    }

    if (service_chain->place_first_flow(int flow_id) != 0) {
        warning_log("place first flow on a chain failed");
        return -1;
    }
    */

    return 0;
}

int Scheduler::delete_a_flow(int flow_id)
{
    //require automatically delete chain
    
    FlowManager *flow_manager(NULL);
    if ((flow_manager = FlowManager::get_instance()) == NULL) {
        warning_log("get flow_manager instance failed");
        return -1;
    }

    if (flow_manager->delete_a_flow(flow_id) != 0) {
        warning_log("delete a flow failed");
        return -1;
    }

    return 0;
}

int Scheduler::place_new_chain_on_physical_nodes(int chain_id, bool &req_result)
{
    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get sc manager failed");
        return -1;
    }

    ServiceChain *service_chain(NULL);
    VnfInstance *vnf_instance(NULL);
    int vi_id(-1);

    if (service_chain_manager->get_service_chain(chain_id, service_chain) != 0) {
        warning_log("get service_chain failed");
        return -1;
    }

    int length = service_chain->get_length();

    VnfInstance *pre_vi(NULL);
    int pre_search_physical_node_ptr = this->search_physical_node_ptr;
    
    bool refused_flag = false;
    for (int i = 0; i < length && refused_flag == false; ++i) {
        if ((vi_id = service_chain->get_first_vi_id(i)) == -1) {
            warning_log("get first vi_id failed");
            return -1;
        }

        if (service_chain_manager->get_vnf_instance(vi_id, vnf_instance) != 0) {
            warning_log("get vi failed");
            return -1;
        }

        bool local_enough_flag = false;
        while(local_enough_flag == false) {
            if (this->search_physical_node_ptr == pre_search_physical_node_ptr) { //back to the beginning
                refused_flag = true;
                break;
            }

            if (settle_a_vnf_instance(vnf_instance, search_physical_node_ptr, service_chain->get_bandwidth_used(), 
                        pre_vi, NULL) != 0) {
                warning_log("settle vi failed");
                return -1;
            }

            if (calculate_a_vnf_instance_cost(vnf_instance, pre_vi, NULL, local_enough_flag) != 0) {
                warning_log("calc cost failed");
                return -1;
            }

            if (local_enough_flag == false) {
                if (remove_a_vnf_instance(vnf_instance, service_chain->get_bandwidth_used(), pre_vi, NULL) != 0) {
                    warning_log("remove vi failed");
                    return -1;
                }
            }

            search_physical_node_ptr_add_one();
        }
        
        pre_vi = vnf_instance;
    }

    //release resource if req_result == false
    if (refused_flag == true) {
        req_result = false;

        if (release_service_chain_when_rejected(service_chain) != 0) {
            warning_log("release service chain when rejuct failed");
            return -1;
        }

        search_physical_node_ptr_random_jump();
    } else {
        req_result = true;
    }

    return 0;
}

int Schedeler::release_service_chain_when_rejected(ServiceChain *service_chain)
{
    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get sc manager failed");
        return -1;
    }
    int length = service_chain->get_length();
    VnfInstance *vnf_instance(NULL), *next_vi(NULL);
    int vi_id(-1);

    for (int i = 0; i < length; ++i) {
        next_vi = NULL;
        int next_vi_id(-1);
        if (i < length-1) {
            if ((next_vi_id = service_chain->get_first_vi_id(i+1)) == -1) {
                warning_log("get first vi_id failed");
                return -1;
            }

            if (service_chain_manager->get_vnf_instance(next_vi_id, next_vi) != 0) {
                warning_log("get vi failed");
                return -1;
            }
        }

        if ((vi_id = service_chain->get_first_vi_id(i)) == -1) {
            warning_log("get first vi_id failed");
            return -1;
        }

        if (service_chain_manager->get_vnf_instance(vi_id, vnf_instance) != 0) {
            warning_log("get vi failed");
            return -1;
        }

        if (vnf_instance->is_settled() && 
                remove_a_vnf_instance(vnf_instance, service_chain->get_bandwidth_used(), NULL, next_vi) != 0) {
            warning_log("remove vi failed");
            return -1;
        }
    }

    return 0;
}

int Scheduler::settle_a_vnf_instance(VnfInstance *vnf_instance, int pn_id, int bandwidth_cost, 
        VnfInstance *pre_vi, VnfInstance *next_vi)
{
    if (vnf_instance->settle(pn_id) != 0) {
        warning_log("set vi location failed");
        return -1;
    }

    PhysicalNodeManager *physical_node_manager(NULL);
    if ((physical_node_manager = PhysicalNodeManager::get_instance()) == NULL) {
        warning_log("get pn manager instance failed");
        return -1;
    }

    int vi_cpu_cost = vnf_instance->get_cpu_cost();
    int vi_memory_cost = vnf_instance->get_memory_cost();

    if (physical_node_manager->assign_host_resource(pn_id, vi_cpu_cost, vi_memory_cost) != 0) {
        warning_log("assign resource failed");
        return -1;
    }

    if (pre_vi != NULL && pre_vi->is_settled()) {
        int pre_pn_id = pre_vi->get_location();
        if (physical_node_manager->assign_bandwidth(pre_pn_id, pn_id, bandwidth_cost) != 0) {
            warning_log("assign bandwidth_cost failed");
            return -1;
        }
    }

    if (next_vi != NULL && next_vi->is_settled()) {
        int next_pn_id = next_vi->get_location();
        if (physical_node_manager->assign_bandwidth(next_pn_id, pn_id, bandwidth_cost) != 0) {
            warning_log("assign bandwidth_cost failed");
            return -1;
        }
    }   

    return 0;
}

int Scheduler::calculate_a_vnf_instance_cost(VnfInstance *vnf_instance, 
        VnfInstance *pre_vi, VnfInstance *next_vi, bool &enough_flag)
{
    if (vnf_instance->is_settled() == false) {
        warning_log("can not calculate an unsettled vi");
        return -1;
    }

    enough_flag = true;

    int cpu_used(0), cpu(0);
    int memory_used(0), memory(0);
    int bandwidth_left(0);
    int pn_id = vnf_instance->get_location();
    
    PhysicalNodeManager *physical_node_manager(NULL);
    if ((physical_node_manager = PhysicalNodeManager::get_instance()) == NULL) {
        warning_log("get pn manager instance failed");
        return -1;
    }

    if (physical_node_manager->get_cpu_statistics(pn_id, cpu_used, cpu) != 0) {
        warning_log("get cpu statistics failed");
        return -1;
    }

    if (physical_node_manager->get_memory_statistics(pn_id, memory_used, memory) != 0) {
        warning_log("get memory statistics failed");
        return -1;
    }

    if (cpu_used > cpu || memory_used > memory) {
        enough_flag = false
    } 

    if (pre_vi != NULL && pre_vi->is_settled()) {
        int pre_pn_id = pre_vi->get_location();
        if (physical_node_manager->get_bandwidth_statistics(pre_pn_id, pn_id, bandwidth_left) != 0) {
            warning_log("get bandwidth_left failed");
            return -1;
        }
    }

    if (bandwidth_left < 0) {
        enough_flag = false;
    }

    if (next_vi != NULL && next_vi->is_settled()) {
        int next_pn_id = next_vi->get_location();
        if (physical_node_manager->get_bandwidth_statistics(next_pn_id, pn_id, bandwidth_left) != 0) {
            warning_log("get bandwidth_left failed");
            return -1;
        }
    }   

    if (bandwidth_left < 0) {
        enough_flag = false;
    }

    return 0;
}


int Scheduler::remove_a_vnf_instance(VnfInstance *vnf_instance, int bandwidth_cost, 
        VnfInstance *pre_vi, VnfInstance *next_vi)
{
    if (vnf_instance->is_settled() == false) {
        warning_log("can not remove a unsettled vi");
        return -1;
    }

    int pn_id = vnf_instance->get_location();

    PhysicalNodeManager *physical_node_manager(NULL);
    if ((physical_node_manager = PhysicalNodeManager::get_instance()) == NULL) {
        warning_log("get pn manager instance failed");
        return -1;
    }

    int vi_cpu_cost = vnf_instance->get_cpu_cost();
    int vi_memory_cost = vnf_instance->get_memory_cost();

    if (physical_node_manager->release_host_resource(pn_id, vi_cpu_cost, vi_memory_cost) != 0) {
        warning_log("release resource failed");
        return -1;
    }

    if (pre_vi != NULL && pre_vi->is_settled()) {
        int pre_pn_id = pre_vi->get_location();
        if (physical_node_manager->release_bandwidth(pre_pn_id, pn_id, bandwidth_cost) != 0) {
            warning_log("release bandwidth_cost failed");
            return -1;
        }
    }

    if (next_vi != NULL && next_vi->is_settled()) {
        int next_pn_id = next_vi->get_location();
        if (physical_node_manager->release_bandwidth(next_pn_id, pn_id, bandwidth_cost) != 0) {
            warning_log("release bandwidth_cost failed");
            return -1;
        }
    }   

    if (vnf_instance->remove() != 0) {
        warning_log("set vi location failed");
        return -1;
    }

    return 0;
}

int Schedeler::new_flow_on_new_chain_arrange(const Req &req, bool &req_result)
{
    int flow_id(-1), chain_id(-1);
    if (create_new_flow(req.get_flow_template_id(), req.get_chain_id(), req.get_lifetime(), flow_id) != 0) {
        warning_log("create new flow failed");
        return -1;
    }

    if (create_new_chain(req.get_length(), false, chain_id) != 0) {
        warning_log("create new chain failed");
        return -1;
    }

    if (place_first_flow_on_a_chain(flow_id, chain_id) != 0) {
        warning_log("place first flow on a chain failed");
        return -1;
    } 

    if (place_new_chain_on_physical_nodes(chain_id, req_result) != 0) {
        warning_log("first fit placement failed");
        return -1;
    }

    if (req_result == false) {
        if (delete_a_flow(flow_id) != 0) {
            warning_log("delete a flow failed");
            return -1;
        }
    }

    return 0;
}

int Scheduler::settle_a_flow_node(FlowNode *flow_node, VnfInstance *vnf_instance, int bandwidth_cost)
{
    if (flow_node->is_settled() == true) {
        warning_log("flow node has been settled, this function require a unsettled flow_node");
        return -1;
    }

    if (vnf_instance->is_settled() == false) {
        warning_log("vnf_instance has not been settled, this function require a settled vnf_instance");
        return -1;
    }
    
    if (settle_a_flow_v(flow_node, vnf_instance) != 0) {
        warning_log("settle a flow v failed");
        return -1;
    }

    if (flow_node->has_pre_node() == true) {
        FlowNode *pre_flow_node = flow_node->get_pre_node();
        if (pre_flow_node == NULL) {
            warning_log("get pre flow node failed");
            return -1;
        }

        if (pre_flow_node->is_settled() == true) {
            if (settle_a_flow_link(pre_flow_node, flow_node, bandwidth_cost) != 0) {
                warning_log("settle a flow link failed");
                return -1;
            }
        }
    }

    if (flow_node->has_next_node() == true) {
        FlowNode *next_flow_node = flow_node->get_next_node();
        if (next_flow_node == NULL) {
            warning_log("get next flow node failed");
            return -1;
        }

        if (next_flow_node->is_settled() == true) {
            if (settle_a_flow_link(next_flow_node, flow_node, bandwidth_cost) != 0) {
                warning_log("settle a flow link failed");
                return -1;
            }
        }
    }

    return 0;
}

int Scheduler::settle_a_flow_v(FlowNode *flow_node, VnfInstance *vnf_instance)
{
    if (vnf_instance->is_settled() == false) {
        warning_log("vnf_instance has not been settled, this function require a settled vnf_instance");
        return -1;
    }

    if (flow_node->settle(vnf_instance->get_id()) != 0) {
        warning_log("flow node settle failed");
        return -1;
    }

    if (vnf_instance->add_settled_flow_node(flow_node->get_id()) != 0) {
        warning_log("vi add settled flow node failed");
        return -1;
    }

    //TODO:update vnf instance resource info

    if (vnf_instance->disable_scale_up_down == false) {
        int pn_id = vnf_instance->get_location();
        PhysicalNodeManager *physical_node_manager(NULL);
        if ((physical_node_manager = PhysicalNodeManager::get_instance()) == NULL) {
            warning_log("get pn manager failed");
            return -1;
        }

        int cpu_cost = flow_node->get_cpu_cost();
        int memory_cost = flow_node->get_memory_cost();

        if (physical_node_manager->assign_host_resource(pn_id, cpu_cost, memory_cost) != 0) {
            warning_log("assign resource failed");
            return -1;
        }
    }

    return 0;
}

int Scheduler::settle_a_flow_link(FlowNode *flow_node_1, FlowNode *flow_node_2, int bandwidth_cost)
{
    int vi_id_1(-1), vi_id_2(-1);
    VnfInstance *vi_1(NULL), *vi_2(NULL);
    int pn_id_1(-1), pn_id_2(-1);

    if ((vi_id_1 = flow_node_1->get_location()) == -1) {
        warning_log("flow node 1 unsettled");
        return -1;
    }

    if ((vi_id_2 = flow_node_2->get_location()) == -1) {
        warning_log("flow node 2 unsettled");
        return -1;
    }

    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get service_chain_manager failed");
        return -1;
    }

    if (service_chain_manager->get_vnf_instance(vi_id_1, vi_1) != 0) {
        warning_log("get vi_1 failed");
        return -1;
    }

    if (service_chain_manager->get_vnf_instance(vi_id_2, vi_2) != 0) {
        warning_log("get vi_2 failed");
        return -1;
    }

    if ((pn_id_1 = vi_1->get_location()) == -1) {
        warning_log("vi_1 unsettled");
        return -1;
    }

    if ((pn_id_2 = vi_2->get_location()) == -1) {
        warning_log("vi_2 unsettled");
        return -1;
    }

    PhysicalNodeManager *physical_node_manager(NULL);
    if ((physical_node_manager = PhysicalNodeManager::get_instance()) == NULL) {
        warning_log("get PhysicalNodeManager failed");
        return -1;
    }

    if (physical_node_manager->assign_bandwidth(pn_id_1, pn_id_2, bandwidth_cost) != 0) {
        warning_log("assign bandwidth on physical node failed");
        return -1;
    }
    
    return 0;
}

int Scheduler::calculate_a_flow_node_cost(FlowNode *flow_node, bool &enough_flag, double &cost_result)
{
    double host_cost(0), bandwidth_cost(0);
    bool host_enough_flag(false), bandwidth_enough_flag(false);
    if (calculate_a_flow_v_cost(flow_node, host_enough_flag, host_cost) != 0) {
        warning_log("calculate_a_flow_v_cost failed");
        return -1;
    }

    if (calculate_a_flow_node_links_cost(flow_node, bandwidth_enough_flag, bandwidth_cost) != 0) {
        warning_log("calculate_a_flow_link_cost failed");
        return -1;
    }

    cost_result = host_cost + this->alpha * bandwidth_cost;
    if (host_enough_flag == true && bandwidth_enough_flag == true) {
        enough_flag = true;
    } else {
        enough_flag = false;
    }

    return 0;
}

int Scheduler::calculate_a_flow_v_cost(FlowNode *flow_node, bool &enough_flag, double &host_cost_result)
{
    int flow_node_cpu_cost = flow_node->get_cpu_cost();
    int flow_node_memory_cost = flow_node->get_memory_cost();

    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get service_chain_manager failed");
        return -1;
    }

    PhysicalNodeManager *physical_node_manager(NULL);
    if ((physical_node_manager = PhysicalNodeManager::get_instance()) == NULL) {
        warning_log("get PhysicalNodeManager failed");
        return -1;
    }

    int vi_id(-1), pn_id(-1);
    VnfInstance *vnf_instance(NULL);

    if ((vi_id = flow_node->get_location()) == -1) {
        warning_log("unsettled flow node");
        return -1;
    }

    if (service_chain_manager->get_vnf_instance(vi_id, vnf_instance) != 0) {
        warning_log("get vnf_instance failed");
        return -1;
    }

    if ((pn_id = vnf_instance->get_location()) == -1) {
        warning_log("unsettled vnf_instance");
        return -1;
    }

    int pn_cpu_used(0), pn_cpu(0);
    int pn_memory_used(0), pn_memory(0);

    if (physical_node_manager->get_cpu_statistics(pn_id, pn_cpu_used, pn_cpu) != 0) {
        warning_log("get cpu statistics failed");
        return -1;
    }

    if (physical_node_manager->get_memory_statistics(pn_id, pn_memory_used, pn_memory) != 0) {
        warning_log("get memory statistics failed");
        return -1;
    }

    if (pn_cpu_used > pn_cpu || pn_memory_used > pn_memory) {
        enough_flag = false;
    } else {
        enough_flag = true;
    }

    double pre_host_cost = fmax((double)(pn_cpu_used - flow_node_cpu_cost) / pn_cpu, 
            (double)(pn_memory_used - flow_node_memory_cost) / pn_memory);
    double cur_host_cost = fmax((double)pn_cpu_used / pn_cpu, (double)pn_memory_used / pn_memory);
    host_cost_result = cur_host_cost - pre_host_cost;

    return 0;
}

int Scheduler::calculate_a_flow_link_cost(FlowNode *flow_node_1, FlowNode *flow_node_2, bool &enough_flag, int &delta_bandwidth)
{
    int vi_id_1(-1), vi_id_2(-1);
    VnfInstance *vi_1(NULL), *vi_2(NULL);
    int pn_id_1(-1), pn_id_2(-1);

    if ((vi_id_1 = flow_node_1->get_location()) == -1) {
        warning_log("flow node 1 unsettled");
        return -1;
    }

    if ((vi_id_2 = flow_node_2->get_location()) == -1) {
        warning_log("flow node 2 unsettled");
        return -1;
    }

    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get service_chain_manager failed");
        return -1;
    }

    if (service_chain_manager->get_vnf_instance(vi_id_1, vi_1) != 0) {
        warning_log("get vi_1 failed");
        return -1;
    }

    if (service_chain_manager->get_vnf_instance(vi_id_2, vi_2) != 0) {
        warning_log("get vi_2 failed");
        return -1;
    }

    if ((pn_id_1 = vi_1->get_location()) == -1) {
        warning_log("vi_1 unsettled");
        return -1;
    }

    if ((pn_id_2 = vi_2->get_location()) == -1) {
        warning_log("vi_2 unsettled");
        return -1;
    }

    PhysicalNodeManager *physical_node_manager(NULL);
    if ((physical_node_manager = PhysicalNodeManager::get_instance()) == NULL) {
        warning_log("get PhysicalNodeManager failed");
        return -1;
    }

    int bandwidth_left(0);
    if (physical_node_manager->get_bandwidth_statistics(pn_id_1, pn_id_2, bandwidth_left) != 0) {
        warning_log("get_bandwidth_statistics failed");
        return -1;
    }

    if (bandwidth_left < 0) {
        enough_flag = false;
    } else {
        enough_flag = true;
    }

    int hop_count(0);
    if (physical_node_manager->get_bandwidth_hop_count(pn_id_1, pn_id_2, hop_count) != 0) {
        warning_log("get_bandwidth_hop_count failed");
        return -1;
    }

    int flow_bandwidth(0);
    int flow_id = flow_node->get_flow_id();
    Flow *flow(NULL);
    if (service_chain_manager->get_flow(flow_id, flow) != 0) {
        warning_log("get flow failed");
        return -1;
    }
    flow_bandwidth = flow->get_flow_bandwidth();

    delta_bandwidth = flow_bandwidth * hop_count;

    return 0;
}

int Scheduler::calculate_a_flow_node_links_cost(FlowNode *flow_node, bool &enough_flag, double &bandwidth_cost)
{
    PhysicalNodeManager *physical_node_manager(NULL);
    if ((physical_node_manager = PhysicalNodeManager::get_instance()) == NULL) {
        warning_log("get PhysicalNodeManager failed");
        return -1;
    }

    int delta_bandwidth(0);
    bool local_enough_flag_1(true), local_enough_flag_2(true);
    int delta_bandwidth_1(0), delta_bandwidth_2(0);
    if (flow_node->has_pre_node() && flow_node->get_pre_node()->is_settled()) {
        if (calculate_a_flow_link_cost(flow_node->get_pre_node(), flow_node, local_enough_flag_1, delta_bandwidth_1) != 0) {
            warning_log("calculate_a_flow_link_cost failed");
            return -1;
        }
    }
    if (flow_node->has_next_node() && flow_node->get_next_node()->is_settled()) {
        if (calculate_a_flow_link_cost(flow_node->get_next_node(), flow_node, local_enough_flag_2, delta_bandwidth_2) != 0) {
            warning_log("calculate_a_flow_link_cost failed");
            return -1;
        }
    }

    if (local_enough_flag_1 == true && local_enough_flag_2 == true) {
        enough_flag = true;
    } else {
        enough_flag = false;
    }

    delta_bandwidth = delta_bandwidth_1 + delta_bandwidth_2;
    bandwidth_cost = (double)delta_bandwidth / this->physical_nodes_total_bandwidth;

    return 0;
}

int Scheduler::remove_a_flow_node(FlowNode *flow_node, VnfInstance *vnf_instance, int bandwidth_cost)
{
    if (flow_node->is_settled() == false) {
        warning_log("flow node has been settled, this function require a settled flow_node");
        return -1;
    }

    if (vnf_instance->is_settled() == false) {
        warning_log("vnf_instance has not been settled, this function require a settled vnf_instance");
        return -1;
    }

    if (flow_node->get_location() != vnf_instance->get_id()) {
        warning_log("id mismatch, flow node is not placed in this vnf instance");
        return -1;
    }

    if (flow_node->has_pre_node() == true) {
        FlowNode *pre_flow_node = flow_node->get_pre_node();
        if (pre_flow_node == NULL) {
            warning_log("get pre flow node failed");
            return -1;
        }

        if (pre_flow_node->is_settled() == true) {
            if (remove_a_flow_link(pre_flow_node, flow_node, bandwidth_cost) != 0) {
                warning_log("remove a flow link failed");
                return -1;
            }
        }
    }

    if (flow_node->has_next_node() == true) {
        FlowNode *next_flow_node = flow_node->get_next_node();
        if (next_flow_node == NULL) {
            warning_log("get next flow node failed");
            return -1;
        }

        if (next_flow_node->is_settled() == true) {
            if (remove_a_flow_link(next_flow_node, flow_node, bandwidth_cost) != 0) {
                warning_log("remove a flow link failed");
                return -1;
            }
        }
    }

    if (settle_a_flow_v(flow_node, vnf_instance) != 0) {
        warning_log("settle a flow v failed");
        return -1;
    }

    return 0;
}

int Scheduler::remove_a_flow_v(FlowNode *flow_node, VnfInstance *vnf_instance)
{
    if (vnf_instance->is_settled() == false) {
        warning_log("vnf_instance has not been settled, this function require a settled vnf_instance");
        return -1;
    }

    
    //TODO:update vnf instance resource info

    if (vnf_instance->disable_scale_up_down == false) {
        int pn_id = vnf_instance->get_location();
        PhysicalNodeManager *physical_node_manager(NULL);
        if ((physical_node_manager = PhysicalNodeManager::get_instance()) == NULL) {
            warning_log("get pn manager failed");
            return -1;
        }

        int cpu_cost = flow_node->get_cpu_cost();
        int memory_cost = flow_node->get_memory_cost();

        if (physical_node_manager->release_host_resource(pn_id, cpu_cost, memory_cost) != 0) {
            warning_log("release resource failed");
            return -1;
        }
    }

    if (vnf_instance->remove_settled_flow_node(flow_node->get_id()) != 0) {
        warning_log("vi remove settled flow node failed");
        return -1;
    }

    if (flow_node->remove() != 0) {
        warning_log("flow node remove failed");
        return -1;
    }

    return 0;
}

int Scheduler::remove_a_flow_link(FlowNode *flow_node_1, FlowNode *flow_node_2, int bandwidth_cost)
{
    int vi_id_1(-1), vi_id_2(-1);
    VnfInstance *vi_1(NULL), *vi_2(NULL);
    int pn_id_1(-1), pn_id_2(-1);

    if ((vi_id_1 = flow_node_1->get_location()) == -1) {
        warning_log("flow node 1 unsettled");
        return -1;
    }

    if ((vi_id_2 = flow_node_2->get_location()) == -1) {
        warning_log("flow node 2 unsettled");
        return -1;
    }

    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get service_chain_manager failed");
        return -1;
    }

    if (service_chain_manager->get_vnf_instance(vi_id_1, vi_1) != 0) {
        warning_log("get vi_1 failed");
        return -1;
    }

    if (service_chain_manager->get_vnf_instance(vi_id_2, vi_2) != 0) {
        warning_log("get vi_2 failed");
        return -1;
    }

    if ((pn_id_1 = vi_1->get_location()) == -1) {
        warning_log("vi_1 unsettled");
        return -1;
    }

    if ((pn_id_2 = vi_2->get_location()) == -1) {
        warning_log("vi_2 unsettled");
        return -1;
    }

    PhysicalNodeManager *physical_node_manager(NULL);
    if ((physical_node_manager = PhysicalNodeManager::get_instance()) == NULL) {
        warning_log("get PhysicalNodeManager failed");
        return -1;
    }

    if (physical_node_manager->release_bandwidth(pn_id_1, pn_id_2, bandwidth_cost) != 0) {
        warning_log("release bandwidth on physical node failed");
        return -1;
    }
}


//-------------vertical and horizontal---------------
int Scheduler::handle_req(const Req &req, bool &req_result)
{
    req_result = false;
    if (req.req_type == "new" && req.chain_id = -1) {
        if (new_flow_on_new_chain_arrange(req, req_result) != 0) {
            warning_log("new flow on new chain arrange failed");
            return -1;
        }
    } else {
        if (sugoi_arrange(const Req &req, bool &req_result) != 0) {
            warning_log("sugoi arrange failed");
            return -1;
        }
    }

    return 0;
}

int Scheduler::sugoi_arrange(const Req &req, bool &req_result)
{

}

int Schedule::route()
{

}

int Scheduler::migration()
{

}

int Scheduler::scale_out()
{

}


//-------------vertical only---------------


//-------------horizontal only---------------
   

}
