#ifndef SERVICE_CHAIN_MANAGER_H
#define SERVICE_CHAIN_MANAGER_H

//#include <cstdio>
#include <vector>
#include <map>

#include "vnf_instance.h"
#include "service_chain.h"

namespace nfv_exam {

class ServiceChainManager {
public:
    ~ServiceChainManager();

    static ServiceChainManager *get_instance();



private:

private:
    std::map<int, ServiceChain*> service_chain_pool;
    int service_chain_id_count;
    std::map<int, VnfInstance*> vnf_instance_pool;
    int vnf_instance_id_count;

};

}

#endif /* SERVICE_CHAIN_MANAGER_H */

