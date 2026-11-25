#pragma once
#include <iostream>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/writer.h>


#include "compiler.hpp"
#include "runner.hpp"

namespace ns_compile_run
{
    using namespace ns_runner;
    using namespace ns_compiler;
    using namespace ns_util;
    class Compile_run 
    {
        private:
        static void RemoveTempFile(const std::string &file_name)
        {
            //清理文件的个数是不确定的，但是有哪些我们是知道的
            std::string _src = PathUtil::Src(file_name);
            if(FileUtil::IsFileExists(_src)) unlink(_src.c_str());

            std::string _compiler_error = PathUtil::CompilerErr(file_name);
            if(FileUtil::IsFileExists(_compiler_error)) unlink(_compiler_error.c_str());

            std::string _execute = PathUtil::Exe(file_name);
            if(FileUtil::IsFileExists(_execute)) unlink(_execute.c_str());

            std::string _stdin = PathUtil::Stdin(file_name);
            if(FileUtil::IsFileExists(_stdin)) unlink(_stdin.c_str());

            std::string _stdout = PathUtil::Stdout(file_name);
            if(FileUtil::IsFileExists(_stdout)) unlink(_stdout.c_str());

            std::string _stderr = PathUtil::Stderr(file_name);
            if(FileUtil::IsFileExists(_stderr)) unlink(_stderr.c_str());
        }
        static std::string CodeToDesc(int code, const std::string &file_name)
        {
            std::string desc;
            switch (code)
            {
            case 0:
                desc = "编译运行成功";
                break;
            case -1:
                desc = "提交的代码是空";
                break;
            case -2:
                desc = "未知错误"; //compiler和runner的系统内部错误被统一归为-2
                break;
            case -3:
                // desc = "代码编译的时候发生了错误";
                FileUtil::ReadFile(PathUtil::CompilerErr(file_name), &desc, true);
                break;
            case SIGABRT: // 6
                desc = "内存超过范围";
                break;
            case SIGXCPU: // 24
                desc = "CPU使用超时";
                break;
            case SIGFPE: // 8
                desc = "浮点数溢出";
                break;
            default:
                desc = "未知: " + std::to_string(code);
                break;
            }

            return desc;
        }
        public:
        Compile_run()
        {}
        ~Compile_run()
        {}
        public:
        //in_json结构
        // code :    ,   input :     limit_cpu :     ,limit_mem:   
        // out_json
        // status:    ,  reason:     stdout:         ,stderr:
        static void start(const std::string& in_json, std::string *out_json)
        {
            //反序列化，解释in_json，提取in_json信息
            Json::Value in_value;
            Json::Reader reader;
            reader.parse(in_json,in_value);

            Json::Value out_value; //用来构造输出型参数out_json

            std::string code = in_value["code"].asString();
            std::string input = in_value["input"].asString();
            int limit_cpu = in_value["limit_cpu"].asInt();
            int limit_mem = in_value["limit_mem"].asInt();

            //设置一些goto之间的参数，避免语法错误
            int ret = 0;
            std::string file_name;
            int status_code = 0;
            if(code.size() == 0)
            {
                status_code = -1;
                out_value["reason"] = "代码为空";
                goto END;
            }

            //先形成源代码文件,把收到的code写入到该文件中
            file_name = FileUtil::UniqueFileName();
            if(!FileUtil::WriteFile(PathUtil::Src(file_name), code)) //out.is_open()时自动创建文件
            {
                status_code = -2; //同下
                out_value["reason"] = "未知错误";
                goto END;
            }
            //编译代码
            if(!Compiler::Compile(file_name)) 
            {
                std::cout << "编译失败" << std::endl;
                status_code = -3;
                out_value["reason"] = "编译错误";
                goto END;
            }
            //运行代码
            ret = Runner::Run(file_name ,limit_cpu ,limit_mem);
            if(ret > 0) //运行奔溃
            {
                status_code = ret;
                goto END;
            }
            else if(ret < 0) //会不会有这种情况？？？
            {
                status_code = -2; //和上面编译的系统内部错误被归为一类
                goto END;
            }
            else
            {
                status_code = 0;
                goto END;
            }
            END:
            out_value["status"] = status_code;
            out_value["reason"] = CodeToDesc(status_code, file_name);
            if(status_code == 0)
            {
                std::string stdout_;
                FileUtil::ReadFile(PathUtil::Stdout(file_name),&stdout_,true);
                out_value["stdout"] = stdout_;
                std::string stderr_;
                FileUtil::ReadFile(PathUtil::Stderr(file_name),&stderr_,true);
                out_value["stderr"] = stderr_;
            }

            //序列化，形成json
            Json::StyledWriter writer;
            *out_json = writer.write(out_value);
            RemoveTempFile(file_name);
        }
    };
}

//想明白系统内部错误fork、exec等失败
//和根据业务或实际设计的错误，如代码为空、编译错误(通过bool判断、实际也应该通过int判断好一点，用bool有bug)、运行时奔溃(通过ret判断)、运行成功(ret==0)
//前者是系统意外，后者是故意设计


//想明白返回值return，和子进程exit被父进程status获得

