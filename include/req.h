#ifndef REQ_H
#define REQ_H

#include <vector>

#include "log.h"

namespace nfv_exam {

class Req {
public:
    void clear();
    std::string to_string();

    int get_chain_id();
    int get_flow_id();
    int get_length();
    int get_flow_template_id();
    int get_lifetime();
    const std::string &get_req_type();

    void set_chain_id(int chain_id);
    void set_flow_id(int flow_id);
    void set_length(int length);
    void set_flow_template_id(int flow_template_id);
    void set_lifetime(int lifetime);
    void set_req_type(const std::string &req_type);


private:
    std::string req_type;
    int length;
    int chain_id;   // only work when req_type=="new"
    int flow_id;    //only work when req_type=="adjust"
    int flow_template_id;
    int lifetime;
};

struct FlowTemplate {
    int length;
    std::vector<int> node_cpu_cost;
    std::vector<int> node_memory_cost;
    int flow_bandwidth_cost;

    std::string to_string()
    {
        char buf[10000]; //magic number
        int cpu_enlargement_factor = 100;
        std::string result;
        sprintf(buf, "length=%d, flow_bandwidth=%d node:", length, flow_bandwidth_cost);
        result += std::string(buf);

        for(int i = 0; i < length; ++i) {
            sprintf(buf, "%lf/%d ", (double)node_cpu_cost[i]/cpu_enlargement_factor, node_memory_cost[i]);
            result += std::string(buf);
        }

        return result;
    }
};

}

#endif /* REQ_H */
