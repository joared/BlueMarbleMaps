#ifndef GEOGUESSGAME
#define GEOGUESSGAME

#include "Feature.h"
#include "DataSet.h"

#include <random>
#include <set>

using namespace BlueMarble;

class GeoGuesGame
{
    public:
        // class Country
        // {
        //     public:
        //         Country(FeaturePtr countryFeature)
        //         {

        //         }
        //     private:
        //         FeaturePtr m_feature;
        // }
        GeoGuesGame(Map& map, DataSet& dataSet)
            : m_features()
            , m_current(0)
            , m_gen(std::random_device()())
            , m_nGuesses(0)
            , m_nCorrectGuesses(0)
            , m_map(map)
            , m_isStarted(false)
            , m_isFinnished(false)
            , m_startTimeMs(0)
            , m_startBounds()
        {
            // dataSet.onGetFeaturesRequest(Attributes(), m_features);
            // for (auto f : m_features)
            // {
            //     m_uniqueCountryNames.insert(f->attributes().get<std::string>("NAME"));
            // }
            // for (auto c : m_uniqueCountryNames)
            // {
            //     m_remainingCountries.push_back(c);
            // }
            // m_current = randomize();
        }

        bool isStarted() { return m_isStarted; }
        bool isFinnished() { return m_isFinnished; }
        int elapsedMs() 
        { 
            if (isFinnished())
                return m_finnishedElapsedMs;
            return getTimeStampMs()-m_startTimeMs; 
        }
        const Rectangle& bounds() { return m_bounds; }
        void start(const Rectangle& bounds)
        {
            m_startBounds = m_map.lngLatToMap(bounds);
            m_startMapConstraints = m_map.mapConstraints().bounds();
            m_startBounds.scale(2.0);
            m_uniqueCountryNames.clear();
            m_remainingCountries.clear();
            m_nCorrectGuesses = 0;
            m_nGuesses = 0;
            m_features.clear();
            m_isFinnished = false;
            m_isStarted = true;
            std::vector<FeaturePtr> features;
            m_map.getFeatures(Attributes(), features);

            // Filter all features outside bounds
            for (auto f : features)
            {
                if(f->isInside(bounds))
                {
                    m_features.push_back(f);
                }
            }

            // Restrict map to the bounds of the features
            FeatureCollection fColl;
            for (auto f: m_features) { fColl.add(f); }
            m_bounds = m_map.mapConstraints().bounds() = m_map.lngLatToMap(fColl.bounds());

            for (auto f : m_features)
            {
                m_uniqueCountryNames.insert(f->attributes().get<std::string>("NAME"));
            }
            for (auto c : m_uniqueCountryNames)
            {
                m_remainingCountries.push_back(c);
            }
            m_current = randomize();
            m_startTimeMs = getTimeStampMs();
        }

        void stop()
        {
            m_isStarted = false;
            m_isFinnished = false;
            m_map.mapConstraints().bounds() = m_startMapConstraints;
            m_map.zoomToArea(m_startBounds, true);
        }

        int randomize()
        {
            std::uniform_int_distribution<> distrib(0, m_remainingCountries.size()-1);
            return distrib(m_gen);
        }
        
        int nGuess() { return m_nGuesses; }
        int nCorrect() { return m_nCorrectGuesses; }
        int nTot() { return m_uniqueCountryNames.size(); }

        std::string currentCountryName()
        {
            return m_remainingCountries[m_current];
        }

        FeatureCollection correctFeatures()
        {
            FeatureCollection features;
            for (auto f : m_features)
            {
                if (f->attributes().get<std::string>("NAME") == currentCountryName())
                    features.add(f);
            }

            return features;
        }

        bool onClick(int X, int Y)
        {
            auto features = m_map.featuresAt(X, Y, 10.0);
            if (features.empty())
                return true;
            
            auto f = features[0];

            if (m_map.isSelected(f))
            {
                auto featColl = correctFeatures();
                if (!guess(f))
                {
                    m_map.deSelectAll();
                    for (auto f : featColl)
                    {
                        m_map.select(f, SelectMode::Add);
                    }
                    auto bounds = m_map.lngLatToMap(featColl.bounds());
                    m_map.zoomToArea(bounds, true);
                }
                else
                {
                    m_map.deSelectAll();
                }
            }
            else
            {
                m_map.select(f);
            }
            m_map.update(true);
        }

        bool guess(FeaturePtr feature)
        {
            auto guess = feature->attributes().get<std::string>("NAME");
            bool correct = guess == currentCountryName();
            
            m_nGuesses++;
            if (correct)
            {
                m_nCorrectGuesses++;
                m_remainingCountries.erase(m_remainingCountries.begin() + m_current);
            }

            if (m_remainingCountries.empty())
            {
                m_finnishedElapsedMs = elapsedMs();
                m_isStarted = false;
                m_isFinnished = true;
                m_map.mapConstraints().bounds() = m_startMapConstraints;
                m_map.zoomToArea(m_startBounds, true);
                return correct;
            }

            m_current = randomize();

            return correct;
        }

    private:
        std::vector<FeaturePtr>  m_features;
        std::set<std::string>    m_uniqueCountryNames;
        std::vector<std::string> m_remainingCountries;
        int                      m_current;
        std::mt19937             m_gen;
        int                      m_nGuesses;
        int                      m_nCorrectGuesses;
        Map&                     m_map;
        bool                     m_isStarted;
        bool                     m_isFinnished;
        int                      m_startTimeMs;
        int                      m_finnishedElapsedMs;
        Rectangle                m_startBounds;
        Rectangle                m_startMapConstraints;
        Rectangle                m_bounds; // Bounding rect of all features
};

#endif /* GEOGUESSGAME */
