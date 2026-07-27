#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include "../comm/log.hpp"
#include "../comm/util.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<sys/time.h>
#include<sys/resource.h>
namespace ns_runner
{
    using namespace ns_log;
    using namespace ns_util;
    class Runner
    {
    public:
        Runner() {}
        ~Runner() {}

    public:
        static void SetProcLimit(int _cpu_limit, int _mem_limit)
        {
            // 限制时长
            struct rlimit cpu_rlimit;
            cpu_rlimit.rlim_max = RLIM_INFINITY;
            cpu_rlimit.rlim_cur = _cpu_limit;
            setrlimit(RLIMIT_CPU, &cpu_rlimit);

            // 运行内存限制
            struct rlimit mem_rlimit;
            mem_rlimit.rlim_max = RLIM_INFINITY;
            mem_rlimit.rlim_cur = _mem_limit * 1024;  // 转化为KB
            setrlimit(RLIMIT_AS, &mem_rlimit);
        }



        // cpu_limit 程序运行可以使用的最大cpu资源上限
        // mem_limit 程序运行可以使用的最大内存资源上限
        static int Run(const std::string &file_name,int cpu_limit,int mem_limit)
        {

            // 运行逻辑不需要判断结果正确与否

            // 一个程序在启动时
            // 标准输入 标准输出:运行输出结果 标准错误：运行错误信息

            std::string _execute = PathUtil::Exe(file_name);
            std::string _stdin = PathUtil::Stdin(file_name);
            std::string _stdout = PathUtil::Stdout(file_name);
            std::string _stderr = PathUtil::Stderr(file_name);
            umask(0);
            int _stdin_fd = open(_stdin.c_str(), O_CREAT | O_RDONLY, 0644);
            int _stdout_fd = open(_stdout.c_str(), O_CREAT | O_WRONLY, 0644);
            int _stderr_fd = open(_stderr.c_str(), O_CREAT | O_WRONLY, 0644);

            if (_stdin_fd < 0 || _stdout_fd < 0 || _stderr_fd < 0)
            {
                LOG(ERROR) << "运行时，打开标准文件失败" << std::endl;
                return -1;
            }

            pid_t pid = fork();
            if (pid < 0)
            {
                LOG(ERROR) << "运行时，创建子进程失败" << std::endl;
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);
                return -2; // 代表创建子进程失败
            }
            else if (pid == 0)
            {
                dup2(_stdin_fd, 0);
                dup2(_stdout_fd, 1);
                dup2(_stderr_fd, 2);

                SetProcLimit(cpu_limit, mem_limit);
                execl(_execute.c_str(), _execute.c_str(), NULL);
                exit(1);
            }
            else
            {
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);
                int status = 0;
                waitpid(pid, &status, 0);

                LOG(INFO) << "运行完毕, info: " << (status & 0x7F) << std::endl;
                return status & 0x7F;
            }
        }
    };
}