#include "agregator.h"
#include <time.h>
#include <chrono>

using namespace std;

namespace YellowWatcher{

    string Replace(string source, string value, string replace){
        string result;
        string_view source_sv(source);
        auto pos = source_sv.find(value);
        while(pos != string_view::npos){
            result.append(source_sv.substr(0, pos))
            .append(replace);
            source_sv.remove_prefix(pos + value.size());
            pos = source_sv.find('.');
        }
        result.append(source_sv);

        return result;
    }

    void AddValue(Agregator::Value* p_value, int64_t value){
        ++p_value->count_;
        p_value->sum_ += value;
        if(value < p_value->min_) p_value->min_ = value;
        if(value > p_value->max_) p_value->max_ = value;
    }

    void AddValueToCall(Agregator::Value* value, const EventData* event){
        //memory
         AddValue(value, event->Memory());

        //memory_peak
        ++value;
        AddValue(value, event->MemoryPeak());

        //in_bytes
        ++value;
        AddValue(value, event->InBytes());

        //out_bytes
        ++value;
        AddValue(value, event->OutBytes());

        //cpu_time
        ++value;
        AddValue(value, event->CpuTime());
    }

    void Agregator::Add(const vector<EventData>& events){
        time_t cur_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
        struct tm * timeinfo;
        timeinfo = localtime (&cur_time);
        int cur_minute = timeinfo->tm_min;
        if(cur_minute == 0) cur_minute = 60;

        const EventData* event = &events[0];
        const EventData* end = event + events.size();
        for(;event != end;++event){

            int event_minute = event->Minute();
            if(event_minute == 0) event_minute = 60;

            if(event_minute - cur_minute <= 1){
                const string& name = event->Name();
                if(name == "CALL"){
                    AddCall(event);
                }
                else if(name == "TLOCK"){
                    AddTlock(event);
                }
                else if(name == "TTIMEOUT"){
                    AddTtimeout(event);
                }
                else if(name == "TDEADLOCK"){
                    AddTdeadlock(event);                
                }
            }
        }
    }

    void Agregator::Clear(){
        tlock_.clear();
        ttimeout_.clear();
        tdeadlock_.clear();
        call_rphost_.clear();
        call_rmngr_.clear();
        call_ragent_.clear();
    }

    void Agregator::AddTlock(const EventData* event){
        if(!event->WaitConnection().empty()){
            Value& value = tlock_[event->ProcessName()];
            ++value.count_;
            value.sum_ += event->Duration();
        }
    }

    void Agregator::AddTtimeout(const EventData* event){
        Value& value = ttimeout_[event->ProcessName()];
        ++value.count_;
    }

    void Agregator::AddTdeadlock(const EventData* event){
        Value& value = tdeadlock_[event->ProcessName()];
        ++value.count_;
    }    

    void Agregator::AddCall(const EventData* event){

        auto process =  event->Process();

        if(process == "rphost"){
            AddCallRphost(event);
        }
        else if(process == "rmngr"){
            AddCallRmngr(event);
        }
        else if(process == "ragent"){
            AddCallRagent(event);
        }
    }

    void Agregator::AddCallRphost(const EventData* event){
        AddValueToCall(call_rphost_["_Total"], event);
        if(event->ProcessName().empty()){
            AddValueToCall(call_rphost_["EmptyProcessName"], event);
        }
        else{
            AddValueToCall(call_rphost_[event->ProcessName()], event);
        }
    }

    void Agregator::AddCallRmngr(const EventData* event){
        AddValueToCall(call_rmngr_["_Total"], event);
        if(event->IName().empty()){
            AddValueToCall(call_rmngr_["EmptyIName"], event);
        }
        else{
            AddValueToCall(call_rmngr_[event->IName()], event);
        }
    }

    void Agregator::AddCallRagent(const EventData* event){
        AddValueToCall(call_ragent_["_Total"], event);
        if(event->IName().empty()){
            AddValueToCall(call_ragent_["EmptyIName"], event);
        }
        else{
            AddValueToCall(call_ragent_[event->IName()], event);
        }
    }

    void AddProcessCallString(string& value_str, string host_escape, string process, unordered_map<string, Agregator::Value[5]> call){
        for(auto it = call.begin(); it != call.end(); ++it){
            
            const Agregator::Value* value = it->second;

            value_str.append("YellowWatcher.")
            .append(host_escape)
            .append(".\\")
            .append(process)
            .append("(")
            .append(it->first)
            .append(")\\memory(bytes).")
            .append(to_string(value->sum_))
            .append(";");
            
            ++value;
            value_str.append("YellowWatcher.")
            .append(host_escape)
            .append(".\\")
            .append(process)
            .append("(")
            .append(it->first)
            .append(")\\memory_peak(bytes).")
            .append(to_string(value->sum_))
            .append(";");

            ++value;
            value_str.append("YellowWatcher.")
            .append(host_escape)
            .append(".\\")
            .append(process)
            .append("(")
            .append(it->first)
            .append(")\\in(bytes).")
            .append(to_string(value->sum_))
            .append(";");

            ++value;
            value_str.append("YellowWatcher.")
            .append(host_escape)
            .append(".\\")
            .append(process)
            .append("(")
            .append(it->first)
            .append(")\\out(bytes).")
            .append(to_string(value->sum_))
            .append(";");
            
            ++value;
            value_str.append("YellowWatcher.")
            .append(host_escape)
            .append(".\\")
            .append(process)
            .append("(")
            .append(it->first)
            .append(u8")\\cpu_time(µs).")
            .append(to_string(value->sum_))
            .append(";");
        }
    }

    string Agregator::ValueAsString(string host) const{
        
        string host_escape = Replace(host, ".", "\\.");
        
        string value_str;

        for(auto it = tdeadlock_.begin(); it != tdeadlock_.end(); ++it){
            value_str.append("YellowWatcher.")
            .append(host_escape)
            .append(".\\tdeadlock(")
            .append(it->first)
            .append(")\\count.")
            .append(to_string(it->second.count_))
            .append(";");
        }

        for(auto it = ttimeout_.begin(); it != ttimeout_.end(); ++it){
            value_str.append("YellowWatcher.")
            .append(host_escape)
            .append(".\\ttimeout(")
            .append(it->first)
            .append(")\\count.")
            .append(to_string(it->second.count_))
            .append(";");
        }

        for(auto it = tlock_.begin(); it != tlock_.end(); ++it){
            value_str.append("YellowWatcher.")
            .append(host_escape)
            .append(".\\tlock(")
            .append(it->first)
            .append(u8")\\wait(µs).")
            .append(to_string(it->second.sum_))
            .append(";");

            value_str.append("YellowWatcher.")
            .append(host_escape)
            .append(".\\tlock(")
            .append(it->first)
            .append(u8")\\wait_count.")
            .append(to_string(it->second.count_))
            .append(";");
        }

        AddProcessCallString(value_str, host_escape, "rphost", call_rphost_);
        AddProcessCallString(value_str, host_escape, "rmngr", call_rmngr_);
        AddProcessCallString(value_str, host_escape, "ragent", call_ragent_);

        return value_str;        
    }

    ostream& operator<< (std::ostream &out, const Agregator::Value* value){
        out
        << "count="<< value->count_
        << ";sum=" << value->sum_
        << ";avg=" << (double)value->sum_/value->count_
        << ";min=" << value->min_
        << ";max=" << value->max_;
        return out;
    }

    ostream& operator<< (ostream &out, const Agregator& agregator){
        for(auto it = agregator.tdeadlock_.begin(); it != agregator.tdeadlock_.end(); ++it){
            const Agregator::Value* value = &it->second;
            out << it->first << "\ttdeadlock\t\t" << value << '\n';
        }

        for(auto it = agregator.ttimeout_.begin(); it != agregator.ttimeout_.end(); ++it){
            const Agregator::Value* value = &it->second;
            out << it->first << "\tttimeout_\t\t" << value << '\n';
        }

        for(auto it = agregator.tlock_.begin(); it != agregator.tlock_.end(); ++it){
            const Agregator::Value* value = &it->second;
            out << it->first << "\ttlock_\t\t" << value << '\n';
        }

        // for(auto it = agregator.call_.begin(); it != agregator.call_.end(); ++it){
            
        //     const Agregator::Value* value = it->second;
        //     out << it->first << "\tmemory\t\t" << value << '\n';
            
        //     ++value;
        //     out << it->first << "\tmemory_peak\t" << value << '\n';

        //     ++value;
        //     out << it->first << "\tin_bytes\t" << value << '\n';

        //     ++value;
        //     out << it->first << "\tout_bytes\t" << value << '\n';
            
        //     ++value;
        //     out << it->first << "\tcpu_time\t" << value << '\n';
        // }
        return out;
    }
}