#pragma once
#include <iostream>
using namespace std;

class Parser
{
private:

public:
    void parse_add(char* buffer, long long &id, vector<float>& v)
    {
        id = -1;
        v.clear();
        char *tok = strtok(buffer, " ");
        if (!tok)
            return;
        id = stoll(tok);
        tok = strtok(NULL, " ");
        if (!tok)
            return;
        while (tok != NULL)
        {
            v.push_back(stof(tok));
            tok = strtok(NULL, " ");
        }
    }
    void parse_search(char* buffer, int dim, vector<float>& v, string& mode, int& k, int& nprobe)
    {
        v.clear();
        mode.clear();
        k =-1;
        nprobe =-1;
        char *tok = strtok(buffer, " ");
        if (!tok)
            return;
        for (int i = 0; i < dim && tok != NULL; i++)
        {
            v.push_back(stof(tok));
            tok = strtok(NULL, " ");
        }
        if (!tok)
            return;
        k = stoi(tok);
        tok = strtok(NULL, " ");
        if (!tok)
            return;
        mode = tok;
        tok = strtok(NULL, " ");
        if (!tok || mode == "BRUTE" || mode != "IVF")
            return;
        nprobe = stoi(tok);
    }
};
