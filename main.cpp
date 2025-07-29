/*
 * FastCGI back-end for the study scheduler.
 * Build:
 *   g++ -std=c++17 -lfcgi++ -lfcgi main.cpp -o scheduler.fcgi
 * Launch (example):
 *   spawn-fcgi -p 9000 ./scheduler.fcgi
 * Then proxy /schedule POST requests from Nginx/Apache to 127.0.0.1:9000
 *
 * Dependencies:   libfcgi-dev   nlohmann/json (header-only)
 */
#include <fcgio.h>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <ctime>
#include <sstream>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using Task = std::tuple<int,std::string,double>; // (priority, subject, hours)

struct Compare {
    bool operator()(const Task& a, const Task& b) const {
        return std::get<0>(a) > std::get<0>(b); // min-heap on priority
    }
};

int main() {
    FCGX_Request request;
    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    while (FCGX_Accept_r(&request) == 0) {
        std::string content;
        char* clen = FCGX_GetParam("CONTENT_LENGTH", request.envp);
        if (clen) {
            int len = std::stoi(clen);
            char* buf = new char[len];
            FCGX_GetStr(buf, len, request.in);
            content.assign(buf, len);
            delete[] buf;
        }

        json input = json::parse(content, nullptr, false);
        json output;

        if (!input.is_discarded()) {
            // ---------- scheduler logic ----------
            std::vector<std::string> subjects;
            std::stringstream ss(input["subjects"].get<std::string>());
            std::string subj;
            while (std::getline(ss, subj, ',')) {
                if(!subj.empty()) subjects.push_back(subj);
            }

            int days        = input["days"];
            double hoursDay = input["hoursPerDay"];
            std::priority_queue<Task,std::vector<Task>,Compare> pq;

            // initialise all subjects with priority 0 (to be scheduled earliest)
            for (auto& s : subjects) pq.emplace(0, s, hoursDay / subjects.size());

            json plan = json::array();
            for (int d = 1; d <= days; ++d) {
                Task best = pq.top(); pq.pop();
                int   nextPri = std::get<0>(best) + 2;   // simple spacing: +2 days every occurrence
                std::string subj = std::get<1>(best);
                double hrs  = std::get<2>(best);

                plan.push_back({{"day",d},{"subject",subj},{"hours",hrs}});

                pq.emplace(nextPri, subj, hrs);          // re-insert with higher priority value
            }
            output["plan"] = plan;
        } else {
            output["error"] = "Invalid JSON";
        }

        std::string body = output.dump();
        std::ostringstream oss;
        oss << "Status: 200 OK\r\n"
            << "Content-Type: application/json\r\n"
            << "Content-Length: " << body.size() << "\r\n\r\n"
            << body;

        FCGX_PutStr(oss.str().c_str(), oss.str().length(), request.out);
        FCGX_Finish_r(&request);
    }
    return 0;
}
