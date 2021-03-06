#include "pass_manager.h"
#include <vector>
#include "satellite_pass.h"
#include "database/database.h"
#include "logger/logger.h"
#include "scheduler/scheduler.h"
#include "orbit_predictor.h"
#include "tle_manager.h"
#include "processing/pass_processing.h"

// Schedule upcoming passes
void schedulePasses()
{
    std::vector<SatellitePass> passes;

    for (SatelliteConfig &satellite : databaseManager->getAllSatellites())
    {
        // Predict passes fullfilling requested configuration and add store them
        logger->info("Predicting passes for " + getTLEFromNORAD(satellite.norad).name);
        if (getTLEFromNORAD(satellite.norad).name.find("No TLE found") != std::string::npos)
            continue;
        OrbitPredictor predictor(satellite.norad, getTLEFromNORAD(satellite.norad), configManager->getConfig().station);
        std::vector<SatellitePass> predictedPasses = predictor.getPassesBetweenOver(time(NULL), time(NULL) + 24 * 60 * 60, satellite.min_elevation);
        passes.insert(passes.end(), predictedPasses.begin(), predictedPasses.end());
    }

    // Sort by order (AOS)
    std::sort(passes.begin(), passes.end(), [](const SatellitePass &v1, const SatellitePass &v2) { return v1.aos < v2.aos; });

    for (SatellitePass pass : passes)
    {
        // Write to console and schedule in our scheduler
        std::tm *timeReadable = gmtime(&pass.aos);
        logger->info("Scheduling pass of " + pass.tle.name + " at " + std::to_string(timeReadable->tm_hour) + ":" + (timeReadable->tm_min > 9 ? std::to_string(timeReadable->tm_min) : "0" + std::to_string(timeReadable->tm_min)) + " with " + std::to_string(std::round(pass.elevation * 10) / 10) + "° elevation, " + (pass.northbound ? "Northbound" : "Southbound"));
        globalScheduler->in(std::chrono::system_clock::from_time_t(pass.aos), processPass, pass);
    }
}

void initPassManager()
{
    // Initial pass prediction + scheduler task
    logger->info("Starting pass scheduler...");
    schedulePasses();
    globalScheduler->every(std::chrono::system_clock::duration(std::chrono::seconds(24 * 60 * 60)), schedulePasses);
}