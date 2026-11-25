#pragma once
#include <iostream>
#include <string>


#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/resource.h>

#include "../comm/util.hpp"


namespace ns_runner 
{
    using namespace ns_util;
    class Runner 
    {
        public:
        Runner()
        {}
        ~Runner()
        {}
        private:
        static void SetProcLimit(int cpu_limit,int mem_limit)
        {
            struct rlimit _cpu_limit;
            _cpu_limit.rlim_cur = cpu_limit;
            _cpu_limit.rlim_max = RLIM_INFINITY;
            setrlimit(RLIMIT_CPU,&_cpu_limit);
            struct rlimit _mem_limit;
            _mem_limit.rlim_cur = mem_limit;
            _mem_limit.rlim_max = RLIM_INFINITY;
            setrlimit(RLIMIT_AS,&_mem_limit);
        }
        public:
        static int Run(std::string& file_name, int cpu_limit, int mem_limit)
        {
            //运行有三个情况：1. 运行完了，结果正确 2. 运行完了，结果不正确 3. 运行到一般奔溃
            //需要打开三个文件重定向标准012：stdin，stdout，stderr(重定向要运行的程序file_name的2标准错误)
            //   编译时需要cpp，exe，compile_err(也是重定向g++进程的2标准错误)，运行时需要上面三个文件
            std::string _execute = PathUtil::Exe(file_name);
            std::string _stdin   = PathUtil::Stdin(file_name);
            std::string _stdout  = PathUtil::Stdout(file_name);
            std::string _stderr  = PathUtil::Stderr(file_name);

            umask(0);
            int stdin_ = open(PathUtil::Stdin(file_name).c_str(), O_CREAT|O_RDONLY, 0644);
            int stdout_ = open(PathUtil::Stdout(file_name).c_str(), O_CREAT|O_WRONLY, 0644);
            int stderr_ = open(PathUtil::Stderr(file_name).c_str(), O_CREAT|O_WRONLY, 0644);

            if(stdin_ < 0 || stdout_ < 0 || stderr_ < 0){
                std::cout << "运行时打开标准文件失败" << std::endl;
                return -1; //代表打开文件失败
            }      

            pid_t pid = fork();
            if(pid < 0)
            {
                close(stdin_);
                close(stdout_);
                close(stderr_);

                std::cout << "运行时fork失败，系统内部问题" <<std::endl; 
                return -2;
            }
            else if(pid == 0)
            {
                SetProcLimit(cpu_limit, mem_limit);
                dup2(stdin_,0);
                dup2(stdout_,1);
                dup2(stderr_,2);
                //子进程
                execl(_execute.c_str(),_execute.c_str(), nullptr); //为啥要加个nullptr
                //走到这里说明execl失败，系统内部问题，要不要用负数
                // exit(1);
                return -3; //就应该是负数
                //还有点问题？？？？？？？？？？？？？？？？？？？明天解决
            }
            else
            {
                close(stdin_);
                close(stdout_);
                close(stderr_);
                int status = 0;
                waitpid(pid, &status, 0);
                // 程序运行异常，一定是因为因为收到了信号！
                std::cout << "运行完毕, info: " << (status & 0x7F) << std::endl; 
                return status & 0x7F; //返回信号码，低七位
                //什么时候会小于0？？？ 就是上面返回的那个-1、-2，最终在cr模块被归为未知错误(系统内部错误)
                /*******************************************
                * 返回值 > 0: 程序异常了，退出时收到了信号，返回值就是对应的信号编号
                * 返回值 == 0: 正常运行完毕的，结果保存到了对应的临时文件中
                * 返回值 < 0: 内部错误
                * ******************************************/
            }
        }
    };
}