#pragma once

// 定义日志文件
#include <iostream>
#include <unistd.h>
#include <mutex>
#include <chrono>
#include <ctime>
#include <sstream>
#include <fstream>


namespace Log
{
    enum class LogLevel
    {
        ERROR,
        INFO,
        FATAL,
    };

    //
    std::string getCurTime()
    {
        auto now = std::chrono::system_clock::now();

        // 转换为 time_t（C风格时间）
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);

        // 转换为本地时间字符串
        std::stringstream ssm;
        ssm << std::ctime(&now_time);
        return ssm.str();
    }

    class LogClass
    {
        LogClass():_ofs(_filename)
        {
            
        }


    public:
        
        static LogClass& getInstance() 
        {
            if(_self == nullptr) _self = new LogClass();
            return *_self;
        } 

        static void setFileName(const std::string& filename)
        {
            _filename = filename;
        }
        template <class T>
        LogClass& operator<<(T&& args)
        {
            _ofs << args;
            return *this;
        }

    private:
        std::mutex _mtx;
        std::ofstream _ofs;
        static std::string _filename;
        static inline LogClass* _self =  nullptr;
    };

#define ENABLE_FILE_LOG() LogClass::setFileName()
}