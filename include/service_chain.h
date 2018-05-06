#ifndef SERVICE_CHAIN_H
#define SERVICE_CHAIN_H

#include <vector>
#include <map>
#include <set>

namespace nfv_exam {

class ServiceChain {
public:
    int init(int id, int length, int bandwidth_used);
    int get_length();
    int add_vnf_instance(int function_id, int vi_id);
    int remove_vnf_instance(int function_id, int vi_id);
    //const std::map<int, std::vector<int> *> &get_vnf_instance();
    std::map<int, std::vector<int> *> &get_vnf_instance();
    std::vector<int> &get_vnf_instance(int function_id);
    int get_first_vi_id(int function_id); // only work in the FF

    int set_bandwidth_used(int bandwidth_used);
    int get_bandwidth_used();

    int get_related_pn_id(std::set<int> &pn_id_set);


private:
    int id;
    int length;
    std::map<int, std::vector<int> *> vnf_instance; //from function_id to vnf_instance_id

    int bandwidth_used;

};

}







#endif /* SERVICE_CHAIN_H */
