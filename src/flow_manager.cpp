#include "service_chain_manager.h"
#include "flow_manager.h"
#include "log.h"

namespace nfv_exam {

FlowManager::~FlowManager()
{
    for (std::map<int, Flow *>::iterator itr = flow_pool.begin();
            itr != flow_pool.end(); ++itr) {
        if (itr->second != NULL) {
            delete itr->second;
        }
    }

    for (std::map<int, FlowNode *>::iterator itr = flow_node_pool.begin();
            itr != flow_node_pool.end(); ++itr) {
        if (itr->second != NULL) {
            delete itr->second;
        }
    }
}

FlowManager *FlowManager::get_instance()
{
    static FlowManager *instance(NULL);

    if (instance == NULL) {
        instance = new FlowManager;
    }
    return instance;
}

std::map<int, Flow*> &FlowManager::get_flow_pool() //only for Scheduler::flow_aging()
{
    return this->flow_pool;
}

int FlowManager::get_flow(int flow_id, Flow **flow)
{
    if (flow_pool.find(flow_id) != flow_pool.end()) {
        (*flow) = flow_pool[flow_id];
        if (*flow == NULL) {
            warning_log("get flow failed, NULL ptr");
            return -1;
        }
        return 0;
    } else {
        warning_log("get flow failed, flow_id = %d", flow_id);
        return -1;
    }
}

int FlowManager::get_flow_node(int flow_node_id, FlowNode **flow_node)
{
    if (flow_node_pool.find(flow_node_id) != flow_node_pool.end()) {
        *flow_node = flow_node_pool[flow_node_id];
        if (*flow_node == NULL) {
            warning_log("get flow_node failed. NULL ptr");
            return -1;
        }
        return 0;
    } else {
        warning_log("get flow_node failed. flow_node_id = %d", flow_node_id);
        return -1;
    }
}

int FlowManager::random_pick_an_active_flow_id(int &flow_id, int &flow_length)
{
    int active_flow_count = this->flow_pool.size();
    if (active_flow_count == 0) {
        notice_log("no active_flow");
        return 1;
    }
    int position = rand() % active_flow_count;

    auto iter = this->flow_pool.begin();
    for (int i = 0; i < position; ++i) {
        ++iter;
        if (iter == this->flow_pool.end()) {
            warning_log("random pick failed.");
            return -1;
        }
    }

    flow_id = iter->first;
    flow_length = iter->second->get_length();
    return 0;
}

int FlowManager::set_flow_evaluation_file_path(const std::string &file_path)
{
    this->flow_evaluation_file_path = file_path;
    return 0;
}

int FlowManager::set_flow_cpu_evaluation_file_path(const std::string &file_path)
{
    this->flow_cpu_evaluation_file_path = file_path;
    return 0;
}

int FlowManager::set_final_result_file_path(const std::string &file_path)
{
    this->final_result_file_path = file_path;
    return 0;
}

int FlowManager::set_flow_memory_evaluation_file_path(const std::string &file_path)
{
    this->flow_memory_evaluation_file_path = file_path;
    return 0;
}

int FlowManager::save_evaluation()
{
    //throughput & flow_count
    FILE *out_file(NULL);
    if ((out_file = fopen(this->flow_evaluation_file_path.c_str(), "w")) == NULL) {
        warning_log("open flow evaluation file failed");
        return -1;
    }

    int active_flow_count_size = this->active_flow_count.size();
    if (active_flow_count_size != this->throughput_record.size()) {
        warning_log("upexpected situation: evaluation size not equal");
        return -1;
    }

    for (int i = 0; i < active_flow_count_size; ++i) {
        fprintf(out_file, "%d\t%d\n", throughput_record[i], active_flow_count[i]);
    }

    fclose(out_file);

    //cpu
    if ((out_file = fopen(this->flow_cpu_evaluation_file_path.c_str(), "w")) == NULL) {
        warning_log("open flow cpu evaluation file failed");
        return -1;
    }

    int cpu_ratio_size = this->total_cpu_used_ratio_history.size();
    for (int i = 0; i < cpu_ratio_size; ++i) {
        fprintf(out_file, "%f\n", this->total_cpu_used_ratio_history[i]);
    }

    fclose(out_file);

    //memory
    if ((out_file = fopen(this->flow_memory_evaluation_file_path.c_str(), "w")) == NULL) {
        warning_log("open flow memory evaluation file failed");
        return -1;
    }

    int memory_ratio_size = this->total_memory_used_ratio_history.size();
    for (int i = 0; i < memory_ratio_size; ++i) {
        fprintf(out_file, "%f\n", this->total_memory_used_ratio_history[i]);
    }

    fclose(out_file);

    return 0;
}

int FlowManager::save_final_result()
{
    FILE *out_file(NULL);
    if ((out_file = fopen(this->final_result_file_path.c_str(), "a")) == NULL) {
        warning_log("open final result file failed");
        return -1;
    }

    int cpu_ratio_size = this->total_cpu_used_ratio_history.size();
    double cpu_sum(0);
    int cpu_count = 0;
    for (int i = cpu_ratio_size/2; i < cpu_ratio_size; ++i) {
        cpu_sum += this->total_cpu_used_ratio_history[i];
        cpu_count += 1;
    }
    fprintf(out_file, "flow_cpu_ratio\n%f\n", cpu_sum / cpu_count);

    int memory_ratio_size = this->total_memory_used_ratio_history.size();
    double memory_sum(0);
    int memory_count = 0;
    for (int i = memory_ratio_size/2; i < memory_ratio_size; ++i) {
        memory_sum += this->total_memory_used_ratio_history[i];
        memory_count += 1;
    }
    fprintf(out_file, "flow_memory_ratio\n%f\n", memory_sum / memory_count);

    fclose(out_file);

    return 0;
}

int FlowManager::update_evaluation()
{
    //update bandwidth(throughput) & flow_count
    int flow_count = this->flow_pool.size();
    int throughput = 0;
    for (auto iter = flow_pool.begin(); iter != flow_pool.end(); ++iter) {
        throughput += (iter->second->get_length() - 1) * iter->second->get_flow_bandwidth();
    }

    this->active_flow_count.push_back(flow_count);
    this->throughput_record.push_back(throughput);

    //update cpu
    int total_cpu_used = get_flow_total_cpu_used();
    total_cpu_used_ratio_history.push_back((double)total_cpu_used / total_cpu);

    //update memory
    int total_memory_used = get_flow_total_memory_used();
    total_memory_used_ratio_history.push_back((double)total_memory_used / total_memory);

    return 0;
}

int FlowManager::init()
{
    this->flow_id_count = 0;
    this->flow_node_id_count = 0;

    PhysicalNodeManager *physical_node_manager(NULL);
    if ((physical_node_manager = PhysicalNodeManager::get_instance()) == NULL) {
        warning_log("get instance failed");
        return -1;
    }
    this->total_cpu = physical_node_manager->get_total_cpu();
    this->total_memory = physical_node_manager->get_total_memory();

    return 0;
}

int FlowManager::create_a_flow(int length, int chain_id, int lifetime, int flow_bandwidth, 
        const std::vector<int> &flow_node_cpu_cost, const std::vector<int> &flow_node_memory_cost, int &flow_id)
{
    this->flow_pool[flow_id_count] = new Flow;
    flow_id = flow_id_count;
    debug_log("flow_id = %d", flow_id);

    if (length != flow_node_cpu_cost.size() || length != flow_node_memory_cost.size()) {
        warning_log("flow nodes count inconsistent");
        return -1;
    }

    this->flow_pool[flow_id]->set_id(flow_id);
    this->flow_pool[flow_id]->set_chain_id(chain_id);
    this->flow_pool[flow_id]->set_lifetime_left(lifetime);
    this->flow_pool[flow_id]->set_length(length);
    this->flow_pool[flow_id]->set_flow_bandwidth(flow_bandwidth);
    std::vector<int> flow_nodes;
    int flow_node_id(0);
    for (int i = 0; i < length; ++i) {
        create_a_flow_node(flow_node_id);
        flow_node_pool[flow_node_id]->init(flow_node_id, flow_id, i, flow_node_cpu_cost[i], flow_node_memory_cost[i]);
        flow_nodes.push_back(flow_node_id);
    }
    for (int i = 0; i < length-1; ++i) {
        flow_node_pool[flow_nodes[i]]->set_next_flow_node_id(flow_nodes[i+1]);
        flow_node_pool[flow_nodes[i]]->set_next_flow_node(flow_node_pool[flow_nodes[i+1]]);
    }
    for (int i = 1; i < length; ++i) {
        flow_node_pool[flow_nodes[i]]->set_pre_flow_node_id(flow_nodes[i-1]);
        flow_node_pool[flow_nodes[i]]->set_pre_flow_node(flow_node_pool[flow_nodes[i-1]]);
    }

    this->flow_pool[flow_id]->set_flow_nodes(flow_nodes);

    flow_id_count += 1;
    return 0;
}

int FlowManager::create_a_flow_node(int &flow_node_id)
{
    this->flow_node_pool[flow_node_id_count] = new FlowNode;
    flow_node_id = flow_node_id_count;
    flow_node_id_count += 1;
    return 0;
}

int FlowManager::delete_a_flow(int flow_id)
{
    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get instance failed");
        return -1;
    }
    
    //FIXED: here has a bug. need to judge when should delete a chain. which vnf_instance

    int chain_id = this->flow_pool[flow_id]->get_chain_id();
    if (chain_id != -1) {
        if (service_chain_manager->remove_a_flow_from_a_chain(flow_id, chain_id) != 0) {
            warning_log("remove a flow from a chain failed");
            return -1;
        }
    }

    const std::vector<int> &flow_nodes = flow_pool[flow_id]->get_flow_nodes();
    for (auto iter = flow_nodes.begin(); iter != flow_nodes.end(); ++iter) {
        delete flow_node_pool[*iter];
        flow_node_pool.erase(*iter);
    }
    delete flow_pool[flow_id];
    flow_pool.erase(flow_id);

    return 0;
}

int FlowManager::get_flow_node(int flow_id, int func_id, FlowNode **flow_node)
{
    Flow *flow(NULL);
    if (get_flow(flow_id, &flow) != 0) {
        warning_log("get flow failed");
        return -1;
    }

    int flow_node_id(-1);
    if ((flow_node_id = flow->get_flow_node_id(func_id)) == -1) {
        warning_log("get flow node id failed");
        return -1;
    }

    if (get_flow_node(flow_node_id, flow_node) != 0) {
        warning_log("get flow node failed");
        return -1;
    }

    return 0;
}

int FlowManager::get_flow_total_cpu_used()
{
    int result = 0;
    for (auto iter = flow_node_pool.begin(); iter != flow_node_pool.end(); ++iter) {
        result += iter->second->get_cpu_cost();
    }
    return result;
}

int FlowManager::get_flow_total_memory_used()
{
    int result = 0;
    for (auto iter = flow_node_pool.begin(); iter != flow_node_pool.end(); ++iter) {
        result += iter->second->get_memory_cost();
    }
    return result;
}




}
