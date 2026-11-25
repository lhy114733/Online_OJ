#include "../comm/httplib.h"
#include "compile_run.hpp"



using namespace httplib;
using namespace ns_compile_run;

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cerr << "Usage:" << argv[0] << "server_port" <<std::endl;
        return 1;
    }
    Server svr;
    svr.Post("/Compile_Run" , [](const Request& req,Response& res){
        std::string in_json = req.body;
        std::string out_json;
        if(!in_json.empty())
        {
            Compile_run::start(in_json, &out_json);
            res.set_content(out_json, "application/json;charset=utf-8");
        }
    });
    svr.listen("0.0.0.0", atoi(argv[1])); //启动http服务
}
