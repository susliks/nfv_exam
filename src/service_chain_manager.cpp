#include "service_chain_manager.h"
#include "log.h"

namespace nfv_exam {

ServiceChainManager::~ServiceChainManager()
{
    for (std::map<int, ServiceChain *>::iterator itr = service_chain_pool.begin();
            itr != service_chain_pool.end(); ++itr) {
        if (itr->second != NULL) {
            delete itr->second;
        }
    }
    for (std::map<int, VnfInstance *>::iterator itr = vnf_instance_pool.begin();
            itr != vnf_instance_pool.end(); ++itr) {
        if (itr->second != NULL) {
            delete itr->second;
        }
    }
}

ServiceChainManager *ServiceChainManager::get_instance()
{
    static ServiceChainManager *instance(NULL);

    if (instance == NULL) {
        instance = new ServiceChainManager;
    }
    return instance;
}


int ServiceChainManager::update_length2active_chain_id(int chain_length)
{
    if (this->length2active_chain_id.find(chain_length) == this->length2active_chain_id.end()) {
        length2active_chain_id[chain_length] = new std::vector<int>;
    } else {
        length2active_chain_id[chain_length]->clear();
    }

    for (auto iter = this->service_chain_pool.begin(); iter != this->service_chain_pool.end(); ++iter) {
        if (iter->second->get_length() == chain_length) {
            this->length2active_chain_id[chain_length]->push_back(iter->first);
        }
    }

    return 0;
}

int ServiceChainManager::get_all_active_chain_id(int chain_length, std::vector<int> &active_chain_id)
{
    if (update_length2active_chain_id(chain_length) != 0) {
        warning_log("update_length2active_chain_id failed");
        return -1;
    }

    if (length2active_chain_id.find(chain_length) != length2active_chain_id.end()) {
        active_chain_id = *length2active_chain_id[chain_length];
        return 0;
    } else {
        warning_log("not exist chain_length = %d", chain_length);
        return -1;
    }
}

//int ServiceChainManager::set_default_vnf_instance_resource(int vnf_instance_default_cpu, int vnf_instance_default_memory)
//{
//    this->vnf_instance_default_cpu = vnf_instance_default_cpu;
//    this->vnf_instance_default_memory = vnf_instance_default_memory;
//    return 0;
//}

int ServiceChainManager::create_a_chain(int length, bool disable_scale_up_down, int &chain_id)
{
    chain_id = service_chain_id_count;
    this->service_chain_pool[chain_id] = new ServiceChain;
    this->service_chain_pool[chain_id]->init(chain_id, length, 0);

    std::vector<int> vnf_instances;
    int vi_id(-1);
    int cpu_cost(0), memory_cost(0);
    if (disable_scale_up_down == true) {
        //wait until place a flow in a chain:
        //cpu_cost = vnf_instance_default_cpu;
        //memory_cost = vnf_instance_default_memory;
    }

    for (int i = 0; i < length; ++i) {
        create_a_vnf_instance(vi_id);
        this->vnf_instance_pool[vi_id]->init(vi_id, -1, disable_scale_up_down, 0, 0, cpu_cost, memory_cost);
        vnf_instances.push_back(vi_id);
    }
    
    for (int i = 0; i < length; ++i) {
        //this->service_chain_pool[chain_id]->add_vnf_instance(i, vnf_instances[i]);
        if (add_vnf_instance(this->service_chain_pool[chain_id], i, vnf_instances[i]) != 0) {
            warning_log("add_vnf_instance failed");
            return -1;
        }
    }
    //for (int i = 0; i < length-1; ++i) {
    //    this->vnf_instance_pool[vnf_instances[i]]->set_next_vi_id(vnf_instances[i+1]);
    //}
    //for (int i = 1; i < length; ++i) {
    //    this->vnf_instance_pool[vnf_instances[i]]->set_pre_vi_id(vnf_instances[i-1]);
    //}

    service_chain_id_count += 1;
    return 0;
}

int ServiceChainManager::create_a_vnf_instance(int &vi_id)
{
    vi_id = vnf_instance_id_count;
    this->vnf_instance_pool[vi_id] = new VnfInstance;
    this->vnf_instance_pool[vi_id]->init(vi_id, -1, false, 0, 0, 0, 0);
    vnf_instance_id_count += 1;
    return 0;
}

int ServiceChainManager::delete_a_chain(int chain_id)
{
    auto vnf_instances = this->service_chain_pool[chain_id]->get_vnf_instance();
    for (auto func_iter = vnf_instances.begin(); func_iter != vnf_instances.end(); ++func_iter) {
        for (auto vi_iter = func_iter->second->begin(); vi_iter != func_iter->second->end(); ++vi_iter) {
            delete this->vnf_instance_pool[*vi_iter];
        }
    }
    delete this->service_chain_pool[chain_id];
    this->service_chain_pool.erase(chain_id);
    return 0;
}

int ServiceChainManager::delete_a_vnf_instance(int vi_id)
{
    if (this->vnf_instance_pool.find(vi_id) == this->vnf_instance_pool.end()) {
        warning_log("not existed vi: %d", vi_id);
        return -1;
    }
    delete this->vnf_instance_pool[vi_id];
    this->vnf_instance_pool.erase(vi_id);
    return 0;
}

//update settled_flow_nodes. 
//no resource release (has been done in release_a_service_chain_when_rejected())
int ServiceChainManager::remove_a_flow_from_a_chain(int flow_id, int chain_id)
{
    FlowManager *flow_manager(NULL);
    if ((flow_manager = FlowManager::get_instance()) == NULL) {
        warning_log("get flow_manager failed");
        return -1;
    }

    Flow *flow(NULL);
    ServiceChain *chain(NULL);
    if (flow_manager->get_flow(flow_id, &flow) != 0) {
        warning_log("get flow failed");
        return -1;
    }
    if (get_service_chain(chain_id, &chain) != 0) {
        warning_log("get chain failed");
        return -1;
    }
    debug_log("chain info:%s", chain->to_string().c_str());
    debug_log("flow info:%s", flow->to_string().c_str());
    


    int flow_length = flow->get_length();
    int chain_length = chain->get_length();
    if (flow_length != chain_length) {
        warning_log("length mismatch");
        return -1;
    }

//TODO:debug
for (int i = 0; i < flow_length; ++i) {
    FlowNode *flow_node(NULL);
    if (flow_manager->get_flow_node(flow_id, i, &flow_node) != 0) {
        warning_log("get flow node failed");
        return -1;
    }
    debug_log("flow_node %d info:%s", i, flow_node->to_string().c_str());
}

    FlowNode *flow_node(NULL);
    VnfInstance *vnf_instance(NULL);
    for (int i = 0; i < flow_length; ++i) {
        if (flow_manager->get_flow_node(flow_id, i, &flow_node) != 0) {
            warning_log("get flow node failed");
            return -1;
        }
        if (flow_node->is_settled() == false) {
            continue;
        }

        debug_log("flow_node %d info:%s", i, flow_node->to_string().c_str());

        int vi_id = flow_node->get_location();
        if (get_vnf_instance(vi_id, &vnf_instance) != 0) {
            warning_log("get vi failed");
            return -1;
        }

        if (vnf_instance->remove_settled_flow_node(flow_node->get_id()) != 0) {
            warning_log("remove_settled_flow_node failed");
            return -1;
        }
    }

    return 0;
}

int ServiceChainManager::place_first_flow_on_an_unsettled_chain(int flow_id, int chain_id)
{
    debug_log("flow_id = %d, chain_id = %d", flow_id, chain_id);
    auto vnf_instances = this->service_chain_pool[chain_id]->get_vnf_instance();
    //linear chain check
    for (auto func_iter = vnf_instances.begin(); func_iter != vnf_instances.end(); ++func_iter) {
        if (func_iter->second->size() != 1) {
            warning_log("it's not a linear service chain");
            return -1;
        }
    }

    FlowManager *flow_manager(NULL);
    if ((flow_manager = FlowManager::get_instance()) == NULL) {
        warning_log("get flow manager instance failed");
        return -1;
    }

    Flow *flow(NULL);
    if (flow_manager->get_flow(flow_id, &flow) != 0 || flow == NULL) {
        warning_log("get flow failed");
        return -1;
    }
    debug_log("flow info:%s", flow->to_string().c_str());

    int flow_length = flow->get_length();
    int chain_length = this->service_chain_pool[chain_id]->get_length();
    if (flow_length != chain_length) {
        warning_log("flow_length != chain_length");
        return -1;
    }

    FlowNode *flow_node(NULL);
    VnfInstance *vnf_instance(NULL);
    ServiceChain *chain = this->service_chain_pool[chain_id];
    debug_log("chain info:%s", chain->to_string().c_str());
    for (int i = 0; i < flow_length; ++i) {
        int vi_id = chain->get_first_vi_id(i);
        if (vi_id == -1) {
            warning_log("get first vi id failed");
            return -1;
        }

        //if (get_vnf_instance(chain_id, i, vnf_instance) != 0) {
        if (get_vnf_instance(vi_id, &vnf_instance) != 0) {
            warning_log("get vnf instance failed");
            return -1;
        }
        if (flow_manager->get_flow_node(flow_id, i, &flow_node) != 0) {
            warning_log("get flow node failed");
            return -1;
        }

        //update vi & flow_node location 
        if (vnf_instance->add_settled_flow_node(flow_node->get_id()) != 0) {
            warning_log("set vi setttled flow node failed");
            return -1;
        }

        if (flow_node->settle(vnf_instance->get_id()) != 0) {
            warning_log("set flow_node location failed");
            return -1;
        }
        
        //update vnf instance cpu and memory cost
        int cpu_used = flow_node->get_cpu_cost();
        int memory_used = flow_node->get_memory_cost();

        //handle h only
        if (vnf_instance->get_disable_scale_up_down() == true) {
            int cpu_cost(0);
            int memory_cost(0);
            if (get_vi_template(cpu_used, memory_used, cpu_cost, memory_cost) != 0) {
                warning_log("get vi template failed");
                return -1;
            }
            if (vnf_instance->set_vi_resource_cost(cpu_cost, memory_cost) != 0) {
                warning_log("set vi resource cost failed");
                return -1;
            }
        }

        if (vnf_instance->set_vi_resource_used(cpu_used, memory_used) != 0) {
            warning_log("set vi first resourced used failed");
            return -1;
        }
    }

    //update chain bandwidth cost
    int bandwidth_used = flow->get_flow_bandwidth();
    chain->set_bandwidth_used(bandwidth_used);

    //update flow's chain_id
    flow->set_chain_id(chain->get_id());

    return 0;
}

//int ServiceChainManager::place_flow_on_a_linear_chain(int flow_id, int chain_id)
//{
//
//}

int ServiceChainManager::get_service_chain(int chain_id, ServiceChain **service_chain)
{
    if (this->service_chain_pool.find(chain_id) == this->service_chain_pool.end()) {
        warning_log("not exist chain_id: %d", chain_id);
        return -1;
    }
    if ((*service_chain = this->service_chain_pool[chain_id]) == NULL) {
        warning_log("null ptr");
        return -1;
    }
    return 0;
}

int ServiceChainManager::get_vnf_instance(int vi_id, VnfInstance **vnf_instance)
{
    if (this->vnf_instance_pool.find(vi_id) == this->vnf_instance_pool.end()) {
        warning_log("not exist vi_id: %d", vi_id);
        return -1;
    }
    if ((*vnf_instance = this->vnf_instance_pool[vi_id]) == NULL) {
        warning_log("null ptr");
        return -1;
    }
    return 0;
}

int ServiceChainManager::get_first_vnf_instance(int chain_id, int func_id, VnfInstance **vnf_instance)
{
    ServiceChain *service_chain(NULL);
    if (get_service_chain(chain_id, &service_chain) != 0) {
        warning_log("get service chain failed");
        return -1;
    }

    //const auto &vnf_instances = service_chain->get_vnf_instance();
    //const std::map<int, std::vector<int> *> &vnf_instances = service_chain->get_vnf_instance(); //fixed: not elegant
    std::map<int, std::vector<int> *> &vnf_instances = service_chain->get_vnf_instance();

    if (vnf_instances.find(func_id) == vnf_instances.end()) {
        warning_log("func_id not found %d", func_id);
        return -1;
    }
       
    if (vnf_instances[func_id]->size() != 1 ) {
        warning_log("this function only for linear chain");
        return -1;
    }

    int vi_id = (*vnf_instances[func_id])[0];
    if (get_vnf_instance(vi_id, vnf_instance) != 0) {
        warning_log("get vnf_instance failed");
        return -1;
    }

    return 0;
}
    
int ServiceChainManager::set_cpu_enlarge_factor(int cpu_enlarge_factor)
{
    this->cpu_enlarge_factor = cpu_enlarge_factor;
    return 0;
}

int ServiceChainManager::init()
{
    load_vi_template();
    service_chain_id_count = 0;
    vnf_instance_id_count = 0;

    return 0;
}

int ServiceChainManager::load_vi_template()
{
    ViTemplate vi_template;
    vi_template.cpu = 2 * this->cpu_enlarge_factor;
    vi_template.memory = 4000;
    this->vi_template.push_back(vi_template);
    vi_template.cpu = 4 * this->cpu_enlarge_factor;
    vi_template.memory = 8000;
    this->vi_template.push_back(vi_template);
    vi_template.cpu = 6 * this->cpu_enlarge_factor;
    //vi_template.cpu = 8 * this->cpu_enlarge_factor;
    //vi_template.memory = 32000;
    //vi_template.memory = 24000;
    //TODO:adjust para
    vi_template.memory = 16000;
    this->vi_template.push_back(vi_template);
    sort(this->vi_template.begin(), this->vi_template.begin() + this->vi_template.size());

    for (auto iter = this->vi_template.begin(); iter != this->vi_template.end(); ++iter) {
        notice_log("add vi template: cpu:%d memory:%d", iter->cpu, iter->memory);
    }

    return 0;
}

int ServiceChainManager::get_vi_template(int fn_cpu_cost, int fn_memory_cost, int &vi_cpu, int &vi_memory)
{
    for (auto iter = this->vi_template.begin(); iter != this->vi_template.end(); ++iter) {
        if (fn_cpu_cost <= iter->cpu && fn_memory_cost <= iter->memory) {
            vi_cpu = iter->cpu;
            vi_memory = iter->memory;
            return 0;
        }
    }
    warning_log("no capable vi template");
    return -1;
}

int ServiceChainManager::add_vnf_instance(ServiceChain *chain, int function_id, int vi_id)
{
    auto &vnf_instance = chain->get_vnf_instance();

    if (vnf_instance.find(function_id) == vnf_instance.end()) {
        warning_log("not exist funciton id: %d", function_id);
        return -1;
    }

    for (auto iter = vnf_instance[function_id]->begin();
            iter != vnf_instance[function_id]->end(); ++iter) {
        if (*iter == vi_id) {
            warning_log("existed vi_id. chain_id=%d, function_id=%d, vi_id=%d", chain->get_id(), function_id, vi_id);
            return -1;
        }
    }

    vnf_instance[function_id]->push_back(vi_id);


    //set edge relation
    VnfInstance *vi(NULL), *pre_vi(NULL), *next_vi(NULL);
    if (get_vnf_instance(vi_id, &vi) != 0) {
        warning_log("get vi failed");
        return -1;
    }
    
    if (function_id > 0) {
        for (auto iter = vnf_instance[function_id-1]->begin();
                iter != vnf_instance[function_id-1]->end(); ++iter) {
            int pre_vi_id = *iter;
            if (get_vnf_instance(pre_vi_id, &pre_vi) != 0) {
                warning_log("get pre vi failed");
                return -1;
            }
            if (vi->add_pre_vi_id(pre_vi_id) != 0 || pre_vi->add_next_vi_id(vi_id) != 0) {
                warning_log("add relation failed");
                return -1;
            }
        }
    }
    if (function_id < chain->get_length()-1) {
        for (auto iter = vnf_instance[function_id+1]->begin();
                iter != vnf_instance[function_id+1]->end(); ++iter) {
            int next_vi_id = *iter;
            if (get_vnf_instance(next_vi_id, &next_vi) != 0) {
                warning_log("get next vi failed");
                return -1;
            }
            if (vi->add_next_vi_id(next_vi_id) != 0 || next_vi->add_pre_vi_id(vi_id) != 0) {
                warning_log("add relation failed");
                return -1;
            }
        }
    }
    debug_log("add_vnf_instance done");
    debug_log("vi info:%s", vi->to_string().c_str());

    return 0;
}

int ServiceChainManager::remove_vnf_instance(ServiceChain *chain, int function_id, int vi_id)
{
    debug_log("function_id:%d vi_id:%d", function_id, vi_id);
    std::map<int, std::vector<int> *> &vnf_instance = chain->get_vnf_instance();

    if (vnf_instance.find(function_id) == vnf_instance.end()) {
        warning_log("not exist funciton id: %d", function_id);
        return -1;
    }

    VnfInstance *vi(NULL), *pre_vi(NULL), *next_vi(NULL);
    if (get_vnf_instance(vi_id, &vi) != 0) {
        warning_log("get vi failed");
        return -1;
    }

    //if h_only: release physical_node resource
    //TODO: not sure yes or no
    //if (vi->get_disable_scale_up_down() == true) {
    if (vi->get_disable_scale_up_down() == true && vi->is_settled()) {
        PhysicalNodeManager *physical_node_manager(NULL);
        if ((physical_node_manager = PhysicalNodeManager::get_instance()) == NULL) {
            warning_log("get instance failed");
            return -1;
        }

        int cpu_cost = vi->get_cpu_cost();
        int memory_cost = vi->get_memory_cost();
        int pn_id = vi->get_location();
        if (pn_id == -1) {
            warning_log("unsettled vi, impossible");
            return -1;
        }

debug_log("release vi resource, pn_id=%d, cpu_cost=%d, memory_cost=%d", pn_id, cpu_cost, memory_cost);
        if (physical_node_manager->release_host_resource(pn_id, cpu_cost, memory_cost) != 0) {
            warning_log("release host resource when h_only, remove a vi failed");
            return -1;
        }
    }

    //update edge relation
    if (function_id > 0) {
        for (auto iter = vnf_instance[function_id-1]->begin();
                iter != vnf_instance[function_id-1]->end(); ++iter) {
            int pre_vi_id = *iter;
            if (get_vnf_instance(pre_vi_id, &pre_vi) != 0) {
                warning_log("get pre vi failed");
                return -1;
            }
            if (vi->remove_pre_vi_id(pre_vi_id) != 0 || pre_vi->remove_next_vi_id(vi_id) != 0) {
                warning_log("remove relation failed");
                return -1;
            }
        }
    }
    if (function_id < chain->get_length()-1) {
        for (auto iter = vnf_instance[function_id+1]->begin();
                iter != vnf_instance[function_id+1]->end(); ++iter) {
            int next_vi_id = *iter;
            debug_log("no exceeded");
            if (get_vnf_instance(next_vi_id, &next_vi) != 0) {
                warning_log("get next vi failed");
                return -1;
            }
            debug_log("get next vi done");
            debug_log("cur_vi info: %s", vi->to_string().c_str());
            debug_log("next_vi info: %s", next_vi->to_string().c_str());
            if (vi->remove_next_vi_id(next_vi_id) != 0 || next_vi->remove_pre_vi_id(vi_id) != 0) {
                warning_log("remove relation failed");
                return -1;
            }
            debug_log("remove relation done");
        }
    }

    bool found = false;
    for (auto iter = vnf_instance[function_id]->begin();
            iter != vnf_instance[function_id]->end(); ++iter) {
        if (*iter == vi_id) {
            found = true;
            vnf_instance[function_id]->erase(iter);
            break;
        }
    }

    if (found == false) {
        warning_log("not existed vi_id. chain_id=%d, function_id=%d, vi_id=%d", chain->get_id(), function_id, vi_id);
        return -1;
    }

    return 0;
}

int ServiceChainManager::get_related_pn_id(ServiceChain *chain, std::set<int> &pn_id_set)
{
    pn_id_set.clear();

    VnfInstance *vi(NULL);
    for (int i = 0; i < chain->get_length(); ++i) {
        auto &vnf_instances = chain->get_vnf_instance(i);
        for (auto iter = vnf_instances.begin(); iter != vnf_instances.end(); ++iter) {
            int vi_id = *iter;
            if (get_vnf_instance(vi_id, &vi) != 0) {
                warning_log("get vi failed");
                return -1;
                //return result;
            }
            if (vi->is_settled()) {
                pn_id_set.insert(vi->get_location());
            }
        }
    }

    return 0;
}


int ServiceChainManager::assign_vi_resource(int vi_id, int cpu_used, int memory_used)
{
    VnfInstance *vi(NULL);
    if (get_vnf_instance(vi_id, &vi) != 0) {
        warning_log("get vi failed");
        return -1;
    }

    if (vi->assign_vi_resource(cpu_used, memory_used) != 0) {
        warning_log("vi assign_vi_resource failed");
        return -1;
    }

    return 0;
}

int ServiceChainManager::release_vi_resource(int vi_id, int cpu_used, int memory_used)
{
    VnfInstance *vi(NULL);
    if (get_vnf_instance(vi_id, &vi) != 0) {
        warning_log("get vi failed");
        return -1;
    }

    if (vi->release_vi_resource(cpu_used, memory_used) != 0) {
        warning_log("vi release_vi_resource failed");
        return -1;
    }

    return 0;
}

int ServiceChainManager::get_cpu_statistics(int vi_id, int &cpu_used, int &cpu)
{
    VnfInstance *vi(NULL);
    if (get_vnf_instance(vi_id, &vi) != 0) {
        warning_log("get vi failed");
        return -1;
    }

    cpu_used = vi->get_cpu_used();
    cpu = vi->get_cpu_cost();

    return 0;
}
int ServiceChainManager::get_memory_statistics(int vi_id, int &memory_used, int &memory)
{
    VnfInstance *vi(NULL);
    if (get_vnf_instance(vi_id, &vi) != 0) {
        warning_log("get vi failed");
        return -1;
    }

    memory_used = vi->get_memory_used();
    memory = vi->get_memory_cost();

    return 0;
}

}
