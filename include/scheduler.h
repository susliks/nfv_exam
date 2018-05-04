#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <map>
#include <string>

#include "flow_manager.h"
#include "service_chain_manager.h"
#include "physical_node_manager.h"
#include "req_manager.h"

namespace nfv_exam {

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
    int release_service_chain_when_rejected(ServiceChain *service_chain)

    int create_new_chain(int length, bool disable_scale_up_down = false, int &chain_id);
    int create_new_flow(int flow_template_id, int chain_id, int lifetime, int &flow_id);

    int settle_a_flow_node(FlowNode *flow_node, VnfInstance *vnf_instance, int bandwidth_cost);
    int settle_a_flow_v(FlowNode *flow_node, VnfInstance *vnf_instance);
    int settle_a_flow_link(FlowNode *flow_node_1, FlowNode *flow_node_2, int bandwidth_cost);

    int calculate_a_flow_node_cost(FlowNode *flow_node, bool &enough_flag, double &cost_result);
    int calculate_a_flow_v_cost(FlowNode *flow_node, bool &enough_flag, double &host_cost_result);
    int calculate_a_flow_node_links_cost(FlowNode *flow_node, bool &enough_flag, double &bandwidth_cost);
    int calculate_a_flow_link_cost(FlowNode *flow_node_1, FlowNode *flow_node_2, bool &enough_flag, int &delta_bandwidth);

    int remove_a_flow_node(FlowNode *flow_node, VnfInstance *vnf_instance, int bandwidth_cost);
    int remove_a_flow_v(FlowNode *flow_node, VnfInstance *vnf_instance);
    int remove_a_flow_link(FlowNode *flow_node_1, FlowNode *flow_node_2, int bandwidth_cost);


        

    //-------------vertical and horizontal---------------
    int handle_req(const Req &req, bool &req_result);
    int sugoi_arrange(const Req &req, bool &req_result)
    int route()
    int migration()
    int scale_out()


    //-------------vertical only---------------
    int handle_req_v_only(const Req &req, bool &req_result);


    //-------------horizontal only---------------
    int handle_req_h_only(const Req &req, bool &req_result);
   





private:
    void search_physical_node_ptr_add_one();
    void search_physical_node_ptr_random_jump();



private:
    std::string strategy;

    int search_physical_node_ptr;
    int server_count;
    int physical_nodes_total_bandwidth; //TODO
    double alpha;

};

}

#endif
