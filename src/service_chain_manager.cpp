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

}
