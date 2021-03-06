#ifndef SERVICE_CHAIN_MANAGER_H
#define SERVICE_CHAIN_MANAGER_H

//#include <cstdio>
#include <vector>
#include <map>
#include <algorithm>
#include <set>

#include "vnf_instance.h"
#include "service_chain.h"
#include "flow_manager.h"
#include "physical_node_manager.h"

namespace nfv_exam {

struct ViTemplate
{
    int cpu;
    int memory;
    bool operator < (const ViTemplate &another) const {
        return cpu < another.cpu;
    }
};

class ServiceChainManager {
public:
    ~ServiceChainManager();

    static ServiceChainManager *get_instance();

    //int set_default_vnf_instance_resource(int vnf_instance_default_cpu, int vnf_instance_default_memory);

    int set_cpu_enlarge_factor(int cpu_enlarge_factor);
    int init();

    int get_service_chain(int chain_id, ServiceChain **service_chain);
    int get_vnf_instance(int vi_id, VnfInstance **vnf_instance);

    int remove_a_flow_from_a_chain(int flow_id, int chain_id);
    int place_first_flow_on_an_unsettled_chain(int flow_id, int chain_id);

    int update_length2active_chain_id(int length);
    int get_all_active_chain_id(int length, std::vector<int> &active_chain_id);

    int create_a_chain(int length, bool disable_scale_up_down, int &chain_id);
    int create_a_vnf_instance(int &vi_id);
    int delete_a_chain(int chain_id); // don't call it immediately. call it by call remove_empty_vnf_instance()
    int delete_a_vnf_instance(int vi_id);

    int load_vi_template(); //todo:hard code
    int get_vi_template(int fn_cpu_cost, int fn_memory_cost, int &vi_cpu, int &vi_memory);

    int get_first_vnf_instance(int chain_id, int func_id, VnfInstance **vnf_instance);

    int add_vnf_instance(ServiceChain *chain, int function_id, int vi_id);
    int remove_vnf_instance(ServiceChain *chain, int function_id, int vi_id);
    int get_related_pn_id(ServiceChain *chain, std::set<int> &pn_id_set);

    int assign_vi_resource(int vi_id, int cpu_used, int memory_used);
    int release_vi_resource(int vi_id, int cpu_used, int memory_used);

    int get_cpu_statistics(int vi_id, int &cpu_used, int &cpu);
    int get_memory_statistics(int vi_id, int &memory_used, int &memory);

private:

private:
    std::map<int, ServiceChain*> service_chain_pool;
    int service_chain_id_count;
    std::map<int, VnfInstance*> vnf_instance_pool;
    int vnf_instance_id_count;

    std::map<int, std::vector<int> *> length2active_chain_id;

    int cpu_enlarge_factor;

    //int vnf_instance_default_cpu;
    //int vnf_instance_default_memory;

//-------------horizontal only---------------
    std::vector<ViTemplate> vi_template;

};

}

#endif /* SERVICE_CHAIN_MANAGER_H */

