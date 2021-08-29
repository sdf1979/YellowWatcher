#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <stack>

namespace YellowWatcher{

    class Parser;
    using PtrFcn = void(*)(const char**, Parser* const);

    enum class TypeChar{
        Digit,
        Colon,
        Dot,
        Minus,
        Comma,
        CommaInQ,
        Equal,
        Quote,
        Dquote,
        NewLine,
        NewLineInQ,
        Char
    };

    class EventData{
        int minute_;
        int second_;
        int msecond_;
        std::size_t duration_;
        std::string name_;
        int level_;
        std::string process_;
        std::string p_process_name_;
        std::string usr_;
        std::int64_t memory_;
        std::int64_t memory_peak_;
        std::int64_t in_bytes_;
        std::int64_t out_bytes_;
        std::int64_t cpu_time_;
        std::string wait_connection_;
        std::string descr_;
        std::string i_name_;

        std::string key_temp_;
        std::string value_temp_;

        void EndValue();

        friend void Minute(const char** ch, Parser* const parser);
        friend void Second(const char** ch, Parser* const parser);
        friend void Msecond(const char** ch, Parser* const parser);
        friend void Duration(const char** ch, Parser* const parser);
        friend void Event(const char** ch, Parser* const parser);
        friend void Level(const char** ch, Parser* const parser);
        friend void Key(const char** ch, Parser* const parser);
        friend void Value(const char** ch, Parser* const parser);
        friend void EndValue(const char** ch, Parser* const parser);
        friend void Mvalue(const char** ch, Parser* const parser);

        friend std::ostream& operator<< (std::ostream &out, const EventData& event_data);
    public:
        EventData();
        EventData(EventData&& event_data);
        EventData& operator=(EventData&& event_data);
        int Minute() const { return minute_; }
        int Second() const { return second_; }
        int Msecond() const { return msecond_; }
        std::size_t Duration() const { return duration_; }
        const std::string& Name() const { return name_; }
        int Level() const { return level_; }
        const std::string& Process() const { return process_; }
        const std::string& ProcessName() const {return p_process_name_; }
        const std::string& Usr() const { return usr_; }
        std::int64_t Memory() const { return memory_; }
        std::int64_t MemoryPeak() const { return memory_peak_; }
        std::int64_t InBytes() const { return in_bytes_; }
        std::int64_t OutBytes() const { return out_bytes_; }
        std::int64_t CpuTime() const { return cpu_time_; }
        const std::string& WaitConnection() const { return wait_connection_; }
        const std::string& Descr() const { return descr_; }
        const std::string& IName() const { return i_name_; }
    };

    class Parser{
        std::vector<EventData> events_;
        std::stack<char> stack_quotes;
        EventData event_data;
        PtrFcn ptr_fcn = nullptr;
        PtrFcn ptr_fcn_break = nullptr;

        friend void Start(const char** ch, Parser* const parser);
        friend void Minute(const char** ch, Parser* const parser);
        friend void Second(const char** ch, Parser* const parser);
        friend void Msecond(const char** ch, Parser* const parser);
        friend void Duration(const char** ch, Parser* const parser);
        friend void Event(const char** ch, Parser* const parser);
        friend void Level(const char** ch, Parser* const parser);
        friend void Key(const char** ch, Parser* const parser);
        friend void Value(const char** ch, Parser* const parser);
        friend void EndValue(const char** ch, Parser* const parser);
        friend void EndEvent(const char** ch, Parser* const parser);
        friend void Mvalue(const char** ch, Parser* const parser);

        friend TypeChar GetTypeChar(char ch, Parser* const parser);
        //TODO отладка
        //std::string data;
    public:
        void Parse(const char* ch, int size);
        void AddEvent(EventData&& event_data);
        std::vector<EventData>&& MoveEvents();
    };
}