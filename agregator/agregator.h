#pragma once

#include "parser.h"
#include <vector>
#include <unordered_map>
#include <string>

namespace YellowWatcher{

    class Agregator{
        struct Value{
            int count_ = 0;
            std::int64_t sum_ = 0;
            std::int64_t min_ = INT64_MAX;
            std::int64_t max_ = INT64_MIN;
        };
        void AddTlock(const EventData* event);
        void AddTtimeout(const EventData* event);
        void AddTdeadlock(const EventData* event);
        void AddCall(const EventData* event);
        void AddCallRphost(const EventData* event);
        void AddCallRmngr(const EventData* event);
        void AddCallRagent(const EventData* event);
        
        std::unordered_map<std::string, Value> tlock_;
        std::unordered_map<std::string, Value> ttimeout_;
        std::unordered_map<std::string, Value> tdeadlock_;

        std::unordered_map<std::string, Value[5]> call_rphost_;
        std::unordered_map<std::string, Value[5]> call_rmngr_;
        std::unordered_map<std::string, Value[5]> call_ragent_;

        friend void AddValue(Agregator::Value*, int64_t);
        friend void AddValueToCall(Agregator::Value* value, const EventData* event);
        friend void AddProcessCallString(std::string& value_str, std::string host_escape, std::string process, std::unordered_map<std::string, Agregator::Value[5]>);

        friend std::ostream& operator<< (std::ostream &out, const Agregator::Value* value);
        friend std::ostream& operator<< (std::ostream &out, const Agregator& agregator);
    public:
        void Add(const std::vector<EventData>& events);
        void Clear();
        std::string ValueAsString(std::string host) const;
    };

}