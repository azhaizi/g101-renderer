// cli.hpp —— 30 行，不引第三方
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
namespace g101 {
class Args {
public:
    Args(int argc, char** argv) {
        for (int i = 1; i < argc; ++i) {
            std::string a = argv[i];
            if (a.rfind("--", 0) == 0) {
                std::string k = a.substr(2);
                std::string v = (i + 1 < argc && std::string(argv[i + 1]).rfind("--", 0) != 0) ? argv[++i] : "";
                m_[k] = v;
            } else pos_.push_back(a);
        }
    }
    bool has(const std::string& k) const { return m_.count(k) != 0; }
    std::string get(const std::string& k, const std::string& d = "") const {
        auto it = m_.find(k); return it == m_.end() ? d : it->second;
    }
    int   geti(const std::string& k, int d)   const { return has(k) ? std::stoi(get(k)) : d; }
    float getf(const std::string& k, float d) const { return has(k) ? std::stof(get(k)) : d; }
private:
    std::unordered_map<std::string, std::string> m_;
    std::vector<std::string> pos_;
};
}