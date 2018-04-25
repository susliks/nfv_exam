#ifndef REQ_MANAGER_H
#define REQ_MANAGER_H

#include <cstdio>
#include <vector>
#include <map>
#include <cmath>

#include "req.h"

namespace nfv_exam {

class ReqManager {
public:
    ~ReqManager();

    static ReqManager *get_instance();

    int set_flow_template_file_path(std::string file_path);
    int set_generate_procedure_count(int new_req_procedure_count, int adjust_req_procedure_count);
    int init();
    int load_flow_template();

private:


    int load_flow_template_from_txt();
    int extend_flow_template_trick(int length, int flow_bandwidth_cost, 
            const std::vector<double> &node_cpu_cost, const std::vector<int> &node_memory_cost);

    int add_flow_template(int length, int flow_bandwidth_cost, 
            const std::vector<double> &node_cpu_cost, const std::vector<int> &node_memory_cost);

private:
    std::string flow_template_file_path;

    std::map<int, FlowTemplate *> flow_template;

    int new_req_procedure_count;
    int adjust_req_procedure_count;
    std::vector<std::vector<int> > new_req_timestamp;
    std::vector<std::vector<int> > adjust_req_timestamp;

    
    int accepted_req_count;
    int total_req_count;

};

}

#endif /* REQ_MANAGER_H */

