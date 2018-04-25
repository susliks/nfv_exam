#include <cstdio>
#include <string>
#include <map>
#include <vector>
#include <cstdlib>

#include "physical_node_manager.h"
#include "physical_node.h"
#include "flow_manager.h"
#include "vnf_instance.h"
#include "req_manager.h"
#include "log.h"

using namespace std;

const int LEVEL0_SON_NUM = 20;
const int LEVEL1_SON_NUM = 20;
const int LEVEL2_SON_NUM = 40;
const int SERVER_CPU = 10;
const int SERVER_MEMORY = 20;
const int SERVER_UP_BANDWIDTH = 30;
const int LEVEL0_BANDWIDTH_DECAY = 4;
const int LEVEL1_BANDWIDTH_DECAY = 4;

const std::string flow_template_file_path = "./data/flow_template.config";
const int new_req_procedure_count = 1;
const int adjust_req_procedure_count = 1;

int main()
{
    notice_log("log start");
    nfv_exam::PhysicalNodeManager *physical_node_manager = nfv_exam::PhysicalNodeManager::get_instance();
    physical_node_manager->build_3_level_topo(LEVEL0_SON_NUM, LEVEL1_SON_NUM, LEVEL2_SON_NUM, SERVER_CPU, SERVER_MEMORY, \
            SERVER_UP_BANDWIDTH, LEVEL0_BANDWIDTH_DECAY, LEVEL1_BANDWIDTH_DECAY);

    nfv_exam::ReqManager *req_manager = nfv_exam::ReqManager::get_instance();
    req_manager->set_flow_template_file_path(flow_template_file_path);
    req_manager->set_generate_procedure_count(new_req_procedure_count, adjust_req_procedure_count);
    req_manager->init();
    
    printf("run success\n");
    return 0;
}
