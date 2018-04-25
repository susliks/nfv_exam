#ifndef VNF_INSTANCE_H
#define VNF_INSTANCE_H

#include <vector>
#include <set>

namespace nfv_exam {

class VnfInstance {
public:



private:
    int id;
    int location;
    std::vector<int> last_vi_id;
    std::vector<int> next_vi_id;
    std::set<int> settled_flow_nodes;

    bool disable_scale_up_down;
    int vi_cpu_used;
    int vi_memory_used;
    int vi_cpu;
    int vi_memory;
};

}

#endif /* VNF_INSTANCE_H */
