#include <cstdio>
#include <string>
#include <map>
#include <vector>
#include <cstdlib>
#include <ctime>

//#include "vnf_instance.h"
//#include "physical_node.h"

//#include "flow_manager.h"
//#include "service_chain_manager.h"
//#include "physical_node_manager.h"
//#include "req_manager.h"
#include "scheduler.h"
#include "log.h"

using namespace std;

const int cpu_enlarge_factor = 100;

const int LEVEL0_SON_NUM = 4;
const int LEVEL1_SON_NUM = 20;
const int LEVEL2_SON_NUM = 20;
const int SERVER_CPU = 12 * cpu_enlarge_factor;
const int SERVER_MEMORY = 32000;
const int SERVER_UP_BANDWIDTH = 1000;
const int LEVEL0_BANDWIDTH_DECAY = 2;
const int LEVEL1_BANDWIDTH_DECAY = 2;


const std::string flow_template_file_path = "./data/flow_template.config";
const std::string flow_evaluation_file_path = "./data/flow_evaluation";
const std::string req_evaluation_file_path = "./data/req_evaluation";
const std::string physical_node_cpu_evaluation_file_path = "./data/cpu_evaluation";
const std::string physical_node_memory_evaluation_file_path = "./data/memory_evaluation";
const std::string physical_node_bandwidth_evaluation_file_path = "./data/bandwidth_evaluation";
const int physical_node_evaluate_frequency = 10;
const int new_req_procedure_count = 6;
const int adjust_req_procedure_count = 6;

const int exam_lifetime = 36000;

//scheduler
const double scheduler_alpha = 1.0;
const std::string scheduler_strategy = "normal";
//const std::string scheduler_strategy = "vertical_only";
//const std::string scheduler_strategy = "horizontal_only";

int main()
{
    srand((unsigned)time(NULL));

    notice_log("log start");
    debug_log("debug_log test");

    nfv_exam::PhysicalNodeManager *physical_node_manager = nfv_exam::PhysicalNodeManager::get_instance();
    physical_node_manager->set_physical_node_cpu_evaluation_file_path(physical_node_cpu_evaluation_file_path);
    physical_node_manager->set_physical_node_memory_evaluation_file_path(physical_node_memory_evaluation_file_path);
    physical_node_manager->set_physical_node_bandwidth_evaluation_file_path(physical_node_bandwidth_evaluation_file_path);
    physical_node_manager->build_3_level_topo(LEVEL0_SON_NUM, LEVEL1_SON_NUM, LEVEL2_SON_NUM, SERVER_CPU, SERVER_MEMORY, \
            SERVER_UP_BANDWIDTH, LEVEL0_BANDWIDTH_DECAY, LEVEL1_BANDWIDTH_DECAY);

    nfv_exam::ReqManager *req_manager = nfv_exam::ReqManager::get_instance();
    req_manager->set_flow_template_file_path(flow_template_file_path);
    req_manager->set_req_evaluation_file_path(req_evaluation_file_path);
    req_manager->set_generate_procedure_count(new_req_procedure_count, adjust_req_procedure_count);
    req_manager->set_exam_lifetime(exam_lifetime);
    req_manager->set_cpu_enlarge_factor(cpu_enlarge_factor);
    req_manager->init();

    nfv_exam::FlowManager *flow_manager = nfv_exam::FlowManager::get_instance();
    flow_manager->set_flow_evaluation_file_path(flow_evaluation_file_path);
    flow_manager->init();

    nfv_exam::ServiceChainManager *service_chain_manager = nfv_exam::ServiceChainManager::get_instance();
    service_chain_manager->set_cpu_enlarge_factor(cpu_enlarge_factor);
    service_chain_manager->init();

    nfv_exam::Scheduler scheduler;
    if (scheduler.set_strategy(scheduler_strategy) != 0) {
        warning_log("set strategy failed");
        return -1;
    }
    scheduler.set_alpha(scheduler_alpha);
    scheduler.init();

    std::vector<nfv_exam::Req> req_list;
    std::vector<bool> req_accepted_flag;

    for (int cur_time = 0; cur_time < exam_lifetime; ++cur_time) {
        printf("%d\n", cur_time);

        if (req_manager->generate_req(cur_time, req_list) != 0) {
            warning_log("generate req failed");
            return -1;
        }
        debug_log("gen req done");
        notice_log("gen req done");

        if (scheduler.handle_req_list(req_list, req_accepted_flag) != 0) {
            warning_log("handle req list failed");
            return -1;
        }
        debug_log("handle req done");
        notice_log("handle req done");

        if (req_manager->update_evaluation(req_accepted_flag) != 0) {
            warning_log("update accepted flag into Req failed");
            return -1;
        }

        //todo


        if (cur_time % physical_node_evaluate_frequency == 0) {
            physical_node_manager->update_resource_used_evaluation();
            flow_manager->update_evaluation();

            //TODO: for debug
            physical_node_manager->save_evaluation();
            req_manager->save_evaluation();
            flow_manager->save_evaluation();
        }
    }

    physical_node_manager->save_evaluation();
    req_manager->save_evaluation();
    flow_manager->save_evaluation();
    
    printf("run success\n");
    return 0;
}
