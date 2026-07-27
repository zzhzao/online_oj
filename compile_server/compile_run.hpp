#pragma once
#include "compiler.hpp"
#include "runner.hpp"
#include <jsoncpp/json/json.h>
#include "../comm/log.hpp"
#include "../comm/util.hpp"
#include <unistd.h>
namespace ns_compile_and_run
{
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_compiler;
    using namespace ns_runner;
    class CompileAndRun
    {

    public:
        // code > 0 : 收到信号导致异常崩溃
        // code < 0 ：整个过程非运行报错  代码为空，编译报错等

        static void RemoveTempFile(const std::string &file_name)
        {
            std::string _src = PathUtil::Src(file_name);
            if (FileUtil::IsFileExists(_src))
            {
                unlink(_src.c_str());
            }
            std::string _complier_error = PathUtil::CompilerError(file_name);
            if (FileUtil::IsFileExists(_complier_error))
            {
                unlink(_complier_error.c_str());
            }

            std::string _exe = PathUtil::Exe(file_name);
            if (FileUtil::IsFileExists(_exe))
            {
                unlink(_exe.c_str());
            }
            std::string _stdin = PathUtil::Stdin(file_name);
            if (FileUtil::IsFileExists(_stdin))
            {
                unlink(_stdin.c_str());
            }
            std::string _stdout = PathUtil::Stdout(file_name);
            if (FileUtil::IsFileExists(_stdout))
            {
                unlink(_stdout.c_str());
            }
            std::string _stderr = PathUtil::Stderr(file_name);
            if (FileUtil::IsFileExists(_stderr))
            {
                unlink(_stderr.c_str());
            }
            
        }
        
        static std::string CodeToDesc(int code, const std::string &file_name)
        {
            std::string desc;
            std::string _stderr;
            switch (code)
            {
            // 待完善
            case 0:
                desc = "编译运行成功";
                break;
            case -1:
                desc = "用户提交的代码为空";
                break;
            case -2:
                desc = "未知错误";
                break;
            case -3:

                FileUtil::ReadFile(PathUtil::CompilerError(file_name), &_stderr, true);
                desc = _stderr;
                break;
            case SIGABRT:
                desc = "内存超过范围";
                break;
            case SIGXCPU:
                desc = "CPU运行超时";
                break;
            case SIGFPE:
                desc = "浮点数溢出";
                break;

            default:
                desc = "未知：" + std::to_string(code);
                break;
            }
            return desc;
        }
        static void Start(const std::string &in_json, std::string *out_json)
        {
            //  输入 ： code  input
            // code : 用户提交的代码   input ： 用户提交的代码对应输入
            // cpu_limit:时间要求  mem_limit ： 空间要求
            // 输出
            // status : 状态码   reason : 请求结果
            // stdout : 运行结果   stderr  ： 错误结果
            Json::Value in_value;
            Json::Reader reader;
            reader.parse(in_json, in_value);

            std::string code = in_value["code"].asString();
            std::string input = in_value["input"].asString();
            int cpu_limit = in_value["cpu_limit"].asInt();
            int mem_limit = in_value["mem_limit"].asInt();

            Json::Value out_value;
            std::string file_name;
            int status_code = 0;
            int run_result = 0;
            if (code.size() == 0)
            {
                // out_value["status"] = -1;
                // out_value["reason"] = "用户代码是空的";
                // return;
                status_code = -1;
                goto END;
            }

            // 毫秒级时间戳 + 原子性递增 ： 来保证唯一值

            // 形成临时src文件
            file_name = FileUtil::UniqFileName();
            if (!FileUtil::WriteFile(PathUtil::Src(file_name), code))
            {
                // out_value["status"] = -2; // 写入文件失败
                // out_value["reason"] = "发生了未知错误";
                // return;
                //LOG(ERROR) << "写入文件失败" << "\n";
                status_code = -2;
                goto END;
            }
            if (!Compiler::Compile(file_name))
            {
                // out_value["status"] = -3; // 编译失败
                // out_value["reason"] = FileUtil::ReadFile(PathUtil::CompilerError(file_name));
                // return;
                status_code = -3;
                goto END;
            }
            run_result = Runner::Run(file_name, cpu_limit, mem_limit);
            if (run_result < 0)
            {
                // out_value["status"] = -2; // 内部错误
                // out_value["reason"] = "发生了未知错误";
                // return;
                //LOG(ERROR) << "内部错误" << "\n";
                status_code = -2;
                goto END;
            }
            else if (run_result > 0)
            {
                // code代表信号
                //  out_value["status"] = code;
                //  out_value["reason"] = SignoToDesc(run_result);//将信号错误转化为原因
                status_code = run_result;
            }
            else
            {
                // 运行成功
                status_code = 0;
            }
        END:
            out_value["status"] = status_code;
            out_value["reason"] = CodeToDesc(status_code, file_name);
            if (status_code == 0)
            {
                std::string _stdout;
                FileUtil::ReadFile(PathUtil::Stdout(file_name), &_stdout, true);
                out_value["stdout"] = _stdout;
                std::string _stderr;
                FileUtil::ReadFile(PathUtil::Stderr(file_name), &_stderr, true);
                out_value["stderr"] = _stderr;
            }
            Json::StyledWriter writer;
            *out_json = writer.write(out_value);
            RemoveTempFile(file_name);
        }
    };
}
