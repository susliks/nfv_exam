#include "physical_node_manager.h"
#include "log.h"

namespace nfv_exam {

PhysicalNodeManager::~PhysicalNodeManager()
{
    for (std::map<int, PhysicalNode *>::iterator itr = physical_node.begin();
            itr != physical_node.end(); ++itr) {
        if (itr->second != NULL) {
            delete itr->second;
        }
    }
}

PhysicalNodeManager *PhysicalNodeManager::get_instance()
{
    static PhysicalNodeManager *instance(NULL);

    if (instance == NULL) {
        instance = new PhysicalNodeManager;
    }
    return instance;
}

int PhysicalNodeManager::get_server_count()
{
    return this->server_count;
}

int PhysicalNodeManager::get_server_cpu()
{
    return this->server_cpu;
}

int PhysicalNodeManager::get_server_memory()
{
    return this->server_memory;
}

int PhysicalNodeManager::build_3_level_topo(int level0_son_num, int level1_son_num, int level2_son_num, \
        int server_cpu, int server_memory, int server_up_bandwidth, \
        double level1_bandwidth_decay_rate, double level2_bandwidth_decay_rate)
{
    this->server_count = level0_son_num * level1_son_num * level2_son_num;
    this->server_cpu = server_cpu;
    this->server_memory = server_memory;

    int physical_node_num = 0;

    std::vector<int> level_son_num;
    level_son_num.push_back(level0_son_num);
    level_son_num.push_back(level1_son_num);
    level_son_num.push_back(level2_son_num);
    std::vector<int> level_node_num;
    level_node_num.push_back(1);
    for (int i = 1; i <= 3; i++) {
        level_node_num.push_back(level_node_num[i-1]*level_son_num[i-1]);
    }
    
    for (int i = 3; i >= 0; i--) {
        notice_log("newing physical node, level = %d, node_num = %d", i, level_node_num[i]);
        int level_up_bandwidth = 1;
        if (i == 2) {
            level_up_bandwidth = server_up_bandwidth * level_son_num[i] / level2_bandwidth_decay_rate;
        } else if (i == 1) { 
            level_up_bandwidth = server_up_bandwidth * level_son_num[i+1] / level2_bandwidth_decay_rate;
            level_up_bandwidth = level_up_bandwidth * level_son_num[i] / level1_bandwidth_decay_rate;
        }
        
        for (int j = 0; j < level_node_num[i]; j++) {
            if (i == 3) {
                physical_node[physical_node_num] = new PhysicalNode(physical_node_num, i, server_cpu, server_memory, server_up_bandwidth);
            } else if (i == 2 || i == 1) {
                physical_node[physical_node_num] = new PhysicalNode(physical_node_num, i, 0, 0, level_up_bandwidth);
            } else if(i == 0) {
                physical_node[physical_node_num] = new PhysicalNode(physical_node_num, i, 0, 0, 0);
            }
            physical_node_num++;
        }
    }
    notice_log("new physical node done");
    
    int son_id = 0;
    int parent_id = level_node_num[3];

    for (int k = 3; k >= 1; k--) {
        for (int i = 0; i < level_node_num[k]; i++) {
            physical_node[son_id]->set_parent(parent_id);
            physical_node[parent_id]->add_son(son_id);
            notice_log("build topo edge %d->%d", parent_id, son_id);
            son_id++;
            if ((i+1) % level_son_num[k-1] == 0) {
                parent_id++;
            }
        }
    }

    if (calculate_total_resource() != 0) {
        warning_log("calculate_total_resource failed");
        return -1;
    }

    return 0;
}

int PhysicalNodeManager::get_bandwidth_statistics(int physical_node_id_1, int physical_node_id_2, int &bandwidth_left)
{
    PhysicalNode physical_node_1;
    PhysicalNode physical_node_2;
    if (get_physical_node(physical_node_id_1, physical_node_1) != 0 ||
            get_physical_node(physical_node_id_2, physical_node_2) != 0) {
        warning_log("get physical node failed, id = %d & %d", physical_node_id_1, physical_node_id_2);
        return -1;
    }

    if (physical_node_1.get_depth() != 3 || physical_node_2.get_depth() != 3) {
        warning_log("both node should be level 3. node_id = %d & %d", 
                physical_node_id_1, physical_node_id_2);
        return -1;
    }

    int parent_id_1 = physical_node_id_1;
    int parent_id_2 = physical_node_id_2;
    hop_count = 0;
    bandwidth_left = 0x7fffffff;
    while (parent_id_1 != parent_id_2) {
        hop_count += 2;
        if (get_physical_node(parent_id_1, physical_node_1) != 0 ||
                get_physical_node(parent_id_2, physical_node_2) != 0) {
            warning_log("get physical node failed, id = %d & %d", parent_id_1, parent_id_2);
            return -1;
        }
        bandwidth_left = min(bandwidth_left, physical_node_1.get_up_bandwidth() - physical_node_1.get_up_bandwidth_used());
        bandwidth_left = min(bandwidth_left, physical_node_2.get_up_bandwidth() - physical_node_2.get_up_bandwidth_used());
        parent_id_1 = physical_node_1.get_parent_id();
        parent_id_2 = physical_node_2.get_parent_id();
    }

    return 0;
}

int PhysicalNodeManager::get_bandwidth_hop_count(int physical_node_id_1, int physical_node_id_2, int &hop_count)
{
    PhysicalNode physical_node_1;
    PhysicalNode physical_node_2;
    if (get_physical_node(physical_node_id_1, physical_node_1) != 0 ||
            get_physical_node(physical_node_id_2, physical_node_2) != 0) {
        warning_log("get physical node failed, id = %d & %d", physical_node_id_1, physical_node_id_2);
        return -1;
    }

    if (physical_node_1.get_depth() != 3 || physical_node_2.get_depth() != 3) {
        warning_log("both node should be level 3. node_id = %d & %d", 
                physical_node_id_1, physical_node_id_2);
        return -1;
    }

    int parent_id_1 = physical_node_id_1;
    int parent_id_2 = physical_node_id_2;
    hop_count = 0;
    while (parent_id_1 != parent_id_2) {
        hop_count += 2;
        if (get_physical_node(parent_id_1, physical_node_1) != 0 ||
                get_physical_node(parent_id_2, physical_node_2) != 0) {
            warning_log("get physical node failed, id = %d & %d", parent_id_1, parent_id_2);
            return -1;
        }
        parent_id_1 = physical_node_1.get_parent_id();
        parent_id_2 = physical_node_2.get_parent_id();
    }

    return 0;
}

int PhysicalNodeManager::assign_bandwidth(int physical_node_id_1, int physical_node_id_2, int bandwidth)
{    
    int parent_id_1 = physical_node_id_1;
    int parent_id_2 = physical_node_id_2;
    PhysicalNode physical_node_1;
    PhysicalNode physical_node_2;
    while (parent_id_1 != parent_id_2) {
        if (get_physical_node(parent_id_1, physical_node_1) != 0 ||
                get_physical_node(parent_id_2, physical_node_2) != 0) {
            warning_log("get physical node failed, id = %d & %d", parent_id_1, parent_id_2);
            return -1;
        }

        if (physical_node_1.assign_bandwidth(bandwidth) != 0) {
            warning_log("assign bandwidth failed, physical_node_id = %d", parent_id_1);
            return -1;
        }

        if (physical_node_2.assign_bandwidth(bandwidth) != 0) {
            warning_log("assign bandwidth failed, physical_node_id = %d", parent_id_2);
            return -1;
        }

        parent_id_1 = physical_node_1.get_parent_id();
        parent_id_2 = physical_node_2.get_parent_id();
    }

    return 0;
}

int PhysicalNodeManager::assign_host_resource(int physical_node_id, int cpu_cost, int memory_cost)
{
    PhysicalNode physical_node;
    if (get_physical_node(physical_node_id, physical_node) != 0) {
        warning_log("get physical_node failed, id = %d", physical_node_id);
        return -1;
    }

    if (physical_node.assign_host_resource(cpu_cost, memory_cost) != 0) {
        warning_log("assign physical_node host cost failed, physical_node_id = %d", physical_node_id);
        return -1;
    }

    return 0;
}

int PhysicalNodeManager::release_bandwidth(int physical_node_id_1, int physical_node_id_2, int bandwidth)
{
    int parent_id_1 = physical_node_id_1;
    int parent_id_2 = physical_node_id_2;
    PhysicalNode physical_node_1;
    PhysicalNode physical_node_2;
    while (parent_id_1 != parent_id_2) {
        if (get_physical_node(parent_id_1, physical_node_1) != 0 ||
                get_physical_node(parent_id_2, physical_node_2) != 0) {
            warning_log("get physical node failed, id = %d & %d", parent_id_1, parent_id_2);
            return -1;
        }

        if (physical_node_1.release_bandwidth(bandwidth) != 0) {
            warning_log("release bandwidth failed, physical_node_id = %d", parent_id_1);
            return -1;
        }

        if (physical_node_2.release_bandwidth(bandwidth) != 0) {
            warning_log("release bandwidth failed, physical_node_id = %d", parent_id_2);
            return -1;
        }

        parent_id_1 = physical_node_1.get_parent_id();
        parent_id_2 = physical_node_2.get_parent_id();
    }

    return 0;
}

int PhysicalNodeManager::release_host_resource(int physical_node_id, int cpu_cost, int memory_cost)
{
    PhysicalNode physical_node;
    if (get_physical_node(physical_node_id, physical_node) != 0) {
        warning_log("get physical_node failed, id = %d", physical_node_id);
        return -1;
    }

    if (physical_node.release_host_resource(cpu_cost, memory_cost) != 0) {
        warning_log("release physical_node host cost failed, physical_node_id = %d", physical_node_id);
        return -1;
    }

    return 0;
}

int PhysicalNodeManager::get_cpu_statistics(int pn_id, int &cpu_used, int &cpu)
{
    PhysicalNode physical_node;
    if (get_physical_node(pn_id, physical_node) != 0) {
        warning_log("get physical_node failed, id = %d", physical_node_id);
        return -1;
    }

    cpu_used = physical_node.get_cpu_used();
    cpu = physical_node.get_total_cpu_available();

    return 0;
}

int PhysicalNodeManager::get_memory_statistics(int pn_id, int &memory_used, int &memory)
{
    PhysicalNode physical_node;
    if (get_physical_node(pn_id, physical_node) != 0) {
        warning_log("get physical_node failed, id = %d", physical_node_id);
        return -1;
    }

    memory_used = physical_node.get_memory_used();
    memory = physical_node.get_total_memory_available();

    return 0;
}

int PhysicalNodeManager::get_bandwidth_statistics(int pn_id, int &up_bandwidth_used, int &up_bandwidth)
{
    PhysicalNode physical_node;
    if (get_physical_node(pn_id, physical_node) != 0) {
        warning_log("get physical_node failed, id = %d", physical_node_id);
        return -1;
    }

    up_bandwidth_used = physical_node.get_up_bandwidth_used();
    up_bandwidth = physical_node.get_total_bandwidth_available();

    return 0;
}

int PhysicalNodeManager::get_total_cpu_statistics(int &total_cpu_used, int &total_cpu)
{
    total_cpu_used = this->total_cpu_used;
    total_cpu = this->total_cpu;
    return 0;
}

int PhysicalNodeManager::get_total_memory_statistics(int &total_memory_used, int &total_memory)
{
    total_memory_used = this->total_memory_used;
    total_memory = this->total_memory;
    return 0;
}

int PhysicalNodeManager::get_total_bandwidth_statistics(int &total_bandwidth_used, int &total_bandwidth)
{
    total_bandwidth_used = this->total_bandwidth_used;
    total_bandwidth = this->total_bandwidth;
    return 0;
}

int PhysicalNodeManager::get_total_bandwidth()
{
    return this->total_bandwidth;
}

int PhysicalNodeManager::assign_total_host_resource(int cpu_cost, int memory_cost)
{
    this->total_cpu_used += cpu_cost;
    this->total_memory_used += memory_cost;
    return 0;
}

int PhysicalNodeManager::assign_total_bandwidth(int bandwidth_cost)
{
    this->total_bandwidth_used += bandwidth_cost;
    return 0;
}

int PhysicalNodeManager::release_total_host_resource(int cpu_cost, int memory_cost)
{
    this->total_cpu_used -= cpu_cost;
    this->total_memory_used -= memory_cost;
    return 0;
}

int PhysicalNodeManager::release_total_bandwidth(int bandwidth_cost)
{
    this->total_bandwidth_used -= bandwidth_cost;
    return 0;
}

int PhysicalNodeManager::calculate_total_cpu_cost()
{
    int result = 0;
    for (std::map<int, PhysicalNode *>::iterator itr = physical_node.begin();
            itr != physical_node.end(); ++itr) {
        if (itr->second != NULL && itr->second->get_depth() == 3) {
            result += itr->second->get_cpu_used();
        }
    }
    return result;
}

int PhysicalNodeManager::calculate_total_memory_cost()
{
    int result = 0;
    for (std::map<int, PhysicalNode *>::iterator itr = physical_node.begin();
            itr != physical_node.end(); ++itr) {
        if (itr->second != NULL && itr->second->get_depth() == 3) {
            result += itr->second->get_memory_used();
        }
    }
    return result;
}

int PhysicalNodeManager::calculate_total_bandwidth_cost()
{
    int result = 0;
    for (std::map<int, PhysicalNode *>::iterator itr = physical_node.begin();
            itr != physical_node.end(); ++itr) {
        if (itr->second != NULL) {
            result += itr->second->get_up_bandwidth_used();
        }
    }
    return result;
}

int PhysicalNodeManager::calculate_total_resource()
{
    this->total_cpu = 0;
    for (std::map<int, PhysicalNode *>::iterator itr = physical_node.begin();
            itr != physical_node.end(); ++itr) {
        if (itr->second != NULL && itr->second->get_depth() == 3) {
            this->total_cpu += itr->second->get_total_cpu_available();
        }
    }
    notice_log("total cpu: %d", this->total_cpu);

    this->total_memory = 0;
    for (std::map<int, PhysicalNode *>::iterator itr = physical_node.begin();
            itr != physical_node.end(); ++itr) {
        if (itr->second != NULL && itr->second->get_depth() == 3) {
            this->total_memory += itr->second->get_total_memory_available();
        }
    }
    notice_log("total memory: %d", this->total_memory);

    this->total_bandwidth = 0;
    for (std::map<int, PhysicalNode *>::iterator itr = physical_node.begin();
            itr != physical_node.end(); ++itr) {
        if (itr->second != NULL) {
            this->total_bandwidth += itr->second->get_total_bandwidth_available();
        }
    }
    notice_log("total bandwidth: %d", this->total_bandwidth);

    return 0;
}

int PhysicalNodeManager::get_physical_node(int node_id, PhysicalNode &physical_node)
{
    if (this->physical_node.find(node_id) != this->physical_node.end()) {
        physical_node = *this->physical_node[node_id];
        return 0;
    } else {
        warning_log("physical_node not found, node_id = %d", node_id);
        return -1;
    }
}

int PhysicalNodeManager::update_resource_used_evaluation()
{
    this->total_cpu_used = calculate_total_cpu_cost();
    this->total_memory_used = calculate_total_memory_cost();
    this->total_bandwidth_used = calculate_total_bandwidth_cost();

    total_cpu_used_ratio_history.push_back((double)total_cpu_used / total_cpu);
    total_memory_used_ratio_history.push_back((double)total_memory_used / total_memory);
    total_bandwidth_used_ratio_history.push_back((double)total_bandwidth_used / total_bandwidth);

    this->resource_used_ratio = (total_cpu_used_ratio_history.back() + 
            total_memory_used_ratio_history.back() + total_bandwidth_used_ratio_history.back()) / 3;

    return 0;
}

double PhysicalNodeManager::get_resource_used_ratio()
{
    return this->resource_used_ratio;
}

int PhysicalNodeManager::set_physical_node_cpu_evaluation_file_path(const std::string &file_path)
{
    this->physical_node_cpu_evaluation_file_path = file_path;
    return 0;
}

int PhysicalNodeManager::set_physical_node_memory_evaluation_file_path(const std::string &file_path)
{
    this->physical_node_memory_evaluation_file_path = file_path;
    return 0;
}

int PhysicalNodeManager::set_physical_node_bandwidth_evaluation_file_path(const std::string &file_path)
{
    this->physical_node_bandwidth_evaluation_file_path = file_path;
    return 0;
}

int PhysicalNodeManager::save_evaluation()
{
    FILE *out_file(NULL);
    if ((out_file = fopen(this->physical_node_cpu_evaluation_file_path.c_str(), "w")) == NULL) {
        warning_log("open cpu evaluation file failed");
        return -1;
    }
    for (auto iter = total_cpu_used_ratio_history.begin(); iter != total_cpu_used_ratio_history.end(); ++iter) {
        fprintf(out_file, "%lf\n", *iter);
    }
    fclose(out_file);
    
    if ((out_file = fopen(this->physical_node_memory_evaluation_file_path.c_str(), "w")) == NULL) {
        warning_log("open memory evaluation file failed");
        return -1;
    }
    for (auto iter = total_memory_used_ratio_history.begin(); iter != total_memory_used_ratio_history.end(); ++iter) {
        fprintf(out_file, "%lf\n", *iter);
    }
    fclose(out_file);
    
    if ((out_file = fopen(this->physical_node_bandwidth_evaluation_file_path.c_str(), "w")) == NULL) {
        warning_log("open bandwidth evaluation file failed");
        return -1;
    }
    for (auto iter = total_bandwidth_used_ratio_history.begin(); iter != total_bandwidth_used_ratio_history.end(); ++iter) {
        fprintf(out_file, "%lf\n", *iter);
    }
    fclose(out_file);
    
    return 0;
}


}
