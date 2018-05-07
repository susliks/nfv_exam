#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <map>
#include <string>
#include <random>
#include <algorithm>

#include "flow_manager.h"
#include "service_chain_manager.h"
#include "physical_node_manager.h"
#include "req_manager.h"

namespace nfv_exam {

struct FlowState
{
    int id;
    int length;
    std::vector<int> cpu_cost;
    std::vector<int> memory_cost;
    int flow_bandwidth;
    std::vector<int> location;
};

struct ServerCandidate
{
    double cost_result;
    int physical_node_id;

    bool operator < (const ServerCandidate &another) const {
        return cost_result < another.cost_result;
    }
};

struct FlowNodeCandidate
{
    int fn_id;
    double cost_result;
    FlowNode *flow_node;
    bool is_settled;
    int flow_bandwidth;

    bool operator < (const FlowNodeCandidate &another) const {
        return cost_result < another.cost_result;
    }
};

struct VnfInstanceCandidate
{
    int vi_id;
    double cost_result;
    VnfInstance *vi;
    
    bool operator < (const VnfInstanceCandidate &another) const {
        return cost_result < another.cost_result;
    }
};

class Scheduler {
public:

    int set_strategy(const std::string &strategy);
    int set_alpha(double alpha);
    int init();

    int handle_req_list(const std::vector<Req> &req_list, std::vector<bool> &req_accepted_flag);

    int place_first_flow_on_a_chain(int flow_id, int chain_id);
    int place_new_chain_on_physical_nodes(int chain_id, bool &req_result); //First Fit
    int settle_a_vnf_instance(VnfInstance *vnf_instance, int pn_id, int bandwidth_cost, VnfInstance *pre_vi, VnfInstance *next_vi);
    int calculate_a_vnf_instance_cost(VnfInstance *vnf_instance, VnfInstance *pre_vi, VnfInstance *next_vi, bool &enough_flag);//todo:not sure whether cost is needed
    int remove_a_vnf_instance(VnfInstance *vnf_instance, int bandwidth_cost, VnfInstance *pre_vi, VnfInstance *next_vi);
    int release_service_chain_when_rejected(ServiceChain *service_chain);

    int create_new_chain(int length, int &chain_id, bool disable_scale_up_down);
    int create_new_flow(int flow_template_id, int chain_id, int lifetime, int &flow_id);

    int settle_a_flow_node(FlowNode *flow_node, VnfInstance *vnf_instance, int bandwidth_cost);
    int settle_a_flow_v(FlowNode *flow_node, VnfInstance *vnf_instance);
    int settle_a_flow_link(FlowNode *flow_node_1, FlowNode *flow_node_2, int bandwidth_cost);

    int calculate_a_flow_node_cost(FlowNode *flow_node, bool &enough_flag, double &cost_result);
    int calculate_a_flow_v_cost(FlowNode *flow_node, bool &enough_flag, double &host_cost_result);
    int calculate_a_flow_node_links_cost(FlowNode *flow_node, bool &enough_flag, double &bandwidth_cost);
    int calculate_a_flow_link_cost(FlowNode *flow_node_1, FlowNode *flow_node_2, bool &enough_flag, int &delta_bandwidth);

    int remove_a_flow_node(FlowNode *flow_node, VnfInstance *vnf_instance, int bandwidth_cost);
    int remove_a_flow_node(FlowNode *flow_node, int vnf_instance_id, int bandwidth_cost);
    int remove_a_flow_v(FlowNode *flow_node, VnfInstance *vnf_instance);
    int remove_a_flow_link(FlowNode *flow_node_1, FlowNode *flow_node_2, int bandwidth_cost);

    int delete_a_flow(int flow_id);

    int save_pre_flow_state(Flow *flow);
    int adjust_flow_resource(Flow *flow, const Req &req, bool &resource_enough_flag);
    int release_flow_when_rejected(Flow *flow);
    int recover_flow_without_adjustment(Flow *flow);

    int remove_empty_vnf_instance(int chain_id);
    

    int new_flow_on_new_chain_arrange(const Req &req, bool &req_result);

    int flow_aging();
        

    //-------------vertical and horizontal---------------
    int handle_req(const Req &req, bool &req_result);
    int sugoi_arrange(const Req &req, bool &req_result);
    int route(Flow *flow, ServiceChain *chain, bool &resource_enough_flag);

    int migration(FlowNode *flow_node, int flow_bandwidth, ServiceChain *chain, bool &resource_enough_flag);
    int get_all_flow_nodes_in_the_same_vi(FlowNode *flow_node, std::vector<FlowNodeCandidate> &fn_candidates);
    bool place_vnf(ServiceChain *chain, int function_id, std::vector<FlowNodeCandidate> &fn_candidates, 
            int l, int r, VnfInstance *located_vi);
    int recover_to_pre_located_vi(std::vector<FlowNodeCandidate> &fn_candidates, int l, int r, VnfInstance *located_vi);

    int scale_out(FlowNode *flow_node, ServiceChain *chain, int flow_bandwidth, bool &resource_enough_flag);
    int get_not_related_servers(ServiceChain *chain, std::vector<ServerCandidate> &server_candidates);
    int get_pn_cost_result(int pn_id, double &cost_result);
    int create_a_settled_vnf_instance(ServiceChain *chain, int function_id, int pn_id, int &vi_id);

    int calculate_flow_node_resource(FlowNode *flow_node, bool &resource_enough_flag);


    //-------------vertical only---------------
    int handle_req_v_only(const Req &req, bool &req_result);
    int v_only_place_flow_on_linear_chain(Flow *flow, ServiceChain *chain, bool &req_result);


    //-------------horizontal only---------------
    int handle_req_h_only(const Req &req, bool &req_result);
    int h_only_greedy_arrange(Flow *flow, ServiceChain *chain, bool &req_result);
    int h_only_get_vi_candidates_on_chain_of_function_i(ServiceChain *chain, int function_id, 
            std::vector<VnfInstanceCandidate> &vi_candidates);

    //int h_only_settle_a_flow_node(FlowNode *flow_node, VnfInstance *vnf_instance, int bandwidth_cost);
    //int h_only_settle_a_flow_v(FlowNode *flow_node, VnfInstance *vnf_instance);
    //int h_only_settle_a_flow_link(FlowNode *flow_node_1, FlowNode *flow_node_2, int bandwidth_cost);

    //int h_only_calculate_a_flow_node_cost(FlowNode *flow_node, bool &enough_flag, double &cost_result);
    //int h_only_calculate_a_flow_v_cost(FlowNode *flow_node, bool &enough_flag, double &host_cost_result);
    //int h_only_calculate_a_flow_node_links_cost(FlowNode *flow_node, bool &enough_flag, double &bandwidth_cost);
    //int h_only_calculate_a_flow_link_cost(FlowNode *flow_node_1, FlowNode *flow_node_2, bool &enough_flag, int &delta_bandwidth);

    //int h_only_remove_a_flow_node(FlowNode *flow_node, VnfInstance *vnf_instance, int bandwidth_cost);
    //int h_only_remove_a_flow_node(FlowNode *flow_node, int vnf_instance_id, int bandwidth_cost);
    //int h_only_remove_a_flow_v(FlowNode *flow_node, VnfInstance *vnf_instance);
    //int h_only_remove_a_flow_link(FlowNode *flow_node_1, FlowNode *flow_node_2, int bandwidth_cost);

    //int h_only_release_flow_when_rejected(Flow *flow);




private:
    void search_physical_node_ptr_add_one();
    void search_physical_node_ptr_random_jump();



private:
    std::string strategy;

    int search_physical_node_ptr;
    int server_count;
    int server_cpu;
    int server_memory;

    int physical_nodes_total_bandwidth; 
    double alpha;

    FlowState flow_state;

    int place_vnf_shuffle_max_time;

    

};

class RouteBestSolution
{
public:
    RouteBestSolution();
    
    void update(int vi_id, bool enough_flag, double cost_result);
    int get_solution();
private:
    double enough_cost_result;
    int enough_vi_id;
    double not_enough_cost_result;
    int not_enough_vi_id;
};

}

#endif
