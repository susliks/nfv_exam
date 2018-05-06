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

int Scheduler::set_alpha(double alpha)
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
    this->server_cpu = physical_node_manager->get_server_cpu();
    this->server_memory = physical_node_manager->get_server_memory();

    this->place_vnf_shuffle_max_time = 3; //TODO: magic number

    return 0;
}

void Scheduler::search_physical_node_ptr_add_one()
{
    this->search_physical_node_ptr = (this->search_physical_node_ptr + 1) % this->server_count;
}

void Scheduler::search_physical_node_ptr_random_jump()
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
    if (get_template_info(flow_template_id, flow_length, node_cpu_cost, node_memory_cost, flow_bandwidth_cost) != 0) {
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

    if (service_chain_manager->place_first_flow_on_an_unsettled_chain(int flow_id, int chain_id) != 0) {
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

int Scheduler::release_service_chain_when_rejected(ServiceChain *service_chain)
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

int Scheduler::new_flow_on_new_chain_arrange(const Req &req, bool &req_result)
{
    int flow_id(-1), chain_id(-1);
    if (create_new_flow(req.get_flow_template_id(), req.get_chain_id(), req.get_lifetime(), flow_id) != 0) {
        warning_log("create new flow failed");
        return -1;
    }

    if (this->strategy != std::string("horizontal_only")) {
        if (create_new_chain(req.get_length(), false, chain_id) != 0) {
            warning_log("create new chain failed");
            return -1;
        }
    } else {
        notice_log("h only mode");
        if (create_new_chain(req.get_length(), true, chain_id) != 0) {
            warning_log("create new chain failed");
            return -1;
        }
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
        if (remove_empty_vnf_instance(chain_id) != 0) {
            warning_log("remove_empty_vnf_instance failed");
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

int Scheduler::remove_a_flow_node(FlowNode *flow_node, int vi_id, int bandwidth_cost)
{
    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get service_chain_manager failed");
        return -1;
    }

    VnfInstance *vnf_instance(NULL);
    if (service_chain_manager->get_vnf_instance(vi_id, vnf_instance) != 0) {
        warning_log("get vnf instance failed");
        return -1;
    }

    if (remove_a_flow_node(flow_node, vnf_instance, bandwidth_cost) != 0) {
        warning_log("remove a flow node failed");
        return -1;
    }

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


int Scheduler::save_pre_flow_state(Flow *flow)
{
    this->flow_state.flow_bandwidth = flow->get_flow_bandwidth();
    this->flow_state.id = flow->get_id();
    this->flow_state.cpu_cost.clear();
    this->flow_state.memory_cost.clear();
    this->flow_state.location.clear();

    FlowNode *flow_node(NULL);
    FlowManager *flow_manager(NULL);
    if ((flow_manager = FlowManager::get_instance()) == NULL) {
        warning_log("get flow manager failed");
        return -1;
    }

    for (int i = 0; i < this->flow_state.length; ++i) {
        if (flow_manager->get_flow_node(flow->get_id(), i, flow_node) != 0) {
            warning_log("get flow node failed");
            return -1;
        }

        this->flow_state.cpu_cost.push_back(flow_node->get_cpu_cost());
        this->flow_state.memory_cost.push_back(flow_node->get_memory_cost());
        this->flow_state.location.push_back(flow_node->get_location());
    }
    
    return 0;
}

int Scheduler::adjust_flow_resource(Flow *flow, const Req &req, bool &resource_enough_flag)
{
    FlowManager *flow_manager(NULL);
    if ((flow_manager = FlowManager::get_instance()) == NULL) {
        warning_log("get flow manager failed");
        return -1;
    }

    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get service_chain_manager failed");
        return -1;
    }

    //get cost
    int flow_length = flow->get_length();
    int req_length(0)
    std::vector<int> new_node_cpu_cost;
    std::vector<int> new_node_memory_cost;
    std::vector<FlowNode *> flow_nodes; 
    std::vector<VnfInstance *> flow_node_location;
    int new_flow_bandwidth_cost(0);
    if (get_template_info(req.get_flow_template_id(), req_length, 
                new_node_cpu_cost, new_node_memory_cost, new_flow_bandwidth_cost) != 0) {
        warning_log("get template info failed");
        return -1;
    }

    if (req_length != flow_length) {
        warning_log("req_length != flow_length");
        return -1;
    }

    //get location
    FlowNode *flow_node(NULL);
    VnfInstance *vnf_instance(NULL);
    for (int i = 0; i < flow_length; ++i) {
        if (flow_manager->get_flow_node(flow->get_id(), i, flow_node) != 0) {
            warning_log("get flownode failed");
            return -1;
        }
        flow_nodes.push_back(flow_node);
        int vi_id = flow_node.get_location();

        if (service_chain_manager->get_vnf_instance(vi_id, vnf_instance) != 0) {
            warning_log("get vnf instance failed");
            return -1;
        }
        if (vnf_instance == NULL) {
            warning_log("NULL ptr");
            return -1;
        }

        flow_node_location.push_back(vnf_instance);
    }

    //remove flow node
    for (int i = 0; i < flow_length; ++i) {
        if (remove_a_flow_node(flow_nodes[i], flow_node_location[i], flow->get_flow_bandwidth()) != 0) {
            warning_log("remvoe failed");
            return -1;
        }
    }

    //change requirement
    for (int i = 0; i < flow_length; ++i) {
        flow_nodes[i]->adjust_cost(new_node_cpu_cost[i], new_node_memory_cost[i]);
    }
    flow->set_flow_bandwidth(new_flow_bandwidth_cost);

    //settle flow node
    resource_enough_flag = true;
    bool local_enough_flag = true;
    double local_cost_result(0);
    for (int i = 0; i < flow_length; ++i) {
        if (settle_a_flow_node(flow_nodes[i], flow_node_location[i], flow->get_flow_bandwidth()) != 0) {
            warning_log("settle failed");
            return -1;
        }
        if (calculate_a_flow_node_cost(flow_nodes[i], local_enough_flag, local_cost_result) != 0) {
            warning_log("calculate_a_flow_node_cost failed");
            return -1;
        }
        if (local_enough_flag == false) {
            resource_enough_flag = false;
        }
    }
    return 0;
}

int Scheduler::release_flow_when_rejected(Flow *flow)
{
    int flow_length = flow->get_length();
    FlowManager *flow_manager(NULL);
    if ((flow_manager = FlowManager:get_instance()) == NULL) {
        warning_log("get flow manager failed");
        retaurn -1;
    }

    FlowNode *flow_node(NULL);
    for (int i = 0; i < flow_length; ++i) {
        if (flow_manager->get_flow_node(flow->get_id(), i, flow_node) != 0) {
            warning_log("get flow node failed");
            return -1;
        }

        if (flow_node->is_settled() == false) {
            continue;
        }

        if (remove_a_flow_node(flow_node, flow_node->get_location(), flow->get_flow_bandwidth()) != 0) {
            warning_log("remove a flow node failed");
            return -1;
        }
    }

    return 0;
}

int Scheduler::recover_flow_without_adjustment(Flow *flow)
{
    if (flow->get_id() != this->flow_state.id) {
        warning_log("flow state mismatch");
        return -1;
    }

    FlowManager *flow_manager(NULL);
    if ((flow_manager = FlowManager::get_instance()) == NULL) {
        warning_log("get flow manager failed");
        return -1;
    }

    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get service_chain_manager failed");
        return -1;
    }

    //change flow bandwidth_cost
    flow->set_flow_bandwidth(this->flow_state.flow_bandwidth);

    FlowNode *flow_node(NULL);
    VnfInstance *vnf_instance(NULL);
    for (int i = 0; i < flow_length; ++i) {
        //get flow node
        if (flow_manager->get_flow_node(flow->get_id(), i, flow_node) != 0) {
            warning_log("get flownode failed");
            return -1;
        }

        if (flow_node->is_settled()) {
            warning_log("this function require a unsettled flow");
            return -1;
        }

        //change requirement
        flow_node->adjust(this->flow_state.cpu_cost[i], this->flow_state.memory_cost[i]);

        //get located vi
        int vi_id = this->flow_state.location[i];
        if (service_chain_manager->get_vnf_instance(vi_id, vnf_instance) != 0) {
            warning_log("get vnf instance failed");
            return -1;
        }
        if (vnf_instance == NULL) {
            warning_log("NULL ptr");
            return -1;
        }

        if (settle_a_flow_node(flow_node, vnf_instance, flow->get_flow_bandwidth()) != 0) {
            warning_log("settle_a_flow_node failed");
            return -1;
        }
    }

    return 0;
}

int Scheduler::remove_empty_vnf_instance(int chain_id)
{
    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get service_chain_manager failed");
        return -1;
    }

    ServiceChain *chain(NULL);
    if (service_chain_manager->get_service_chain(chain_id, chain) != 0) {
        warning_log("get chain faield");
        return -1;
    }

    int chain_length = chain->get_length();

    VnfInstance *vnf_instance(NULL);
    for (int i = 0;i < chain_length; ++i) {
        std::vector<int> &vnf_instance = chain->get_vnf_instance(i);
        for (auto iter = vnf_instance.begin(); iter != vnf_instance.end(); ++iter) {
            int vi_id = *iter;
            if (service_chain_manager->get_vnf_instance(vi_id, vnf_instance) != 0) {
                warning_log("get vi failed");
                return -1;
            }

            if (vnf_instance->has_settled_flow_node() == false) {
                if (chain->remove_vnf_instance(i, vi_id) != 0) {
                    warning_log("remove a vi from a chain failed");
                    return -1;
                }
                if (service_chain_manager->delete_a_vnf_instance(vi_id) != 0) {
                    warning_log("delete vi failed");
                    return -1;
                }
            }
        }
    }

    //delete empty chain
    std::vector<int> &vnf_instance = chain->get_vnf_instance(0);
    if (vnf_instance.size() == 0) {
        //check
        for (int i = 1; i < chain_length; ++i) {
            if (vnf_instance.size() != 0) {
                warning_log("check: imposible situatiopn: it should be no vi in this chain");
                return -1;
            }
        }
        if (service_chain_manager->delete_a_chain(chain_id) != 0) {
            warning_log("delete a chain failed");
            return -1;
        } 
    }

    return 0;
}

int Scheduler::flow_aging()
{
    FlowManager *flow_manager(NULL);
    if ((flow_manager = FlowManager::get_instance()) == NULL) {
        warning_log("get flow manager failed");
        return -1;
    }

    auto &flow_pool = flow_manager->get_flow_pool();
    for (auto iter = flow_pool.begin(); iter != flow_pool.end(); ++iter) {
        if ((*iter)->aging() == 1) { //means lifetime_left == 0
            Flow *flow = *iter;
            int chain_id = flow->get_chain_id();
            if (release_flow_when_rejected(flow) != 0) {
                warning_log("release flow when aging failed");
                return -1;
            }
            
            if (flow_manager->delete_a_flow(flow->get_id()) != 0) {
                warning_log("delete a flow failed");
                return -1;
            }

            if (remove_empty_vnf_instance(chain_id) != 0) {
                warning_log("remove_empty_vnf_instance failed");
                return -1;
            }
        }
    }

    return 0;
}







//-------------vertical and horizontal---------------

int Scheduler::handle_req(const Req &req, bool &req_result)
{
    req_result = false;
    if (req.get_req_type() == std::string("new") && req.get_chain_id() = -1) {
        if (new_flow_on_new_chain_arrange(req, req_result) != 0) {
            warning_log("new flow on new chain arrange failed");
            return -1;
        }
    } else {
        if (sugoi_arrange(req, req_result) != 0) {
            warning_log("sugoi arrange failed");
            return -1;
        }
    }

    return 0;
}

int Scheduler::sugoi_arrange(const Req &req, bool &req_result)
{
    req_result = true; //init
    bool resource_enough_flag(false);

    Flow *flow(NULL);
    ServiceChain *chain(NULL);
    FlowManager *flow_manager(NULL);
    if ((flow_manager = FlowManager::get_instance()) == NULL) {
        warning_log("get flow_manager failed");
        return -1;
    }

    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get service_chain_manager failed");
        return -1;
    }

    if (req.get_req_type() == std::string("adjust")) {
        if (flow_manager->get_flow(req.get_flow_id(), flow) != 0 ) {
            warning_log("get flow failed");
            return -1;
        }

        if (save_pre_flow_state(flow) != 0) {
            warning_log("save_pre_flow_state failed");
            return -1;
        }
        if (adjust_flow_resource(flow, req, &resource_enough_flag) != 0) {
            warning_log("adjust_flow_resource failed");
            return -1;
        }
        if (service_chain_manager->get_service_chain(flow->get_chain_id(), chain) != 0) {
            warning_log("get service_chain failed");
            return -1;
        }
    } else if (req.get_req_type() == std::string("new")) {
        int flow_id(-1);
        if (create_new_flow(req.get_flow_template_id(), req.get_chain_id(), req.get_lifetime(), flow_id) != 0) {
            warning_log("create new flow failed");
            return -1;
        }
        if (flow_manager->get_flow(flow_id, flow) != 0) {
            warning_log("get flow failed");
            return -1;
        }
        if (service_chain_manager->get_service_chain(req.get_chain_id(), chain) != 0) {
            warning_log("get service_chain failed");
            return -1;
        }
        if (route(flow, chain, &resource_enough_flag) != 0) {
            warning_log("route failed");
            return -1;
        }
    } else {
        warning_log("illegal req type: %s", req.get_req_type().c_str());
        return -1;
    }

    if (resource_enough_flag == true) {
        req_result = true;
        return 0;
    } else {
        req_result = true;//init

        int flow_length = flow->get_length();
        FlowNode *flow_node(NULL);
        for (int i = 0; i < flow_length && req_result == true; ++i) {
            resource_enough_flag = false; //init

            if (flow_manager->get_flow_node(flow->get_id(), i, flow_node) != 0) {
                warning_log("get flow node failed");
                return -1;
            }

            if (calculate_flow_node_resource(flow_node, resource_enough_flag)) {
                warning_log("calculate_flow_node_resource failed");
                return -1;
            }
            if (resource_enough_flag == true) {
                continue;
            }

            if (migration(flow_node, chain, resource_enough_flag) != 0) {
                warning_log("migration failed");
                return -1;
            }
            if (resource_enough_flag == true) {
                continue;
            }
            
            if (scale_out(flow_node, chain, flow->get_flow_bandwidth(), resource_enough_flag) != 0) {
                warning_log("scale_out failed");
                return -1;
            }
            if (resource_enough_flag == true) {
                continue;
            }

            req_result = false; // if reach here, that means reject
        }

        if (req_result == false) {
            if (release_flow_when_rejected(flow) != 0) {
                warning_log("release flow when rejected failed");
                return -1;
            }
            if (req.get_req_type() == std::string("adjust")) {
                if (recover_flow_without_adjustment(flow) != 0) {
                    warning_log("recover_flow_without_adjustment failed");
                    return -1;
                }
            } else if (req.get_req_type() == std::string("new")) {
                if (delete_a_flow(flow->get_id()) != 0) {
                    warning_log("delete a flow failed");
                    return -1;
                }
            } else {
                warning_log("illegal req type:%s", req.get_req_type().c_str());
                return -1;
            }
        }
    }

    if (remove_empty_vnf_instance(chain->get_id()) != 0) {
        warning_log("remove_empty_vnf_instance failed");
        return -1;
    }

    return 0;
}

int Scheduler::calculate_flow_node_resource(FlowNode *flow_node, bool &resource_enough_flag)
{
    bool local_enough_flag(false);
    double local_cost_result(0);
    if (calculate_a_flow_node_cost(flow_node, local_enough_flag, local_cost_result) != 0) {
        warning_log("calculate_a_flow_node_cost failed");
        return -1;
    }
    if (local_enough_flag == false) {
        resource_enough_flag = false;
    } else {
        resource_enough_flag = true;
    }
    return 0;
}

int Scheduler::route(Flow *flow, Chain *chain, bool &resource_enough_flag)
{
    int flow_length = flow->get_length();
    FlowNode *flow_node(NULL);
    VnfInstance *vnf_instance(NULL);

    FlowManager *flow_manager(NULL);
    if ((flow_manager = FlowManager::get_instance()) == NULL) {
        warning_log("get flow manager failed");
        return -1;
    }

    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get service_chain_manager failed");
        return -1;
    }

    // stage 1
    for (int i = 0; i < flow_length; ++i) {
        auto vnf_instance_of_func_i = chain->get_vnf_instance(i);
        int vi_count = vnf_instance_of_func_i.size();
        if (vi_count == 0) {
            warning_log("no vnf instance in function %d", i);
            return -1;
        }

        if (flow_manager->get_flow_node(flow->get_id(), i, flow_node) != 0) {
            warning_log("get flow node failed");
            return -1;
        }

        bool enough_flag(false);
        double cost_result(1e9);
        RouteBestSolution route_best_solution;
        for (int j = 0; j < vi_count; ++j) {
            int vi_id = vnf_instance_of_func_i[j];
            if (service_chain_manager->get_vnf_instance(vi_id, vnf_instance) != 0) {
                warning_log("get vnf_instance failed");
                return -1;
            }
            
            if (settle_a_flow_node(flow_node, vnf_instance, flow->get_flow_bandwidth()) != 0) {
                warning_log("settle_a_flow_node failed");
                return -1;
            }

            if (calculate_a_flow_node_cost(flow_node, enough_flag, cost_result) != 0) {
                warning_log("calculate_a_flow_node_cost failed");
                return -1;
            }

            route_best_solution.update(vi_id, enough_flag, cost_result);

            if (remove_a_flow_node(flow_node, vnf_instance, flow->get_flow_bandwidth()) != 0) {
                warning_log("remove_a_flow_node failed");
                return -1;
            }
        }
        int best_vi_id = route_best_solution.get_solution();
        if (best_vi_id == -1) {
            warning_log("get best solution failed");
            return -1;
        }

        if (service_chain_manager->get_vnf_instance(best_vi_id, vnf_instance) != 0) {
            warning_log("get vnf_instance failed");
            return -1;
        }
        if (settle_a_flow_node(flow_node, vnf_instance) != 0) {
            warning_log("settle a flow node failed");
            return -1;
        }
    }

    //stage 2
    bool update_flag = true;
    while (update_flag == true) {
        update_flag = false;
        
        for (int i = 0; i < flow_length; ++i) {
            auto vnf_instance_of_func_i = chain->get_vnf_instance(i);
            int vi_count = vnf_instance_of_func_i.size();
            if (vi_count == 0) {
                warning_log("no vnf instance in function %d", i);
                return -1;
            }

            if (flow_manager->get_flow_node(flow->get_id(), i, flow_node) != 0) {
                warning_log("get flow node failed");
                return -1;
            }

            int pre_located_vi_id = flow_node->get_location();
            if (service_chain_manager->get_vnf_instance(pre_located_vi_id, vnf_instance) != 0) {
                warning_log("get pre vnf_instance failed");
                return -1;
            }

            if (remove_a_flow_node(flow_node, vnf_instance, flow->get_flow_bandwidth()) != 0) {
                warning_log("remove a flow node failed");
                return -1;
            }

            bool enough_flag(false);
            double cost_result(1e9);
            RouteBestSolution route_best_solution;
            for (int j = 0; j < vi_count; ++j) {
                int vi_id = vnf_instance_of_func_i[j];
                if (service_chain_manager->get_vnf_instance(vi_id, vnf_instance) != 0) {
                    warning_log("get vnf_instance failed");
                    return -1;
                }
                
                if (settle_a_flow_node(flow_node, vnf_instance, flow->get_flow_bandwidth()) != 0) {
                    warning_log("settle_a_flow_node failed");
                    return -1;
                }

                if (calculate_a_flow_node_cost(flow_node, enough_flag, cost_result) != 0) {
                    warning_log("calculate_a_flow_node_cost failed");
                    return -1;
                }

                route_best_solution.update(vi_id, enough_flag, cost_result);

                if (remove_a_flow_node(flow_node, vnf_instance, flow->get_flow_bandwidth()) != 0) {
                    warning_log("remove_a_flow_node failed");
                    return -1;
                }
            }
            int best_vi_id = route_best_solution.get_solution();
            if (best_vi_id == -1) {
                warning_log("get best solution failed");
                return -1;
            }

            if (service_chain_manager->get_vnf_instance(best_vi_id, vnf_instance) != 0) {
                warning_log("get vnf_instance failed");
                return -1;
            }
            if (settle_a_flow_node(flow_node, vnf_instance) != 0) {
                warning_log("settle a flow node failed");
                return -1;
            }

            if (best_vi_id != pre_located_vi_id) {
                update_flag = true;
            }
        }
    }
    return 0;
}

int Scheduler::migration(FlowNode *flow_node, ServiceChain *chain, bool &resource_enough_flag)
{
    int pre_vi_id(-1), next_vi_id(-1), vi_id(-1);
    FlowNode *pre_fn(NULL), *next_fn(NULL);
    VnfInstance *pre_vi(NULL), *next_vi(NULL);
    VnfInstance *located_vi(NULL);
    int function_id = flow_node->get_function_id();

    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get instance failed");
        return -1;
    }

    //for recover_to_settled()
    vi_id = flow_node->get_location();
    if (service_chain_manager->get_vnf_instance(vi_id, located_vi) != 0) {
        warning_log("get located_vi failed");
        return -1;
    }

    //get pre_vi & next_vi, for calculate_a_vnf_instance_cost()
    if (flow_node->has_pre_node()) {
        pre_fn = flow_node->get_pre_node();
        if (pre_fn == NULL || pre_fn->is_settled() == false) {
            warning_log("null ptr or pre_fn unsettled:impossible");
            return -1;
        }
        pre_vi_id = pre_fn->get_location();
        if (service_chain_manager->get_vnf_instance(pre_vi_id, pre_vi) != 0) {
            warning_log("get pre_vi failed");
            return -1;
        }
    }

    if (flow_node->has_next_node()) {
        next_fn = flow_node->get_next_node();
        if (next_fn == NULL || next_fn->is_settled() == false) {
            warning_log("null ptr or next_fn unsettled:impossible");
            return -1;
        }
        next_vi_id = next_fn->get_location();
        if (service_chain_manager->get_vnf_instance(next_vi_id, next_vi) != 0) {
            warning_log("get next_vi failed");
            return -1;
        }
    }

    std::vector<FlowNodeCandidate> fn_candidates;
    if (get_all_flow_nodes_in_the_same_vi(flow_node, fn_candidates) != 0) {
        warning_log("get_all_flow_nodes_in_the_same_vi failed");
        return -1;
    }
    sort(fn_candidates);
    int fn_candidates_size = fn_candidates.size();

    for (int mig_fn_count = 1; mig_fn_count <= fn_candidates_size; ++mig_fn_count) {
        for (int i = 0; i < (fn_candidates_size - mig_fn_count + 1); ++i) {
            //std::vector<FlowNodeCandidate> local_fn_candidates;
            //for (int j = i; j < i+mig_fn_count; ++j) {
            //    local_fn_candidates.push_back(fn_candidates[j]);
            //}
            //int local_fn_candidates_size = local_fn_candidates.size();
            int l = i;
            int r = i + mig_fn_count - 1;
  
            for (int j = l; j <= r; j++) {
                if (remove_a_flow_node(fn_candidates[j].flow_node, located_vi, fn_candidates[j].flow_bandwidth) != 0) {
                    warning_log("remove_a_flow_node failed");
                    return -1;
                }
            }
            bool local_enough_flag(false);
            if (calculate_a_vnf_instance_cost(located_vi, pre_vi, next_vi, local_enough_flag) != 0) {
                warning_log("calculate_a_vnf_instance_cost failed");
                return -1;
            }

            if (local_enough_flag == true && place_vnf(chain, function_id, fn_candidates, l, r, located_vi) == true) {
                resource_enough_flag = true;
                return 0;
            } else {
                if (recover_to_pre_located_vi(fn_candidates, l, r, located_vi) != 0) {
                    warning_log("recover_to_pre_located_vi failed");
                    return -1;
                }
            }
        }
    }
    
    resource_enough_flag = false;
    return 0;
}

bool Scheduler::place_vnf(ServiceChain *chain, int function_id, std::vector<FlowNodeCandidate> &fn_candidates, 
        int l, int r, VnfInstance *located_vi)
{
    ServiceChainManager *service_chain_manager(NULL);
    VnfInstance *vi(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get instance failed");
        return false;
        //return -1;
    }

    std::vector<int> vnf_instances;
    vnf_instances = chain->get_vnf_instance(function_id);

    std::vector<VnfInstanceCandidate> vi_candidates;
    for (auto iter = vnf_instances.begin(); iter != vnf_instances.end(); ++iter) {
        if (*iter != located_vi->get_id()) {
            int vi_id = *iter;

            if (service_chain_manager->get_vnf_instance(vi_id, vi) != 0) {
                warning_log("get vi failed");
                return false;
                //return -1;
            }
            int pn_id = vi->get_location();
            double cost_result(0);
            if (get_pn_cost_result(pn_id, cost_result) != 0) {
                warning_log("get_pn_cost_result failed");
                return false;
                //return -1;
            }
            
            vi_candidates.push_back(VnfInstanceCandidate());
            vi_candidates.back().vi_id = vi_id;
            vi_candidates.back().vi = vi;
            vi_candidates.back().cost_result = cost_result;
        }
    }
    sort(vi_candidates);

    for (int i = 0; i < this->place_vnf_shuffle_max_time; i++) {
        std::vector<FlowNodeCandidate> local_fn_candidates;
        for (int j = l; j <= r; ++j) {
            local_fn_candidates.push_back(fn_candidates[j]);
        }
        random_shuffle(local_fn_candidates);

        int vi_candidates_size = vi_candidates.size();
        int local_fn_candidates_size = local_fn_candidates.size();
        for (int j = 0; j < vi_candidates_size; ++j) {
            for (int k = 0; k < local_fn_candidates_size; ++k) {
                if (local_fn_candidates[k].is_settled == false) {
                    if (settle_a_flow_node(local_fn_candidates[k].flow_node, vi_candidates[j].vi, 
                                local_fn_candidates[k].flow_bandwidth) != 0) {
                        warning_log("settle_a_flow_node failed");
                        return false;
                        //return -1;
                    }
                    local_fn_candidates[k].is_settled = true;

                    bool local_enough_flag(false);
                    double local_cost_result(0);
                    if (calculate_a_flow_node_cost(local_fn_candidates[k].flow_node, local_enough_flag, local_cost_result) != 0) {
                        warning_log("calculate_a_flow_node_cost failed");
                        return false;
                        //return -1;
                    }

                    if (local_enough_flag == false) {
                        if (remove_a_flow_node(local_fn_candidates[k].flow_node, vi_candidates[j].vi, 
                                    local_fn_candidates[k].flow_bandwidth) != 0) {
                            warning_log("remove_a_flow_node failed");
                            return false;
                            //return -1;
                        }
                        local_fn_candidates[k].is_settled = false;
                    }
                }
            }
        }

        bool success_flag = true;
        for (int k = 0; k < local_fn_candidates_size && success_flag; ++k) {
            if (local_fn_candidates[k].is_settled == false) {
                success_flag = false;
            }
        }

        if (success_flag == true) {
            return true;
        } else {
            VnfInstance *vi(NULL);
            FlowNode *fn(NULL);
            for (int k = 0; k < local_fn_candidates_size; ++k) {
                if (local_fn_candidates[k].is_settled == true) {
                    //get settled vi
                    fn = local_fn_candidates[k].flow_node;
                    int vi_id = fn->get_location();
                    int flow_bandwidth = local_fn_candidates[k].flow_bandwidth;
                    if (service_chain_manager->get_vnf_instance(vi_id, vi) != 0) {
                        warning_log("get vi failed");
                        return false;
                        //return -1;
                    }

                    if (remove_a_flow_node(fn, vi, flow_bandwidth) != 0) {
                        warning_log("remove_a_flow_node failed");
                        return false;
                        //return -1;
                    }
                    local_fn_candidates[k].is_settled = false;
                }
            }
        }
    }

    return false;
}

int Scheduler::recover_to_pre_located_vi(std::vector<FlowNodeCandidate> &fn_candidates, int l, int r, VnfInstance *located_vi)
{
    for (int i = l; i <= r; ++i) {
        if (settle_a_flow_node(fn_candidates[i].flow_node, located_vi, fn_candidates[i].flow_bandwidth) != 0) {
            warning_log("settle_a_flow_node failed");
            return -1;
        }
    }
    return 0;
}

int Scheduler::get_all_flow_nodes_in_the_same_vi(FlowNode *flow_node, vector<FlowNodeCandidate *> &flow_nodes)
{
    FlowManager *flow_manager(NULL);
    if ((flow_manager = FlowManager::get_instance()) == NULL) {
        warning_log("get instance failed");
        return -1;
    }

    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get instance failed");
        return -1;
    }

    //PhysicalNodeManager *physical_node_manager(NULL);
    //if ((physical_node_manager = PhysicalNodeManager::get_instance()) == NULL) {
    //    warning_log("get instance failed");
    //    return -1;
    //}

    int vi_id = flow_node->get_location();
    VnfInstance *vi(NULL);
    if (vi_id == -1) {
        warning_log("unsettled flow_node");
        return -1;
    }

    if (service_chain_manager->get_vnf_instance(vi_id, vi) != 0) {
        warning_log("get vi failed");
        return -1;
    }

    std::vector<int> settled_flow_nodes;
    if (vi->get_settled_flow_node(settled_flow_nodes) != 0) {
        warning_log("get_settled_flow_node failed");
        return -1;
    }

    int settled_flow_nodes_size = settled_flow_nodes.size();
    FlowNode *flow_node(NULL);
    for (int i = 0; i < settled_flow_nodes_size; ++i) {
        //int pn_id = vi->get_location();
        //if (pn_id == -1) {
        //    warning_log("unsettled vi");
        //    return -1;
        //}
        int fn_id = settled_flow_nodes[i];
        if (flow_manager->get_flow_node(fn_id, flow_node) != 0) {
            warning_log("get flow node failed");
            return -1;
        }

        double cpu_cost_result = (double)flow_node->get_cpu_cost() / this->server_cpu;
        double memory_cost_result = (double)flow_node->get_memory_cost() / this->server_memory;
        double cost_result = (cpu_cost_result + memory_cost_result) / 2;
        
        
        flow_nodes.push_back(FlowNodeCandidate());
        flow_nodes.back().fn_id = fn_id;
        flow_nodes.back().flow_node = flow_node;
        flow_nodes.back().cost_result = cost_result;
        flow_nodes.back().is_settled = false;
        flow_nodes.back().flow_bandwidth = flow_node->get_flow_bandwidth();
    }

    return 0;
}

int Scheduler::scale_out(FlowNode *flow_node, ServiceChain *chain, int flow_bandwidth, bool &resource_enough_flag)
{
    int function_id = flow_node->get_function_id();
    int vi_id = flow_node->get_location();

    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get instance failed");
        return -1;
    }

    VnfInstance *vi(NULL);
    if (service_chain_manager->get_vnf_instance(vi_id, vi) != 0) {
        warning_log("get vi failed");
        return -1;
    }

    if (remove_a_flow_node(flow_node, vi, flow_bandwidth) != 0) {
        warning_log("remove_a_flow_node failed");
        return -1;
    }

    vector<ServerCandidate> server_candidates;
    if (get_not_related_servers(chain, server_candidates) != 0) {
        warning_log("get_not_related_servers failed");
        return -1;
    }

    sort(server_candidates);

    int server_candidates_size = server_candidates.size();
    int new_vi_id(-1);
    VnfInstance *new_vi(NULL);
    for (int i = 0; i < server_candidates_size; ++i) {
        if (create_a_settled_vnf_instance(chain, function_id, server_candidates[i].physical_node_id, new_vi_id) != 0) {
            warning_log("create_a_settled_vnf_instance failed");
            return -1;
        }
        if (service_chain_manager->get_vnf_instance(new_vi_id, new_vi) != 0) {
            warning_log("get new_vi failed");
            return -1;
        }
        if (settle_a_flow_node(flow_node, new_vi, flow_bandwidth) != 0) {
            warning_log("settle_a_flow_node failed");
            return -1;
        }

        bool local_enough_flag(false);
        double local_cost_result(0);
        if (calculate_a_flow_node_cost(flow_node, local_enough_flag, local_cost_result) != 0) {
            warning_log("calculate_a_flow_node_cost failed");
            return -1;
        }

        if (local_enough_flag == true) {
            resource_enough_flag = true;
            return 0;
        } else {
            if (remove_a_flow_node(flow_node, new_vi, flow_bandwidth) != 0) {
                warning_log("remove_a_flow_node failed");
                return -1;
            }
            if (chain->remove_vnf_instance(function_id, vi_id) != 0) {
                warning_log("chain remove_vnf_instance failed");
                return -1;
            }
            if (service_chain_manager->delete_a_vnf_instance(vi_id) != 0) {
                warning_log("delete_a_vnf_instance failed");
                return -1;
            }
        }
    }
    resource_enough_flag = false;
    return 0;
}

int Scheduler::get_not_related_servers(ServiceChain *chain, std::vector<ServerCandidate> &server_candidates)
{
    server_candidates.clear();

    std::set<int> related_pn_id_set;
    if (chain->get_related_pn_id(related_pn_id_set) != 0) {
        warning_log("get_related_pn_id failed");
        return -1;
    }

    for (int pn_id = 0; pn_id < this->server_count; ++pn_id) {
        double cost_result;

        if (related_pn_id_set.find(pn_id) != related_pn_id_set.end()) {
            continue;
        }

        if (get_pn_cost_result(pn_id, cost_result) != 0) {
            warning_log("get_pn_cost_result failed");
            return -1;
        }

        server_candidates.push_back(ServerCandidate());
        server_candidates.back().cost_result = cost_result;
        server_candidates.back().physical_node_id = pn_id;
    }
    notice_log("server_candidates size: %d", server_candidates.size());
    return 0;
}

int Scheduler::get_pn_cost_result(int pn_id, double &cost_result)
{
    double cpu_cost_ratio(0), memory_cost_ratio(0), bandwidth_cost_ratio(0);
    int cpu_used(0), cpu(0), memory_used(0), memory(0), up_bandwidth_used(0), up_bandwidth(0);

    PhysicalNodeManager *physical_node_manager(NULL);
    if ((physical_node_manager = PhysicalNodeManager::get_instance()) == NULL) {
        warning_log("get instance failed");
        return -1;
    }

    if (physical_node_manager->get_cpu_statistics(pn_id, cpu_used, cpu) != 0 ||
            physical_node_manager->get_memory_statistics(pn_id, memory_used, memory) != 0 ||
            physical_node_manager->get_bandwidth_statistics(pn_id, up_bandwidth_used, up_bandwidth) != 0) {
        warning_log("get statistics failed");
        return -1;
    }

    cpu_cost_ratio = (double)cpu_used / cpu;
    memory_cost_ratio = (double)memory_used / memory;
    bandwidth_cost_ratio = (double)up_bandwidth_used / up_bandwidth;

    cost_result = fmax(cpu_cost_ratio, memory_cost_ratio) + this->alpha * bandwidth_cost_ratio;
    return 0;
}

int Scheduler::create_a_settled_vnf_instance(ServiceChain *chain, int function_id, int pn_id, int &vi_id)
{
    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get instance failed");
        return -1;
    }

    if (service_chain_manager->create_a_vnf_instance(vi_id) != 0) {
        warning_log("create_a_vnf_instance failed");
        return -1;
    }

    if (chain->add_vnf_instance(function_id, vi_id) != 0) {
        warning_log("chain add_vnf_instance failed");
        return -1;
    }

    VnfInstance *vi(NULL);
    if (service_chain_manager->get_vnf_instance(vi_id, vi) != 0) {
        warning_log("get vi failed");
        return -1;
    }
    if (vi->settle(pn_id) != 0) {
        warning_log("settle vi failed");
        return -1;
    }
    
    return 0;
}


//-------------vertical only---------------
int Scheduler::handle_req_v_only(const Req &req, bool &req_result)
{
    req_result = false;
    if (req.get_req_type() == std::string("new") && req.get_chain_id() = -1) {
        if (new_flow_on_new_chain_arrange_v_only(req, req_result) != 0) {
            warning_log("new flow on new chain arrange failed");
            return -1;
        }
    } else {
        if (sugoi_arrange(req, req_result) != 0) {
            warning_log("sugoi arrange failed");
            return -1;
        }
    }

}


//-------------horizontal only---------------
int Scheduler::handle_req_h_only(const Req &req, bool &req_result)
{
    req_result = false;
    if (req.get_req_type() == std::string("new") && req.get_chain_id() = -1) {
        if (new_flow_on_new_chain_arrange(req, req_result) != 0) {
            warning_log("new flow on new chain arrange failed");
            return -1;
        }
    } else {

    }
}

  


RouteBestSolution::RouteBestSolution()
{
    this->enough_vi_id = -1;
    this->not_enough_vi_id = -1;
    this->enough_cost_result = this->not_enough_cost_result = 1e9;
}

void RouteBestSolution::update(int vi_id, bool enough_flag, double cost_result)
{
    if (enough_flag == true) {
        if (cost_result < this->enough_cost_result) {
            this->enough_cost_result = cost_result;
            this->enough_vi_id = vi_id;
        }
    } else {
        if (cost_result < this->not_enough_cost_result) {
            this->not_enough_cost_result = cost_result;
            this->not_enough_vi_id = vi_id;
        }
    }
}

int RouteBestSolution::get_solution()
{
    if (this->enough_vi_id != -1) {
        return this->enough_vi_id;
    } else if (this->not_enough_vi_id != -1) {
        return this->not_enough_vi_id;
    } else {
        warning("not valid solution");
        return -1;
    }
}

}
