#include <cmath>
#include <algorithm>

#include "req_manager.h"
#include "physical_node_manager.h"
#include "service_chain_manager.h"
#include "flow_manager.h"
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

int ReqManager::set_cpu_enlarge_factor(int cpu_enlarge_factor)
{
    this->cpu_enlarge_factor = cpu_enlarge_factor;
    return 0;
}

int ReqManager::set_flow_template_file_path(const std::string &file_path)
{
    this->flow_template_file_path = file_path;
    return 0;
}

int ReqManager::set_req_evaluation_file_path(const std::string &file_path)
{
    this->req_evaluation_file_path = file_path;
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

    if (build_length2flow_template_id() != 0) {
        warning_log("build_length2flow_template_id failed");
        return -1;
    }

    if (generate_timestamp() != 0) {
        warning_log("generate timestamp failed");
        return -1;
    }


    //todo
    //
    return 0;
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

        //int flow_template_id = this->flow_template.size();

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
    //int cpu_enlargement_factor = 100;

    int new_flow_template_id = this->flow_template.size();
    
    this->flow_template[new_flow_template_id] = new FlowTemplate;
    this->flow_template[new_flow_template_id]->length = length;
    this->flow_template[new_flow_template_id]->flow_bandwidth_cost = flow_bandwidth_cost;
    for (int i = 0; i < length; ++i) {
        this->flow_template[new_flow_template_id]->node_cpu_cost.push_back(
                floor(node_cpu_cost[i] * this->cpu_enlarge_factor + eps));
        this->flow_template[new_flow_template_id]->node_memory_cost.push_back(node_memory_cost[i]);
    }
    notice_log("add a flow_template succussful:%s", this->flow_template[new_flow_template_id]->to_string().c_str());
    return 0;
}

int ReqManager::generate_timestamp()
{
    for (int i = 0; i < this->new_req_procedure_count; ++i) {
        new_req_timestamp.push_back(std::vector<int>());
        if (generate_poisson_seq(this->exam_lifetime, new_req_timestamp.back()) != 0) {
            warning_log("generate poisson seq failed");
            return -1;
        }
        new_req_timestamp_ptr.push_back(0);
        notice_log("gen new req done. procedure = %d.", i);
        std::string new_log_str;
        int new_req_timestamp_size = new_req_timestamp[i].size();
        for (int j = 0; j < fmin(30, new_req_timestamp_size); ++j) {
            new_log_str = new_log_str + std::string(" ") + std::to_string(new_req_timestamp[i][j]);
        }
        notice_log("sequence: %s ...", new_log_str.c_str());
    }
    

    for (int i = 0; i < this->adjust_req_procedure_count; ++i) {
        adjust_req_timestamp.push_back(std::vector<int>());
        if (generate_poisson_seq(this->exam_lifetime, adjust_req_timestamp.back()) != 0) {
            warning_log("generate poisson seq failed");
            return -1;
        }
        adjust_req_timestamp_ptr.push_back(0);
        notice_log("gen adjust req done. procedure = %d.", i);
        std::string adjust_log_str;
        int adjust_req_timestamp_size = adjust_req_timestamp[i].size();
        for (int j = 0; j < fmin(30, adjust_req_timestamp_size); ++j) {
            adjust_log_str = adjust_log_str + std::string(" ") + std::to_string(adjust_req_timestamp[i][j]);
        }
        notice_log("sequence: %s ...", adjust_log_str.c_str());
    }
    notice_log("new_req_timestamp size = %d", this->new_req_timestamp.size());
    notice_log("new_req_timestamp_ptr size = %d", this->new_req_timestamp_ptr.size());
    notice_log("adjust_req_timestamp size = %d", this->adjust_req_timestamp.size());
    notice_log("adjust_req_timestamp_ptr size = %d", this->adjust_req_timestamp_ptr.size());

    return 0;
}

double ReqManager::random_exponential(double lambda)    
{
    double pV = 0.0;
    while(true)
    {
        pV = (double)rand() / (double)RAND_MAX;
        if (pV != 1)
        {
            break;
        }
    }
    pV = (-1.0 / lambda) * log(1 - pV);
    return pV;
}

int ReqManager::generate_poisson_seq(int exam_lifetime, std::vector<int> &req_timestamp)
{
    std::vector<double> local_timestamp;
    local_timestamp.push_back(0);
    while(local_timestamp.back() <= exam_lifetime + 10) {
        local_timestamp.push_back(local_timestamp.back() + random_exponential(0.36));
    }
    int local_timestamp_size = local_timestamp.size();
    req_timestamp.clear();
    for (size_t i = 0; i < local_timestamp_size; ++i) {
        req_timestamp.push_back(ceil(local_timestamp[i]));
    }
    return 0;
}

int ReqManager::set_exam_lifetime(int exam_lifetime)
{
    this->exam_lifetime = exam_lifetime;
    return 0;
}

int ReqManager::generate_req(int cur_time, std::vector<Req> &req_list)
{
    req_list.clear();

    Req req;
    for (int i = 0; i < this->new_req_procedure_count; ++i) {
        while (new_req_timestamp[i][new_req_timestamp_ptr[i]] == cur_time) {
            req.clear();
            if (generate_req_info("new", req) != 0) {
                warning_log("generate new req failed");
                return -1;
            }
            req_list.push_back(req);
            new_req_timestamp_ptr[i] += 1;
        }
    }
    debug_log("new req done");

    for (int i = 0; i < this->adjust_req_procedure_count; ++i) {
        while (adjust_req_timestamp[i][adjust_req_timestamp_ptr[i]] == cur_time) {

            //trick
            if (cur_time < 20) {
                adjust_req_timestamp_ptr[i] += 1;
                continue;
            }

            req.clear();
            int return_value = generate_req_info("adjust", req);
            if (return_value == -1) {
                warning_log("generate adjust req failed");
                return -1;
            } else if (return_value == 1) {
                notice_log("no active flow, skip this adjust part");
                continue;
            }
            req_list.push_back(req);
            adjust_req_timestamp_ptr[i] += 1;
        }
    }
    debug_log("adjust req done");

    notice_log("new req count = %d. req info:", req_list.size());
    int req_list_size = req_list.size();
    for (int i = 0; i < req_list_size; ++i) {
        notice_log("req %d: %s", i, req_list[i].to_string().c_str());
    }

    return 0;
}

int ReqManager::generate_req_info(const std::string &req_type, Req &req)
{
    if (req_type == "adjust") {
        req.set_req_type(req_type);
        int flow_id(-1), flow_length(-1);
        int return_value = random_pick_an_active_flow(flow_id, flow_length);
        if (return_value == -1) {
            warning_log("random_pick_an_active_flow failed");
            return -1;
        } else if (return_value == 1) {
            notice_log("skip");
            return 1;
        }
        req.set_chain_id(-1);
        req.set_flow_id(flow_id);
        req.set_length(flow_length);
        req.set_flow_template_id(get_random_template_id(flow_length));
    } else if (req_type == "new") {
        req.set_req_type(req_type);
        req.set_flow_template_id(get_random_template_id());
        req.set_length(this->flow_template[req.get_flow_template_id()]->length);
        int chain_id(-1);
        if (decide_chain_id(req.get_length(), chain_id) != 0) {
            warning_log("decide_chain_id failed");
            return -1;
        }
        req.set_flow_id(-1);
        req.set_chain_id(chain_id);
    } else {
        warning_log("illegal req type");
        return -1;
    }

    req.set_lifetime(ceil(random_exponential((double)1/3600)));
    //TODO:debug!
    //req.set_lifetime(1000000);
    notice_log("generate req info done");

    return 0;
}

int ReqManager::random_pick_an_active_flow(int &flow_id, int &flow_length)
{
    FlowManager *flow_manager(NULL); 
    if ((flow_manager = FlowManager::get_instance()) == NULL) {
        warning_log("get instance failed");
        return -1;
    }
    
    int return_value = flow_manager->random_pick_an_active_flow_id(flow_id, flow_length);
    if (return_value == -1) {
        warning_log("random_pick_an_active_flow_id failed");
        return -1;
    } else if (return_value == 1) {
        notice_log("skip");
        return 1;
    }

    return 0;
}

int ReqManager::random_pick_an_active_chain(int length, int &chain_id)
{
    ServiceChainManager *service_chain_manager(NULL); 
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get instance failed");
        return -1;
    }

    std::vector<int> active_chain_id;
    if (service_chain_manager->get_all_active_chain_id(length, active_chain_id) != 0) {
        warning_log("get active chain id failed");
        return -1;
    }
    
    int chain_count = active_chain_id.size();
    if (chain_count == 0) {
        warning_log("no active chain");
        return -1;
    }

    int position = rand() % chain_count;
    chain_id = active_chain_id[position];
    return 0;
}

int ReqManager::decide_chain_id(int length, int &chain_id) 
{
    PhysicalNodeManager *physical_node_manager(NULL); 
    if ((physical_node_manager = PhysicalNodeManager::get_instance()) == NULL) {
        warning_log("get instance failed");
        return -1;
    }

    double resource_used_ratio = physical_node_manager->get_resource_used_ratio();
    //int enlargement_factor = 100;
    //int threshold = floor(resource_used_ratio * enlargement_factor);
    //int random_int = rand() % enlargement_factor;
    int threshold = floor(resource_used_ratio * this->cpu_enlarge_factor);
    int random_int = rand() % this->cpu_enlarge_factor;
    notice_log("threshold:%d, random_int:%d", threshold, random_int);
    if (random_int > threshold) {
        chain_id = -1;
        notice_log("new req in new chain");
    } else {
        if (random_pick_an_active_chain(length, chain_id) != 0) {
            warning_log("pick active chain of length %d failed, maybe it's real and ok", length);
            chain_id = -1;
        }
        notice_log("new req in old chain(at least, try to, not sure for success), chain_id = %d", chain_id);
    }
    
    return 0;
}

int ReqManager::get_random_int(int l, int r)
{
    return rand() % (r-l+1) + l;
}

int ReqManager::get_random_template_id()
{
    int flow_template_count = this->flow_template.size();
    return rand() % flow_template_count;
}

int ReqManager::get_random_template_id(int length)
{
    int flow_template_count = this->length2flow_template_id[length].size();
    return this->length2flow_template_id[length][rand()%flow_template_count];
}

int ReqManager::build_length2flow_template_id()
{
    int flow_template_count = flow_template.size();
    //for (auto iter = flow_template.begin(); iter != flow_template.end(); ++iter) {
    for (int i = 0; i < flow_template_count; ++i) {
        int len = this->flow_template[i]->length;
        if (this->length2flow_template_id.find(len) == length2flow_template_id.end()) {
            length2flow_template_id[len] = std::vector<int>();
        }
        length2flow_template_id[len].push_back(i);
    }
    return 0;
}

int ReqManager::update_evaluation(const std::vector<bool> &req_accepted_flag)
{
    for (auto iter = req_accepted_flag.begin(); iter != req_accepted_flag.end(); ++iter) {
        this->total_req_count += 1;
        if (*iter == true) {
            this->accepted_req_count += 1;
        }
    }
    
    if (this->total_req_count > 0) {
        accept_ratio_history.push_back((double)this->accepted_req_count / this->total_req_count);
    }
    return 0;
}

int ReqManager::save_evaluation()
{
    FILE *out_file(NULL);
    if ((out_file = fopen(this->req_evaluation_file_path.c_str(), "w")) == NULL) {
        warning_log("open req evaluation file failed");
        return -1;
    }

    fprintf(out_file, "%d / %d\n", this->accepted_req_count, this->total_req_count);
    for (auto iter = this->accept_ratio_history.begin(); iter != this->accept_ratio_history.end(); ++iter) {
        fprintf(out_file, "%lf\n", *iter);
    }

    fclose(out_file);
    return 0;
}

int ReqManager::get_template_info(int flow_template_id, int &length, std::vector<int> &node_cpu_cost, 
            std::vector<int> &node_memory_cost, int &flow_bandwidth_cost)
{
    if (this->flow_template.find(flow_template_id) == this->flow_template.end()) {
        warning_log("not exist template_id = %d", flow_template_id);
        return -1;
    }

    length = flow_template[flow_template_id]->length;
    node_cpu_cost = flow_template[flow_template_id]->node_cpu_cost;
    node_memory_cost = flow_template[flow_template_id]->node_memory_cost;
    flow_bandwidth_cost = flow_template[flow_template_id]->flow_bandwidth_cost;

    return 0;
}



}
