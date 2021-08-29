#include "parser.h"

using namespace std;

namespace YellowWatcher{
    
    EventData::EventData():
        minute_(0),
        second_(0),
        msecond_(0),
        duration_(0),
        level_(0),
        memory_(0),
        memory_peak_(0),
        in_bytes_(0),
        out_bytes_(0),
        cpu_time_(0){}

    EventData::EventData(EventData&& event_data):
        minute_(event_data.minute_),
        second_(event_data.second_),
        msecond_(event_data.msecond_),
        duration_(event_data.duration_),
        name_(move(event_data.name_)),
        level_(event_data.level_),
        process_(move(event_data.process_)),
        p_process_name_(move(event_data.p_process_name_)),
        usr_(move(event_data.usr_)),
        memory_(event_data.memory_),
        memory_peak_(event_data.memory_peak_),
        in_bytes_(event_data.in_bytes_),
        out_bytes_(event_data.out_bytes_),
        cpu_time_(event_data.cpu_time_),
        key_temp_(move(event_data.key_temp_)),
        value_temp_(move(event_data.value_temp_)),
        wait_connection_(move(event_data.wait_connection_)),
        descr_(move(event_data.descr_)),
        i_name_(move(event_data.i_name_)){
        event_data.minute_ = 0;
        event_data.second_ = 0;
        event_data.msecond_ = 0;
        event_data.duration_ = 0;
        event_data.level_ = 0;
        event_data.memory_ = 0;
        event_data.memory_peak_ = 0;
        event_data.in_bytes_ = 0;
        event_data.out_bytes_ = 0;
        event_data.cpu_time_ = 0;
    }

    EventData& EventData::operator=(EventData&& event_data){
        if (&event_data == this) return *this;
        
        minute_ = event_data.minute_;
        second_ = event_data.second_;
        msecond_ = event_data.msecond_;
        duration_ = event_data.duration_;
        name_ = move(event_data.name_);
        level_ = event_data.level_;
        process_ =move(event_data.process_);
        p_process_name_ = move(event_data.p_process_name_);
        usr_ = move(event_data.usr_);
        memory_ = event_data.memory_;
        memory_peak_ = event_data.memory_peak_;
        in_bytes_ = event_data.in_bytes_;
        out_bytes_ = event_data.out_bytes_;
        cpu_time_ = event_data.cpu_time_;
        wait_connection_ = move(event_data.wait_connection_);
        descr_ = move(event_data.descr_);
        i_name_ = move(event_data.i_name_);

        key_temp_ = move(event_data.key_temp_);
        value_temp_ = move(event_data.value_temp_);

        event_data.minute_ = 0;
        event_data.second_ = 0;
        event_data.msecond_ = 0;
        event_data.duration_ = 0;
        event_data.level_ = 0;
        event_data.memory_ = 0;
        event_data.memory_peak_ = 0;
        event_data.in_bytes_ = 0;
        event_data.out_bytes_ = 0;
        event_data.cpu_time_ = 0;

        return *this;        
    }

    bool IsGuid(const char* begin, const char* end){
        int size_guid = 0;
        for(;begin != end;++begin){
            char ch = *begin;
            if((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch == '-' && (size_guid == 8 || size_guid == 13 || size_guid == 18 || size_guid == 23))){
                ++size_guid;
            }
            else{
                return false;
            }
        }
        return true;
    }

    string ReplaceGuid(string value){
        size_t size = value.size();
        if(size < 37){
            return move(value);
        }
        else{
            const char* begin = value.c_str() + size - 36;
            if(IsGuid(begin, begin + 36)){
                string new_value;
                new_value.append(value.substr(0, size - 36));
                new_value.append("_guid");
                return new_value;
            }
            else{
                return(move(value));
            }
        }        
    }

    void EventData::EndValue(){
        if(key_temp_ == "process") process_ = move(value_temp_);
        else if(key_temp_ == "p:processName") p_process_name_ = ReplaceGuid(move(value_temp_));
        else if(key_temp_ == "Usr") usr_ = move(value_temp_);
        else if(key_temp_ == "Memory"){
            memory_ = stoll(value_temp_);
            value_temp_ = "";
        }
        else if(key_temp_ == "MemoryPeak"){
            memory_peak_ = stoll(value_temp_);
            value_temp_ = "";
        }
        else if(key_temp_ == "InBytes"){
            in_bytes_ = stoll(value_temp_);
            value_temp_ = "";
        }
        else if(key_temp_ == "OutBytes"){
            out_bytes_ = stoll(value_temp_);
            value_temp_ = "";
        }
        else if(key_temp_ == "CpuTime"){
            cpu_time_ = stoll(value_temp_);
            value_temp_ = "";
        }
        else if(key_temp_ == "WaitConnections") wait_connection_ = move(value_temp_);
        else if(key_temp_ == "Descr") descr_ = move(value_temp_);
        else if(key_temp_ == "IName") i_name_ = move(value_temp_);
        else value_temp_ = "";
        key_temp_ = "";
    }
    
    ostream& operator<< (std::ostream &out, const EventData& event_data){
        out << event_data.minute_ << ':' << event_data.second_ << '.' << event_data.msecond_ << '-' << event_data.duration_ << ',' << event_data.name_;
        return out;
    }

    TypeChar GetTypeChar(char ch, Parser* const parser){
        if(ch >= '0' && ch <= '9'){
            return TypeChar::Digit;
        }
        else if(ch == ':'){
            return TypeChar::Colon;
        }
        else if(ch == '.'){
            return TypeChar::Dot;
        }
        else if(ch == '-'){
            return TypeChar::Minus;
        }
        else if(ch == ','){
            if(parser->stack_quotes.empty()){
                return TypeChar::Comma;
            }
            else{
                return TypeChar::CommaInQ;
            }
        }
        else if(ch == '='){
            return TypeChar::Equal;
        }
        else if(ch == '\''){
            return TypeChar::Quote;
        }
        else if(ch == '"'){
            return TypeChar::Dquote;
        }
        else if(ch == '\n'){
            if(parser->stack_quotes.empty()){
                return TypeChar::NewLine;
            }
            else{
                return TypeChar::NewLineInQ;
            }
        }
        else{
            return TypeChar::Char;
        }
    };

    enum class State{
        Start,
        Minute,
        Second,
        Msecond,
        Duration,
        Event,
        Level,
        Key,
        Value,
        EndValue,
        EndEvent,
        Mvalue,
        Error
    };

    //EventData event_data;
    // PtrFcn ptr_fcn = nullptr;
    // PtrFcn ptr_fcn_break = nullptr;

    void Start(const char** ch, Parser* const parser){
        parser->event_data = {};
        if(**ch == '\n'){
            ++*ch;            
        }        
    }

    void Minute(const char** ch, Parser* const parser){
        parser->event_data.minute_ = parser->event_data.minute_ * 10 + (**ch - '0');
        ++*ch;
    }

    void Second(const char** ch, Parser* const parser){
        char ch_ = **ch;
        if(ch_ != ':'){
            parser->event_data.second_ = parser->event_data.second_ * 10 + (**ch - '0');
        }
        ++*ch;
    }

    void Msecond(const char** ch, Parser* const parser){
        char ch_ = **ch;
        if(ch_ != '.'){
            parser->event_data.msecond_ = parser->event_data.msecond_ * 10 + (ch_ - '0');
        }
        ++*ch;
    }

    void Duration(const char** ch, Parser* const parser){
        char ch_ = **ch;
        if(ch_ != '-'){
            parser->event_data.duration_ = parser->event_data.duration_ * 10 + (ch_ - '0');
        }
        ++*ch;
    }

    void Event(const char** ch, Parser* const parser){
        char ch_ = **ch;
        if(ch_ != ','){
            parser->event_data.name_.push_back(ch_);
        }
        ++*ch;
    }

    void Level(const char** ch, Parser* const parser){
        char ch_ = **ch;
        if(ch_ != ','){
            parser->event_data.level_ = parser->event_data.level_ * 10 + (ch_ - '0');
        }
        ++*ch;
    }

    void Key(const char** ch, Parser* const parser){
        char ch_ = **ch;
        if(ch_ != ','){
            parser->event_data.key_temp_.push_back(ch_);
        }
        ++*ch;
    }

    void Value(const char** ch, Parser* const parser){
        char ch_ = **ch;
        if(ch_ != '=' && ch_ != '\r'){
            parser->event_data.value_temp_.push_back(ch_);
        }
        ++*ch;
    }

    void EndValue(const char** ch, Parser* const parser){
        parser->event_data.EndValue();
    }

    void EndEvent(const char** ch, Parser* const parser){
        parser->AddEvent(move(parser->event_data));
    }

    void Mvalue(const char** ch, Parser* const parser){
        char ch_ = **ch;
        if(ch_ == '\'' || ch_ == '"'){
            if(parser->stack_quotes.empty()){
                parser->stack_quotes.push(ch_);
            }
            else{
                if(parser->stack_quotes.top() == ch_){
                    parser->stack_quotes.pop();
                }
            }
        }
        parser->event_data.value_temp_.push_back(ch_);
        ++*ch;        
    }

    void Error(const char** ch, Parser* const parser){
        throw "Parser error!";
    }

    State GetState(PtrFcn ptr_fcn_){
        if(ptr_fcn_ == Start) return State::Start;
        else if(ptr_fcn_ == Minute) return State::Minute;
        else if(ptr_fcn_ == Second) return State::Second;
        else if(ptr_fcn_ == Msecond) return State::Msecond;
        else if(ptr_fcn_ == Duration) return State::Duration;
        else if(ptr_fcn_ == Event) return State::Event;
        else if(ptr_fcn_ == Level) return State::Level;
        else if(ptr_fcn_ == Key) return State::Key;
        else if(ptr_fcn_ == Value) return State::Value;
        else if(ptr_fcn_ == EndValue) return State::EndValue;
        else if(ptr_fcn_ == EndEvent) return State::EndEvent;
        else if(ptr_fcn_ == Mvalue) return State::Mvalue; 
        else return State::Error;
    }

PtrFcn TransitionTable[12][12] = {
//              Digit     Colon   Dot      Minus     Comma     CommaInQ Equal   Quote   Dquote  NewLine   NewLineInQ Char       
/*Start*/     { Minute,   Error,  Error,   Error,    Error,    Error,   Error,  Error,  Error,  Error,    Error,     Error  },
/*Minute*/    { Minute,   Second, Error,   Error,    Error,    Error,   Error,  Error,  Error,  Error,    Error,     Error  },
/*Second*/    { Second,   Error,  Msecond, Error,    Error,    Error,   Error,  Error,  Error,  Error,    Error,     Error  },
/*Msecond*/   { Msecond,  Error,  Error,   Duration, Error,    Error,   Error,  Error,  Error,  Error,    Error,     Error  },
/*Duration*/  { Duration, Error,  Error,   Error,    Event,    Error,   Error,  Error,  Error,  Error,    Error,     Error  },
/*Event*/     { Error,    Error,  Error,   Error,    Level,    Error,   Error,  Error,  Error,  Error,    Error,     Event  },
/*Level*/     { Level,    Error,  Error,   Error,    Key,      Error,   Error,  Error,  Error,  Error,    Error,     Error  },
/*Key*/       { Key,      Key,    Key,     Key,      Key,      Error,   Value,  Error,  Error,  Error,    Error,     Key    },
/*Value*/     { Value,    Value,  Value,   Value,    EndValue, Error,   Value,  Mvalue, Mvalue, EndValue, Error,     Value  },
/*EndValue*/  { Error,    Error,  Error,   Error,    Key,      Error,   Error,  Error,  Error,  EndEvent, Error,     Error  },
/*EndEvent*/  { Error,    Error,  Error,   Error,    Error,    Error,   Error,  Error,  Error,  Start,    Error,     Error  },
/*Mvalue*/    { Mvalue,   Mvalue, Mvalue,  Mvalue,   EndValue, Mvalue,  Mvalue, Mvalue, Mvalue, EndValue, Mvalue,    Mvalue }
};

    void Parser::Parse(const char* ch, int size){
        if(!ptr_fcn){
            ptr_fcn = Start;
        }
        const char* end = ch + size;

        TypeChar type_char;
        
        //int read = 0;
        const char* cur_ch = ch;

        for(;;){
            if(!ptr_fcn_break){
                ptr_fcn(&ch, this);
            }
            else{
                ptr_fcn = ptr_fcn_break;
                ptr_fcn_break = nullptr;
            }

            if(ch == end){
                ptr_fcn_break = ptr_fcn;
                break;
            }

            type_char = GetTypeChar(*ch, this);
            PtrFcn ptr_fcn_prev = ptr_fcn;
            ptr_fcn = TransitionTable[(int)GetState(ptr_fcn)][(int)type_char];
            if(ptr_fcn == Error){
                // string str = string(end - size, size);
                // cout << "All read:\n";
                // cout << data;
                // cout << "\nBuffer:\n";
                // cout << str;
                throw "Parser error!";                
            } 
        }
    };

    void Parser::AddEvent(EventData&& event_data){
        events_.push_back(move(event_data));
    }

    std::vector<EventData>&& Parser::MoveEvents(){
        return move(events_);
    }
}