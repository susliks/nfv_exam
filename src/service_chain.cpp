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


    //set edge relation
    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get service_chain_manager failed");
        return -1;
    }

    VnfInstance *vi(NULL), *pre_vi(NULL), *next_vi(NULL);
    if (service_chain_manager->get_vnf_instance(vi_id, vi) != 0) {
        warning_log("get vi failed");
        return -1;
    }
    
    if (function_id > 0) {
        for (auto iter = this->vnf_instance[function_id-1]->begin();
                iter != this->vnf_instance[function_id-1]->end(); ++iter) {
            int pre_vi_id = *iter;
            if (service_chain_manager->get_vnf_instance(pre_vi_id, pre_vi) != 0) {
                warning_log("get pre vi failed");
                return -1;
            }
            if (vi->add_pre_vi_id(pre_vi_id) != 0 || pre_vi->add_next_vi_id(vi_id) != 0) {
                warning_log("add relation failed");
                return -1;
            }
        }
    }
    if (function_id < this->length-1) {
        for (auto iter = this->vnf_instance[function_id+1]->begin();
                iter != this->vnf_instance[function_id+1]->end(); ++iter) {
            int next_vi_id = *iter;
            if (service_chain_manager->get_vnf_instance(next_vi_id, next_vi) != 0) {
                warning_log("get next vi failed");
                return -1;
            }
            if (vi->add_next_vi_id(next_vi_id) != 0 || next_vi->add_pre_vi_id(vi_id) != 0) {
                warning_log("add relation failed");
                return -1;
            }
        }
    }

    return 0;
}

int ServiceChain::remove_vnf_instance(int function_id, int vi_id)
{
    if (this->vnf_instance.find(function_id) == this->vnf_instance.end()) {
        warning_log("not exist funciton id: %d", function_id);
        return -1;
    }

    //update edge relation
    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get service_chain_manager failed");
        return -1;
    }

    VnfInstance *vi(NULL), *pre_vi(NULL), *next_vi(NULL);
    if (service_chain_manager->get_vnf_instance(vi_id, vi) != 0) {
        warning_log("get vi failed");
        return -1;
    }
    if (function_id > 0) {
        for (auto iter = this->vnf_instance[function_id-1]->begin();
                iter != this->vnf_instance[function_id-1]->end(); ++iter) {
            int pre_vi_id = *iter;
            if (service_chain_manager->get_vnf_instance(pre_vi_id, pre_vi) != 0) {
                warning_log("get pre vi failed");
                return -1;
            }
            if (vi->remove_pre_vi_id(pre_vi_id) != 0 || pre_vi->remove_next_vi_id(vi_id) != 0) {
                warning_log("remove relation failed");
                return -1;
            }
        }
    }
    if (function_id < this->length-1) {
        for (auto iter = this->vnf_instance[function_id+1]->begin();
                iter != this->vnf_instance[function_id+1]->end(); ++iter) {
            int next_vi_id = *iter;
            if (service_chain_manager->get_vnf_instance(next_vi_id, next_vi) != 0) {
                warning_log("get next vi failed");
                return -1;
            }
            if (vi->remove_next_vi_id(next_vi_id) != 0 || next_vi->remove_pre_vi_id(vi_id) != 0) {
                warning_log("remove relation failed");
                return -1;
            }
        }
    }

    bool found = false;
    for (auto iter = this->vnf_instance[function_id]->begin();
            iter != this->vnf_instance[function_id]->end(); ++iter) {
        if (*iter == vi_id) {
            found = true;
            this->vnf_instance[function_id].erase(iter);
            break;
        }
    }

    if (found == false) {
        warning_log("not existed vi_id. chain_id=%d, function_id=%d, vi_id=%d", this->id, function_id, vi_id);
        return -1;
    }

    return 0;
}

std::map<int, std::vector<int> *> &ServiceChain::get_vnf_instance()
{
    return this->vnf_instance;
}

//const std::map<int, std::vector<int> *> &ServiceChain::get_vnf_instance()
//{
//    return this->vnf_instance;
//}

std::vector<int> &ServiceChain::get_vnf_instance(int function_id)
{
    if (this->vnf_instance.find(function_id) != this->vnf_instance.end()) {
        return *this->vnf_instance[function_id];
    } else {
        warning_log("not existed function_id:%d", function_id);
        return vector<int>();
    }
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

int ServiceChain::get_related_pn_id(std::set<int> &pn_id_set)
{
    pn_id_set.clear();

    ServiceChainManager *service_chain_manager(NULL);
    if ((service_chain_manager = ServiceChainManager::get_instance()) == NULL) {
        warning_log("get instance failed");
        return -1;
        //return result;
    }

    VnfInstance *vi;
    for (int i = 0; i < this->length; ++i) {
        for (auto iter = vnf_instance[i]->begin(); iter != vnf_instance[i]->end(); ++iter) {
            int vi_id = *iter;
            if (service_chain_manager->get_vnf_instance(vi_id, vi) != 0) {
                warning_log("get vi failed");
                return -1;
                //return result;
            }
            if (vi->is_settled()) {
                pn_id_set.insert(vi->get_location());
            }
        }
    }

    return 0;
}

}
