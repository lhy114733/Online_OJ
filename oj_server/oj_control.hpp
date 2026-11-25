#include <mutex>
#include <assert.h>

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/writer.h>
#include "oj_view.hpp"
#include "oj_model.hpp"

namespace ns_control 
{
    using namespace ns_view;
    using namespace ns_model;

    class Machine 
    {
        public:
        std::string _ip;
        uint16_t _port;
        uint64_t _load;
        std::mutex* _mutex; //为啥锁用指针？？？  网络服务用的httplib，确实是并发的，存在并发问题
        //std::mutex* _mutex;
        public:
        Machine():_ip(""), _port(0), _load(0), _mutex(nullptr)
        {}
        void IncLoad()
        {
            if(_mutex) _mutex->lock();
            _load++;
            if(_mutex) _mutex->unlock();
        }
        void DesLoad()
        {
            if(_mutex) _mutex->lock();
            _load--;
            if(_mutex) _mutex->unlock();
        }
    };

    const std::string machine_conf_file_name = "./conf/service_machine.conf";
    class LoadBlance 
    {
        private:
        std::vector<Machine> _machines;
        std::vector<int> _online;
        std::vector<int> _offline;
        std::mutex _mutex;
        private:
        bool LoadMachine()
        {
            std::ifstream in(machine_conf_file_name);
            if(!in.is_open())
            {
                std::cout << "文件打开失败" << std::endl;
                return false;
            }
            std::string line;
            while(std::getline(in, line))
            {
                std::vector<std::string> tokens;
                StringUtil::SplitString(line, &tokens, " ");
                if(tokens.size() != 2)
                {
                    std::cout << "该机器加载失败" << std::endl;
                    continue;
                }
                Machine m;
                m._ip = tokens[0];
                m._port = stoi(tokens[1]);
                m._load = 0;
                m._mutex = new std::mutex();

                _online.push_back(_machines.size()); //在线机器插入一个 数组分布0:0,1:1,2:2,... 
                _machines.push_back(m);
            }
            in.close();
            return true;
        }
        public:
        LoadBlance()
        {
            assert(LoadMachine());
        }
        void SmartChoice()
        {
            for(auto m : _online)
            {
                
            }
        }
    };

    class Control 
    {
        private:
        Model _model;
        View _view;
        public:
        Control()
        {}
        bool OneQuestion(std::string number,std::string* html) //返回一个题目详细内容
        {
            Question q;
            if(!_model.GetOneQuestion(number, &q))
            {
                return false;
            }
            _view.OneExpandHtml(q, html);
            return true;
        }
        bool AllQuestions(std::string* html) //返回题目列表
        {
            std::vector<Question> qs;
            if(_model.GetAllQuestions(&qs))
            {
                return false;
            }
            _view.AllExpandHtml(qs, html);
            return true;
        }
        void Judge(const std::string& number, const std::string& in_json, std::string* out_json)
        {
            //需要调用compile server模块

            // 0. 获得要判的题目的详细信息(这是在本地的)
            Question q;
            _model.GetOneQuestion(number, &q);
            // 1. 反序列化获得的in_json(来自客户端的json)
            Json::Value in_value;
            Json::Reader reader;
            reader.parse(in_json,in_value);
            std::string code = in_value["code"].asString();
            // 2. 拼接用户提交代码和测试用例q.tail，序列化形成for_compile_json，用于编译；
            Json::Value for_compile_value;
            for_compile_value["code"] = code + '\n' + q.tail;
            for_compile_value["input"] = in_value["input"].asString();
            for_compile_value["limit_cpu"] = in_value["limit_cpu"].asInt();
            for_compile_value["limit_mem"] = in_value["limit_mem"].asInt();
            Json::StyledWriter writer;
            std::string for_compile_json = writer.write(for_compile_value);
            // 3. 负载均衡选择一个判题机器
            while(1) 
            {
                SmartChoice();

            }

        }
        ~Control()
        {}
    };
}