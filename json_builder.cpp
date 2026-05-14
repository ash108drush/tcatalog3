#include "json_builder.h"

namespace json {

Builder::ValueContext Builder::Value(Node node){
    if(IsRoot()){
        root_ = std::move(node);
        value_can_build_ = true;
        return BaseContext{ *this };
    }
    if(nodes_stack_.size() == 0){
        throw std::logic_error("Double Value Not in Dict or Array");
    }

    Node* node_ptr = nodes_stack_.back();
    if(node_ptr->IsArray()){
        std::get<Array>(node_ptr->GetValue()).emplace_back(std::move(node));
        return BaseContext{ *this };
    }
    if(node_ptr->IsDict()){
        if(is_key_){
            std::get<Dict>(node_ptr->GetValue()).insert({current_key_,std::move(node)});
            is_key_ = false;
        }else{
            throw std::logic_error("Value not after Key in Dict");
        }
         return BaseContext{ *this };
    }
    throw std::logic_error("Value not in Dict not in Array");
    return BaseContext{ *this };
}

Builder::ArrayContext Builder::StartArray() {
    if(IsRoot()){
        root_ =  Node(Array());
        nodes_stack_.push_back(std::move(&root_));
        return BaseContext {*this};
    }

    if(nodes_stack_.size()  == 0){
        throw std::logic_error("Start Array node stack size is Zero");
    }

    Node* node_ptr = nodes_stack_.back();
    if(node_ptr->IsDict()){
        if(is_key_){
            Node node_new =  Node(Array());
            auto pair = std::get<Dict>(node_ptr->GetValue()).emplace(current_key_,std::move(node_new));
            Node *nd = &pair.first->second;
            nodes_stack_.push_back(std::move(nd));
            is_key_ = false;
        }else{
            throw std::logic_error("StartArray not after Key in Dict");
        }
        return BaseContext {*this};
    }

    if(node_ptr->IsArray()){
        Node node_new = Node(Array());
        Node& nd = std::get<Array>(node_ptr->GetValue()).emplace_back(std::move(node_new));
        nodes_stack_.push_back(std::move(&nd));
    }
    return BaseContext {*this};
 }

Builder::DictContext Builder::StartDict() {
    if(IsRoot()){
        root_ = Node(Dict());
        nodes_stack_.push_back(std::move(&root_));
        return BaseContext{ *this };
    }
    if(nodes_stack_.size()  == 0){
        throw std::logic_error("Start Dict node stack size is Zero");
    }
    Node* node_ptr = nodes_stack_.back();
    if(node_ptr->IsArray()){
        Node node_new = Node(Dict());
        Node& nd = std::get<Array>(node_ptr->GetValue()).emplace_back(std::move(node_new));
        nodes_stack_.push_back(std::move(&nd));
        return BaseContext{ *this };
    }

    if(node_ptr->IsDict()){
        if(is_key_){
            Node node_new = Node(Dict());
            auto pair = std::get<Dict>(node_ptr->GetValue()).emplace(current_key_,std::move(node_new));
            Node *nd = &pair.first->second;
            nodes_stack_.push_back(std::move(nd));
            is_key_ = false;
        }else{
            throw std::logic_error("StartDict not after Key in Dict");
        }
        return BaseContext{ *this };
    }

    return BaseContext{ *this };
}

Builder::BaseContext Builder::BaseContext::EndArray() {
    if(builder_.nodes_stack_.size() ==0){
        throw std::logic_error("End Array Zero Stack Size");
    }
    Node* node_ptr = builder_.nodes_stack_.back();
    if(node_ptr->IsArray()){
        builder_.nodes_stack_.pop_back();
    }else{
        throw std::logic_error("Array expected");
    }
    return *this;
}

Builder::BaseContext Builder::BaseContext::EndDict() {
    if(builder_.nodes_stack_.size() ==0){
        throw std::logic_error("End Dict Zero Stack Size");
    }
    if(builder_.is_key_ ){
        throw std::logic_error("Key not closed");
    }
    Node* node_ptr = builder_.nodes_stack_.back();
    if(node_ptr->IsDict()){
        builder_.nodes_stack_.pop_back();
    }else{
        throw std::logic_error("Dict expected");
    }
    return *this;
}


Builder::BaseContext Builder::BaseContext::Key(std::string string){
    if(builder_.nodes_stack_.size() ==0){
        throw std::logic_error("Key not in stack size");
    }
    if(builder_.is_key_){
        throw std::logic_error("Double key");
    }
    Node* node_ptr = builder_.nodes_stack_.back();
    if(!node_ptr->IsDict()){
        throw std::logic_error("Key not in dict");
    }
    builder_.current_key_ = string;
    builder_.is_key_ = true;
    return *this;
}

Node Builder::BaseContext::Build(){
    if(builder_.first_run_){
        throw std::logic_error("Build first run");
    }
    if(builder_.nodes_stack_.size() > 0){
        throw std::logic_error("Not All Array or Dict End");
    }
    return builder_.root_;
}

} // end namespace
