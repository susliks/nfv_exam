#ifndef REQ_MANAGER_H
#define REQ_MANAGER_H

#include <cstdio>
#include <vector>
#include <map>
#include <cmath>
#include <string>

#include "req.h"

namespace nfv_exam {

class ReqManager {
public:
    ~ReqManager();

    static ReqManager *get_instance();

    int set_flow_template_file_path(const std::string &file_path);
    int set_generate_procedure_count(int new_req_procedure_count, int adjust_req_procedure_count);
    int set_exam_lifetime(int exam_lifetime);
    int init();

    int generate_req(int cur_time, std::vector<Req> &req_list);
    int update_evaluation(const std::vector<bool> &req_accepted_flag);

    int get_template_info(int flow_template_id, int &length, std::vector<int> &node_cpu_cost, 
            std::vector<int> &node_memory_cost, int &flow_bandwidth_cost); 

    int set_req_evaluation_file_path(const std::string &file_path);
    

private:


    int load_flow_template_from_txt();
    int extend_flow_template_trick(int length, int flow_bandwidth_cost, 
            const std::vector<double> &node_cpu_cost, const std::vector<int> &node_memory_cost);

    int add_flow_template(int length, int flow_bandwidth_cost, 
            const std::vector<double> &node_cpu_cost, const std::vector<int> &node_memory_cost);

    int generate_timestamp();
    double random_exponential(double lambda);
    int generate_poisson_seq(int exam_lifetime, std::vector<int> &req_timestamp);

    int generate_req_info(const std::string &req_type, Req &req);

    int get_random_int(int l, int r);
    int get_random_template_id(int length);
    int get_random_template_id();
    int build_length2flow_template_id();

    int random_pick_an_active_flow(int &flow_id, int &flow_length);
    int random_pick_an_active_chain(int length, int &chain_id);
    int decide_chain_id(int length, int &chain_id);

    int save_evaluation();

private:
    std::string flow_template_file_path;
    std::string req_evaluation_file_path;

    std::map<int, FlowTemplate *> flow_template;
    std::map<int, std::vector<int> > length2flow_template_id;

    int exam_lifetime;
    int new_req_procedure_count;
    int adjust_req_procedure_count;
    std::vector<std::vector<int> > new_req_timestamp;
    std::vector<std::vector<int> > adjust_req_timestamp;
    std::vector<int> new_req_timestamp_ptr;
    std::vector<int> adjust_req_timestamp_ptr;

    
    int accepted_req_count;
    int total_req_count;
    std::vector<double> accept_ratio_history;

};

}

#endif /* REQ_MANAGER_H */

