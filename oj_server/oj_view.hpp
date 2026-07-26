#pragma once
#include<iostream>
#include<string>
#include<ctemplate/template.h>
#include"oj_model2.hpp"

namespace ns_view
{
    using namespace ns_model;
    const std::string template_path = "./template_html/";
    class View
    {
    public:
        View(){}
        ~View(){}

        void AllExpandHtml(const vector<struct Question> &questions,std::string *html)
        {
            // 题目编号 题目标题 题目难度 
            // 推荐使用表格显示
            // 1. 形成路径
            std::string src_html = template_path + "all_questions.html";
            // 2. 形成数字典
            ctemplate::TemplateDictionary root("all_questions");
            for(const auto &q :questions)
            {
                ctemplate::TemplateDictionary *sub = root.AddSectionDictionary("question_list");
                sub->SetValue("number",q.number);
                sub->SetValue("title",q.title);
                sub->SetValue("star",q.star);

            }
            // 3. 获取被渲染的网页
            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html,ctemplate::DO_NOT_STRIP);

            // 4. 开始渲染
            tpl->Expand(html,&root);
        }
        void OneExpandHtml(const struct Question &q,std::string *html)
        {
            std::string src_html = template_path + "one_question.html";
            ctemplate::TemplateDictionary root("one_question");
            root.SetValue("number",q.number);
            root.SetValue("title",q.title);
            root.SetValue("star",q.star);
            root.SetValue("desc",q.desc);
            root.SetValue("pre_code",q.header);

            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html,ctemplate::DO_NOT_STRIP);
            tpl->Expand(html,&root);
        }
    };
}