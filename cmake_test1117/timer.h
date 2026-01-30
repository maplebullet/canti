#ifndef TIMER_H
#define TIMER_H
#pragma once
#include <time.h>
#include <iostream>
#include <map>
#include <Windows.h>
class myTimer
    {
    public:
    myTimer(){}
    ~myTimer(){}
        void start(std::string modelName) {
            double time0 = static_cast<double>(clock())/1000;
            startTime[modelName] = time0;
        }
        double end(std::string modelName) {
            if (startTime.count(modelName) == 0) {
                std::cout << "Timer: don`t have " << modelName << "model start time point" << std::endl;
                return 0;
            }
            double time0 = static_cast<double>(clock())/1000;
            double time = (time0 - startTime[modelName]);
            std::cout << "Timer:"<<  modelName<< " run time : " << time << " s" << std::endl;
            std::map<std::string, double>::iterator it;
            it = startTime.find(modelName);
            startTime.erase(it);
            return time;
        }
        double getStartTime(){
            return static_cast<double>(clock())/1000;
        }
    private:
        std::map<std::string, double> startTime;
    };

#endif // TIMER_H
