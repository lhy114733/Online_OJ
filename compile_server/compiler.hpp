#pragma once



#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "../comm/util.hpp"


namespace ns_compiler
{
    //在这里用命名空间
    using namespace ns_util;
    class Compiler 
    {
        public:
        Compiler()
        {}
        ~Compiler()
        {}
        static bool Compile(const std::string& file_name)
        {
            pid_t pid = fork();
            if(pid < 0)
            {
                std::cout << "fork失败" <<std::endl;
                return false;
                //这里也应该是exit()吧，后续看一下
            }
            else if(pid ==0 ) //子进程
            {
                umask(0);
                //注意：O_CREAT 必须带第三个参数（文件权限），否则 mode 是未定义/0，文件权限变成 0000
                //O_TRUNC 也必须加，否则复用时旧内容会残留在文件里
                int compile_err = open(PathUtil::CompilerErr(file_name).c_str(),O_CREAT | O_WRONLY | O_TRUNC, 0644);
                if(compile_err < 0) 
                {
                    std::cout << "Compile_Err文件创建打开失败" << std::endl;
                    // return false;
                    // 注意这里直接终止程序，而不是return false，退出码设置为1，理解两者的区别
                    // return false应该是被编译的程序有错误而失败，而这里的错误是系统造成的，
                    // 不是人家代码的问题，应该是exit(1)
                    exit(1);   
                    //好像没啥用，直接终止了子进程，父进程也没回收退出码，这直接到"编译成功"了，是一个bug？？？？
                }
                //重定向标准错误
                dup2(compile_err,2);
                //g++ file_name file_name.cc
                execlp("g++","g++",\
                PathUtil::Src(file_name).c_str(),"-o",PathUtil::Exe(file_name).c_str(), nullptr);
                
                //走到这里说明g++失败了
                std::cout << "g++启动失败，可能是参数问题" << std::endl;
                exit(2); //exit理由同上，有bug？？？？
            }
            else 
            {
                waitpid(pid,nullptr,0); //事实上并未对子进程退出码关心，
                // 如果是因为1、2(系统内部错误)两种退出，那按现在代码结果为因为没形成exe文件而被认为是false，
                // 被误认为是用户提交的代码错误，compile_err文件里啥也没有，用户也不知道啥错了，是一个很严重的bug
                //                   bug？？？？？？？？？？？？？？？？？？？
                //    正常来说子进程退出码是1、2既不是false(编译错误)也不是true(编译成功)，最终在cr模块被识别未-2的状态码，reason未系统内的未知错误
                if(FileUtil::IsFileExists(PathUtil::Exe(file_name)))
                {
                    std::cout << "编译成功" << std::endl;
                    return true;
                }
            }
            // 编译失败
            std::cout << "编译失败,没有形成可执行程序" <<std::endl;
            return false;
        }
    };
}