#include "req_manager.h"
#include "log.h"

namespace nfv_exam {

ReqManager::~ReqManager()
{
    for (std::map<int, FlowTemplate *>::iterator itr = flow_template.begin();
            itr != flow_template.end(); ++itr) {
        if (itr->second != NULL) {
            delete itr->second;
        }
    }
}

ReqManager *ReqManager::get_instance()
{
    static ReqManager *instance(NULL);

    if (instance == NULL) {
        instance = new ReqManager;
    }
    return instance;
}

int ReqManager::set_flow_template_file_path(std::string file_path)
{
    this->flow_template_file_path = file_path;
    return 0;
}

int ReqManager::set_generate_procedure_count(int new_req_procedure_count, int adjust_req_procedure_count)
{
    this->new_req_procedure_count = new_req_procedure_count;
    this->adjust_req_procedure_count = adjust_req_procedure_count;
    return 0;
}

int ReqManager::init()
{
    this->accepted_req_count = 0;
    this->total_req_count = 0;

    load_flow_template_from_txt();
    //todo
    //
}

int ReqManager::load_flow_template_from_txt()
{
    FILE *in_file(NULL);
    if ((in_file = fopen(this->flow_template_file_path.c_str(), "r")) == NULL) {
        warning_log("open flow_template file failed");
        return -1;
    }
    notice_log("open file succuss");

    //c style file operation
    //not elegant
    int n;
    fscanf(in_file, "%d", &n);
    std::vector<double> cpu_cost;
    std::vector<int> memory_cost;
    for (int i = 0; i < n; ++i) {
        cpu_cost.clear();
        memory_cost.clear();
        int b;
        int length = -1;
        fscanf(in_file, "%d", &length);
        //int length = 4; //magic number

        int flow_template_id = this->flow_template.size();

        fscanf(in_file, "%d", &b);
        
        for (int j = 0; j < length; ++j) {
            double c;
            int m;
            fscanf(in_file, "%lf%d", &c, &m);
            cpu_cost.push_back(c);
            memory_cost.push_back(m);
        }
        notice_log("read success");

        if (add_flow_template(length, b, cpu_cost, memory_cost) != 0) {
            warning_log("add a flow template failed");
            return -1;
        }

        if (extend_flow_template_trick(length, b, cpu_cost, memory_cost) != 0) {
            warning_log("extend_flow_template failed");
            return -1;
        }
    }

    fclose(in_file);
    return 0;
}

//not elegant
int ReqManager::extend_flow_template_trick(int length, int flow_bandwidth_cost, 
            const std::vector<double> &node_cpu_cost, const std::vector<int> &node_memory_cost)
{
    for (int new_length = 1; new_length < length; ++new_length) {
        if (add_flow_template(new_length, flow_bandwidth_cost, node_cpu_cost, node_memory_cost) != 0) {
            warning_log("add a flow template failed");
            return -1;
        }
    }
    return 0;
}

int ReqManager::add_flow_template(int length, int flow_bandwidth_cost, 
            const std::vector<double> &node_cpu_cost, const std::vector<int> &node_memory_cost)
{
    //not elegant
    double eps = 1e-7;
    int cpu_enlargement_factor = 100;

    int new_flow_template_id = this->flow_template.size();
    
    this->flow_template[new_flow_template_id] = new FlowTemplate;
    this->flow_template[new_flow_template_id]->length = length;
    this->flow_template[new_flow_template_id]->flow_bandwidth_cost = flow_bandwidth_cost;
    for (int i = 0; i < length; ++i) {
        this->flow_template[new_flow_template_id]->node_cpu_cost.push_back(
                floor(node_cpu_cost[i] * cpu_enlargement_factor + eps));
        this->flow_template[new_flow_template_id]->node_memory_cost.push_back(node_memory_cost[i]);
    }
    notice_log("add a flow_template succussful:%s", this->flow_template[new_flow_template_id]->to_string().c_str());
    return 0;
}




}
