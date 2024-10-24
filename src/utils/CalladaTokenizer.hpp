#pragma once

#include <utils/Common.hpp>
#include <utils/Log.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <regex>
#include <unordered_map>
#include <algorithm>
#include <stack>

class Token {
public:
    std::string tag_name;
    std::unordered_map<std::string, std::string> props;
    std::vector<Token> children;
    std::string data;

    Token() = default;

    Token(
        std::string tag_name,
        std::unordered_map<std::string, std::string> props,
        std::vector<Token> children,
        std::string data
    ) : tag_name(tag_name),
        props(props),
        children(children),
        data(data) {}

    std::vector<Token*> get_tokens(const std::string& tag_name) {
        std::vector<Token*> results;
        for (auto& t : children) {
            if (t.tag_name == tag_name) {
                results.push_back(&t);
            }
            std::vector<Token*> child_results = t.get_tokens(tag_name);

            results.reserve(results.size() + child_results.size());
            std::copy(child_results.begin(), child_results.end(), std::back_inserter(results));
        }
        return results;
    }
    
    std::vector<Token*> get_tokens(const std::string& tag_name, const int& max_depth) {
        std::vector<Token*> results;
        for (auto& t : children) {
            if (t.tag_name == tag_name) {
                results.push_back(&t);
            }

            if (max_depth > 1) {
                std::vector<Token*> child_results = t.get_tokens(tag_name, max_depth - 1);

                results.reserve(results.size() + child_results.size());
                std::copy(child_results.begin(), child_results.end(), std::back_inserter(results));
            }
        }
        return results;
    }

    std::vector<Token*> get_tokens(
        const std::string& tag_name, 
        std::unordered_map<std::string, std::string> props
    ) {
        std::vector<Token*> results;

        for (auto& t : children) {
            if (t.tag_name == tag_name) {
                bool is_good = true;
                for (const auto& pair : props) {
                    auto key = pair.first;
                    auto val = pair.second;

                    auto a = t.props.find(key) == t.props.end();
                    auto b = val != t.props[key];

                    if (t.props.find(key) == t.props.end() || val != t.props[key]) {
                        is_good = false;
                    }
                }

                if (is_good) {
                    results.push_back(&t);
                }
            }
            std::vector<Token*> child_results = t.get_tokens(tag_name, props);

            results.reserve(results.size() + child_results.size());
            std::copy(child_results.begin(), child_results.end(), std::back_inserter(results));
        }
        return results;
    }
};



std::vector<Token> tokenize(const std::string& xml) {
    std::regex tag_regex("<(/?)([\\w:-]+)(\\s+(?:[^>]*?))?(/?)>");
    std::string::const_iterator search_start(xml.cbegin());
    std::smatch match;
    std::vector<Token> results;
    std::stack<Token> stack;

    while (std::regex_search(search_start, xml.cend(), match, tag_regex)) {
        std::string prefix = match.prefix();
        if (!prefix.empty() && !stack.empty()) {
            stack.top().data += Common::trim(prefix);
        }

        std::string slash = match[1];
        std::string tag_name = match[2];
        std::string props_string = Common::trim(std::string(match[3]));
        std::string self_closing = match[4];

        // Parse props
        std::unordered_map<std::string, std::string> props;
        std::vector<std::string> props_list = Common::split_string(props_string, ' ');
        for (const auto& p : props_list) {
            if (p.empty()) { 
                continue;
            } else if (p.find('=') == std::string::npos) { 
                props[p] = ""; 
                continue;
            }

            auto key_value_pair = Common::split_string(p, '=');
            auto key = Common::trim(key_value_pair[0]);
            auto value = Common::trim(key_value_pair[1]);
            value = Common::trim(value.substr(1, value.length() - 2));
            props[key] = value;
        }

        if (slash.empty() && self_closing.empty()) {
            // Opening tag
            stack.push(Token(tag_name, props, {}, ""));
        } else if (!self_closing.empty()) {
            // Self-closing tag
            Token token(tag_name, props, {}, "");
            if (!stack.empty()) {
                stack.top().children.push_back(std::move(token));
            } else {
                results.push_back(std::move(token));
            }
        } else {
            // Closing tag
            if (!stack.empty() && stack.top().tag_name == tag_name) {
                Token token = std::move(stack.top());
                stack.pop();
                if (stack.empty()) {
                    results.push_back(std::move(token));
                } else {
                    stack.top().children.push_back(std::move(token));
                }
            } else {
                Log::log_error_and_terminate("Mismatched closing tag: " + tag_name, __FILE__, __LINE__);
            }
        }

        search_start = match.suffix().first;
    }

    if (!stack.empty()) {
        stack.top().data += Common::trim(std::string(search_start, xml.cend()));
    }

    if (!stack.empty()) {
        Log::log_error_and_terminate("Unclosed tag: " + stack.top().tag_name, __FILE__, __LINE__);
    }

    return results;
}






// std::vector<Token> tokenize(const std::string& xml) {
//     std::regex opening_tag("<([\\w:-]+)(\\s+(?:[^>]*?))?(/?)>");
//     std::string::const_iterator search_start(xml.cbegin());
//     std::smatch opening_match;
//     std::smatch closing_match;
//     std::vector<Token> results;
//     // Token token;

//     while (std::regex_search(search_start, xml.cend(), opening_match, opening_tag)) {
//         auto tag_name = std::string(opening_match[1]);
//         auto props_string = Common::trim(std::string(opening_match[2]));
//         auto self_closing_char = std::string(opening_match[3]);

//         // Parse the props into a map.
//         std::unordered_map<std::string, std::string> props;
//         std::vector<std::string> props_list = Common::split_string(props_string, ' ');
//         for (const auto& p : props_list) {
//             if (p.empty()) { 
//                 continue;
//             } else if (p.find('=') == std::string::npos) { 
//                 props[p] = ""; 
//                 continue;
//             }

//             auto key_value_pair = Common::split_string(p, '=');
//             auto key = Common::trim(key_value_pair[0]);
//             auto value = Common::trim(key_value_pair[1]);
//             value = Common::trim(value.substr(1, value.length() - 2));
//             props[key] = value;
//         }

//         // Is it a self closing tag?
//         if (!self_closing_char.empty()) {
//             results.emplace_back(
//                 tag_name,
//                 props,
//                 std::vector<Token>(),
//                 ""
//             );
//             search_start = opening_match.suffix().first;
//             continue;
//         }

//         std::regex closing_tag("</\\s*(" + tag_name + ")\\s*>");
//         std::regex_search(opening_match.suffix().first, xml.cend(), closing_match, closing_tag);
        
//         auto content_start = opening_match.suffix().first;
//         auto content_end = closing_match.prefix().second;
//         auto content = std::string(content_start, content_end);

//         auto children = tokenize(content);
//         auto data = std::string("");
        
//         // If there are no children, then this is a leaf node.
//         if (children.empty()) {
//             data = Common::trim(content);
//             data = std::regex_replace(data, std::regex(R"(\s+)"), " ");
//         }

//         results.emplace_back(
//             tag_name,
//             props,
//             children,
//             data
//         );
//         search_start = closing_match.suffix().first;
//     }

//     return results;   
// }