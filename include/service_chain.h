#ifndef SERVICE_CHAIN_H
#define SERVICE_CHAIN_H

#include <vector>
#include <map>

namespace nfv_exam {

class ServiceChain {
public:


private:
    int id;
    std::map<int, std::vector<int> > vnf_instance; //from function_id to vnf_instance_id

};

}







#endif /* SERVICE_CHAIN_H */
