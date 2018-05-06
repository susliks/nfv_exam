#include "req.h"

namespace nfv_exam {

void Req::clear()
{
    this->req_type = "none";
}

std::string Req::to_string()
{
    std::string result;
    result += std::string("req_type=") + std::string(this->req_type);
    result += std::string(" length=") + std::to_string(this->length);
    result += std::string(" chain_id=") + std::to_string(this->chain_id);
    result += std::string(" flow_id=") + std::to_string(this->flow_id);
    result += std::string(" flow_template_id=") + std::to_string(this->flow_template_id);
    result += std::string(" lifetime=") + std::to_string(this->lifetime);
    
    return result;
}

int Req::get_chain_id() const
{
    return this->chain_id;
}

int Req::get_flow_id() const
{
    return this->flow_id;
}

int Req::get_length() const
{
    return this->length;
}

int Req::get_flow_template_id() const
{
    return this->flow_template_id;
}

int Req::get_lifetime() const
{
    return this->lifetime;
}

const std::string &Req::get_req_type() const
{
    return this->req_type;
}

void Req::set_chain_id(int chain_id)
{
    this->chain_id = chain_id;
}

void Req::set_flow_id(int flow_id)
{
    this->flow_id = flow_id;
}

void Req::set_length(int length)
{
    this->length = length;
}

void Req::set_flow_template_id(int flow_template_id)
{
    this->flow_template_id = flow_template_id;
}

void Req::set_lifetime(int lifetime)
{
    this->lifetime = lifetime;
}

void Req::set_req_type(const std::string &req_type)
{
    this->req_type = req_type;
}

}
