#include "service_chain.h"
#include "log.h"

namespace nfv_exam {

int ServiceChain::init(int id, int length, int bandwidth_used)
{
    this->id = id;
    this->length = length;
    this->bandwidth_used = bandwidth_used;

    for (auto iter = vnf_instance.begin(); iter != vnf_instance.end(); ++iter) {
        delete iter->second;
    }
    this->vnf_instance.clear();

    for (int i = 0; i < length; i++) {
        vnf_instance[i] = new vector<int>;
    }
    return 0;
}

int ServiceChain::get_length()
{
    return this->length;
}

int ServiceChain::add_vnf_instance(int function_id, int vi_id)
{
    if (this->vnf_instance.find(function_id) == this->vnf_instance.end()) {
        warning_log("not exist funciton id: %d", function_id);
        return -1;
    }

    for (auto iter = this->vnf_instance[function_id]->begin();
            iter != this->vnf_instance[function_id]->end(); ++iter) {
        if (*iter == vi_id) {
            warning_log("existed vi_id. chain_id=%d, function_id=%d, vi_id=%d", this->id, function_id, vi_id);
            return -1;
        }
    }

    this->vnf_instance[function_id].push_back(vi_id);
    return 0;
}

const std::map<int, std::vector<int> *> &ServiceChain::get_vnf_instance()
{
    return this->vnf_instance;
}

int ServiceChain::set_bandwidth_used(int bandwidth_used)
{
    this->bandwidth_used = bandwidth_used;
    return 0;
}

int ServiceChain::get_first_vi_id(int function_id)
{
    if (this->vnf_instance.find(function_id) == this->vnf_instance.end()) {
        warning_log("not exist func id");
        return -1;
    }

    if (this->vnf_instance[function_id]->size() == 0) {
        warning_log("no vi in this func id");
        return -1;
    }

    return (*this->(vnf_instance[function_id]))[0];
}

int ServiceChain::get_bandwidth_used()
{
    return this->bandwidth_used;
}



}
