#include <iostream>
#include <map>
#include <set>
#include <string>
#include <deque>
#include <memory>
#include "json.h"

namespace json{

class Builder {
    class BaseContext;
    class DictContext;
    class ArrayContext;
    class ValueContext;

public:
    Builder() = default;
    DictContext StartDict();
    ArrayContext StartArray();
    ValueContext Value(Node node);

    BaseContext Key(std::string) = delete;
    BaseContext EndDict() = delete;
    BaseContext EndArray() = delete;
    Node Build() = delete;

    bool IsRoot(){
        bool tmp = first_run_;
        first_run_ = false;
        return tmp;
    }

private:
    Node root_;
    std::vector<Node *> nodes_stack_;
    std::string current_key_ = "";

    bool is_key_ = false;
    bool first_run_ = true;
    bool value_can_build_ = false;

    class BaseContext {
    public:

        BaseContext(Builder& builder)
            : builder_(builder){ }

        BaseContext Key(std::string string);
        BaseContext EndDict();
        BaseContext EndArray();
        Node Build();

        DictContext StartDict() {
            return builder_.StartDict();
        }
        ArrayContext StartArray() {
             return builder_.StartArray();
        }
        BaseContext Value(Node node) {
            return builder_.Value(node);
        }
        bool IsRoot(){
            return builder_.IsRoot();
        }        
        Builder& GetBuilder(){
            return builder_;
        }
    private:
        Builder& builder_;

    };

     class BuildContext : public BaseContext {
     public:
         BuildContext(BaseContext base)
             : BaseContext(base){}
     };

    class DictContext : public BaseContext {

    public:
        class KeyContext;
        DictContext(BaseContext base)
            : BaseContext(base) {}
        DictContext StartDict() = delete;
        ArrayContext StartArray() = delete;
        BaseContext EndArray() = delete;
        ValueContext Value(Node) = delete;
        Node Build() = delete;
        KeyContext Key(std::string str){
            return  BaseContext{*this}.Key(str);

        }

        class KeyContext : public BaseContext {
            class ValueKeyContext;
        public:
            KeyContext(BaseContext base)
                : BaseContext(base){
            }
            KeyContext Key(std::string) = delete;
            BaseContext EndArray() = delete;
            BaseContext EndDict() = delete;
            Node Build() = delete;
            ValueKeyContext  Value(Node node) {
                return  this->GetBuilder().Value(node);
            };
        private:
            class ValueKeyContext : public BaseContext {
            public:
                ValueKeyContext (BaseContext base)
                    : BaseContext(base) {
                }
                ArrayContext StartArray() = delete;
                BaseContext EndArray() = delete;
                DictContext StartDict() = delete;
                ValueContext Value() = delete;
                Node Build() = delete;
            };
        };

   };




    class ArrayContext : public BaseContext {
        class StartArrayValueContext;
        class StartArrayNextValueContext;
    public:
        ArrayContext(BaseContext base) : BaseContext(base) {}

        BaseContext EndDict() = delete;
        DictContext::KeyContext Key() = delete;
        Node Build() = delete;
        StartArrayValueContext  Value(Node node){
            return BaseContext{*this}.Value(node);
        };

    private:
        class StartArrayValueContext : public BaseContext {
        public:
            StartArrayValueContext (BaseContext base)
                : BaseContext(base) {
            }
            StartArrayNextValueContext Value(Node node){
                return  this->GetBuilder().Value(node);
            };
            DictContext::KeyContext Key() = delete;
            Node Build() = delete;
            BaseContext EndDict() = delete;
        };

        class StartArrayNextValueContext : public BaseContext {
        public:
            StartArrayNextValueContext (BaseContext base)
                : BaseContext(base) {
            }
            DictContext::KeyContext Key() = delete;
            BaseContext EndDict() = delete;
            Node Build() = delete;
        };

    };

    class ValueContext : public BaseContext {
    public:
        ValueContext(BaseContext base)
            : BaseContext(base) {
        }
        ArrayContext StartArray() = delete;
        BaseContext EndArray() = delete;
        DictContext StartDict() = delete;
        ValueContext Value() = delete;
    };




};

} //end namespace

