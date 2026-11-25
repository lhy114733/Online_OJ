#pragma once

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <vector>

#include <cassert>


#include "../comm/util.hpp"


const std::string questins_list = "./questions/questions.list";
const std::string questins_path = "./questions/";

namespace ns_model 
{
    using namespace ns_util;
    struct Question
    {
        std::string number;
        std::string title;  //题目的标题
        std::string star;   //难度: 简单 中等 困难
        int cpu_limit;      //题目的时间要求(S)
        int mem_limit;      //题目的空间要去(KB)
        std::string desc;   //题目的描述
        std::string header; //题目预设给用户在线编辑器的代码
        std::string tail;   //题目的测试用例，需要和header拼接，形成完整代码
    };

    class Model 
    {
        private:
        std::unordered_map<std::string,Question> _questions;
        public:
        Model()
        {
            assert(LoadQuestionList(questins_list));
        }
        bool LoadQuestionList(const std::string& list_file_name)
        {
            std::ifstream in(list_file_name);
            if(!in.is_open())
            {
                std::cout << "list文件打开失败" << std::endl;
                return false;
            }

            std::string line;
            std::vector<std::string> tokens;
            Question q;
            while(std::getline(in , line))
            {
                tokens.clear();
                bzero(&q,sizeof(q));
                StringUtil::SplitString(line, &tokens, " ");
                if(tokens.size() != 5)
                {
                    std::cout << "该行错误" <<std::endl;
                    continue;
                }
                q.number = tokens[0];
                q.title = tokens[1];
                q.star = tokens[2];
                q.cpu_limit = stoi(tokens[3]);
                q.mem_limit = stoi(tokens[4]);

                std::string path = questins_path;
                path += q.number;
                path += "/";

                FileUtil::ReadFile(path+"desc.txt", &(q.desc), true);
                FileUtil::ReadFile(path+"header.cpp", &(q.header), true);
                FileUtil::ReadFile(path+"tail.cpp", &(q.tail), true);

                _questions.insert({q.number,q}); //{}的作用
            }
            std::cout << "题库加载成功" << std::endl;
            in.close();
            return true;
        }
        bool GetAllQuestions(std::vector<Question>* out)
        {
            if(_questions.size() == 0)
            {
                std::cout << "获取失败" << std::endl;
                return false;
            }
            for(auto &q : _questions)
            {
                out->push_back(q.second); //注意把哈希表插入到vector
            }
            return true;
        }
        bool GetOneQuestion(const std::string& number , Question* out)
        {
            auto iter = _questions.find(number);
            if(iter == _questions.end()){
                std::cout << "没有该题目:" << number << std::endl;
                return false;
            }
            (*out) = iter->second;
            return true;
        }
    };
}