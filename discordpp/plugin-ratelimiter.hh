//
// Created by Aidan on 2/2/2020.
//

#pragma once

#include <boost/asio/use_future.hpp>
#include <boost/asio.hpp>
#include <boost/chrono.hpp>

namespace discordpp{
    std::size_t getLimitedRoute(const std::string &route/*, const bool& useCache = false*/);

    template<class BASE>
    class PluginRateLimiter: public BASE, virtual BotStruct{
    public:
        virtual std::future<json>
        call(const std::string &requestType, const std::string &targetURL, const json &body = {}) override{
            boost::chrono::time_point<boost::chrono::steady_clock> when = boost::chrono::steady_clock::now();
            size_t limitRoute = getLimitedRoute("/" + requestType + targetURL);
            {
                auto bucket = limitBucket.find(limitRoute);
                if(bucket != limitBucket.end()){
                    auto time = limited.find(bucket->second);
                    if(time != limited.end()){
                        when = time->second;
                    }
                }
            }

            //std::future res = boost::asio::steady_timer(*aioc, when).async_wait(boost::asio::use_future);

            auto out = BASE::call(requestType, targetURL, body);
            //if[out][]
            return out;
        }

    private:
        std::map<size_t, std::string> limitBucket;
        std::map<std::string, boost::chrono::time_point<boost::chrono::steady_clock>> limited;
    };

    std::size_t getLimitedRoute(const std::string &route/*, const bool& useCache = false*/){
        /*static std::map<std::size_t, std::size_t> cache;
        if(useCache){
            auto entry = cache.find(std::hash<std::string>{}(route));
            if(entry != cache.end()){
                return entry->second;
            }
        }*/
        std::ostringstream out;
        size_t last = route.find('\\');
        size_t next = route.find('\\', last + 1);
        std::string lastItem;
        while(last != std::string::npos){
            std::string item = route.substr(last, next);
            if(std::find_if_not(item.begin(), item.end(), [](char c){return isalpha(c);}) == item.end() ||
               lastItem == "channels" || lastItem == "guilds" || lastItem == "webhooks"){
                out << item;
            }else{
                out << "|";
            }
            lastItem = item;
        }
        std::size_t bucket = std::hash<std::string>{}(out.str());
        /*if(useCache){
            cache[std::hash<std::string>{}(route)] = bucket;
        }*/
        return bucket;
    }
}
