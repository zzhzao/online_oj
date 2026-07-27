#include "compile_run.hpp"
#include "../comm/httplib.h"
using namespace ns_compile_and_run;
using namespace httplib;

void Usage(std::string proc)
{
    std::cerr << "Usage: " << "\n\t" << proc << " port" << std::endl;
}

//./compile_server port
int main(int argc,char *argv[])
{

    if(argc != 2)
    {
        Usage(argv[0]);
        return 1;
    }
    Server svr;
//     {
//     "code" : "#include<iostream> \n int main(){std::cout << \"你好！\" << std::endl;}",
//     "input" : "",
//     "cpu_limit" : 1,
//     "mem_limit" : 50000
// }
    svr.Post("/compile_and_run",[](const Request &req, Response &resp){
        std::string in_json = req.body;
        std::string out_json;
        if(!in_json.empty())
        {
            CompileAndRun::Start(in_json,&out_json);
            resp.set_content(out_json,"application/json;charset=utf-8");
        }

    });
    svr.listen("0.0.0.0",atoi(argv[1]));
    return 0;
}