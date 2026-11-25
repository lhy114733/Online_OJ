#pragma once
#include <string>
#include <fstream>
#include <atomic>

#include <sys/stat.h>
#include <sys/time.h>

#include <boost/algorithm/string.hpp>

namespace ns_util 
{
    class TimeUtil
    {
    public:
        static std::string GetTimeStamp()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);
            return std::to_string(_time.tv_sec);
        }
        //获得毫秒时间戳
        static std::string GetTimeMs()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);
            return std::to_string(_time.tv_sec * 1000 + _time.tv_usec / 1000);
        }
    };

    //构造完整文件名：cpp,exe,compile_err,stdin,stderr,stdout
    class PathUtil
    {
        private:
        static std::string AddSuffix(const std::string& file_name,const std::string& suffix)
        {
            std::string name = file_name + '.' + suffix;
            return name;
        }
        public:
        static std::string Src(const std::string& file_name)
        {
            return AddSuffix(file_name,"cpp");
        }
        static std::string Exe(const std::string& file_name)
        {
            return AddSuffix(file_name,"exe");
        }
        static std::string CompilerErr(const std::string& file_name)
        {
            return AddSuffix(file_name,"compile_err");
        }

        static std::string Stdin(const std::string& file_name)
        {
            return AddSuffix(file_name,"stdin");
        }
        static std::string Stdout(const std::string& file_name)
        {
            return AddSuffix(file_name,"stdout");
        }
        static std::string Stderr(const std::string& file_name)
        {
            return AddSuffix(file_name,"stderr");
        }
    };

    class FileUtil 
    {
        public:
        static bool IsFileExists(const std::string &path_name)
        {
            struct stat st;
            if (stat(path_name.c_str(), &st) == 0)
            {
                //获取属性成功，文件已经存在
                return true;
            }

            return false;
        }
        //这部分和上面那部分和事件有关的是抄的
        static std::string UniqueFileName()
        {
            static std::atomic_uint id(0);
            id++;
            // 毫秒级时间戳+原子性递增唯一值: 来保证唯一性
            std::string ms = TimeUtil::GetTimeMs();
            std::string uniq_id = std::to_string(id);
            return ms + "_" + uniq_id;
        }
        static bool WriteFile(const std::string& target, const std::string& context)
        {
            std::ofstream out(target);
            if(!out.is_open())
            {
                return false;
            }
            out.write(context.c_str(),context.size());
            out.close();
            return true;
        }
        static bool ReadFile(const std::string& target, std::string* context, bool keepflag = false)
        {
            context->clear();

            std::ifstream in(target);
            if(!in.is_open())
            {
                return false;
            }
            std::string line;
            while(std::getline(in,line))
            {
                (*context) += line;
                (*context) += (keepflag? "\n" : ""); 
            }
            return true;
        }
    };

    class StringUtil 
    {
        public:
        static void SplitString(const std::string &str, std::vector<std::string> *target, const std::string &sep)
        {
            boost::split((*target), str, boost::is_any_of(sep), boost::algorithm::token_compress_on);
        }
    };
}


