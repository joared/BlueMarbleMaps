#ifndef BLUEMARBLE_PERFORMANCEMONITOR
#define BLUEMARBLE_PERFORMANCEMONITOR

#include "Map.h"
#include "Animation.h"
#include <map>


namespace BlueMarble
{

    class Sample
    {
        public:
            Sample(const std::string& name)
                : m_name(name)
                , m_sum(0.0)
                , m_nSamples(0)
                , m_startTimeStamp(-1)
            {}

            // Sample(const Sample& other)
            //     : m_name(other.m_name)
            //     , m_sum(0.0)
            //     , m_nSamples(0)
            //     , m_startTimeStamp(-1)
            // {}

            Sample()
                : m_name("Error sample")
                , m_sum(0.0)
                , m_nSamples(0)
                , m_startTimeStamp(-1)
            {}

            void sampleStart(int timeStamp)
            {
                assert(m_startTimeStamp == -1);
                m_startTimeStamp = timeStamp;
            }

            void sampleEnd(int timeStamp)
            {
                assert(m_startTimeStamp != -1);

                double elapsed = timeStamp - m_startTimeStamp;
                m_sum += elapsed;
                m_nSamples++;
                m_startTimeStamp = -1;
            }

            const std::string& name() { return m_name; }
            int totalTime() { return m_sum; }
            int nSamples() { return m_nSamples; }
            double average() { return m_sum/(double)m_nSamples; }
            std::string toString() 
            {
                std::string s;
                s += "[" + m_name + "]";
                s += " avg: " + std::to_string(average());
                s += " tot: " + std::to_string(totalTime());
                s += " n: " + std::to_string(nSamples());

                return s;
            }
        private:
            std::string m_name;
            double      m_sum;
            int         m_nSamples;
            int         m_startTimeStamp;
    };

    class PerformanceSampler
    {
        public:
            PerformanceSampler();
            Sample& getSample(const std::string& sampleKey);
            void sampleStart(const std::string& sampleKey, int timeStamp);
            void sampleEnd(const std::string& sampleKey, int timeStamp);
            std::vector<Sample> getSamples();
        private:
            std::map<std::string, Sample> m_operationNameToSampleMap;
    };

    class PerformanceReport
    {
        public:
            PerformanceReport(const PerformanceSampler& sampler);
            std::string toString();
        private:
            PerformanceSampler m_sampler;
    };

    class PerformanceMonitor : private MapEventHandler
    {
        public:
            PerformanceMonitor(Map& map);
            void start();
            PerformanceReport stop();
            PerformanceReport staticStartStop(int durationMs);
            bool isRunning();
        private:
            void OnAreaChanged(Map& map) override final {}; // Nothing
            void OnUpdating(Map& map) override final;
            void OnCustomDraw(Map& map) override final;
            void OnUpdated(Map& map) override final;

            Map& m_map;
            bool m_isRunning;
            PerformanceSampler m_sampler;
    };
}

#endif /* BLUEMARBLE_PERFORMANCEMONITOR */
