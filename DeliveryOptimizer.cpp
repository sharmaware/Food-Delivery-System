#include "provided.h"
#include <vector>
using namespace std;

class DeliveryOptimizerImpl
{
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const;
private:
    double acceptanceProbability(double energy, double newEnergy, double temp) const;
    const StreetMap* m_sm;
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
{
    m_sm = sm;
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

double DeliveryOptimizerImpl::acceptanceProbability(double energy, double newEnergy, double temp) const{
    if(newEnergy < energy){
        return 1.0;
    }
    
    return exp((energy - newEnergy)/temp);
    
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
    oldCrowDistance = 0;
    newCrowDistance = 0;
    
    oldCrowDistance += distanceEarthMiles(depot, deliveries[0].location);
    for(int i = 0; i < deliveries.size()-1; i++){
        oldCrowDistance += distanceEarthMiles(deliveries[i].location, deliveries[i+1].location);
    }
    oldCrowDistance += distanceEarthMiles(depot, deliveries[deliveries.size()-1].location);
    
    int distance = oldCrowDistance;
    double temperature = 10000;
    double coolingFactor = 0.3;
    vector<DeliveryRequest> currentSolution (deliveries);
    vector<DeliveryRequest> bestSolution (deliveries);
    double bestDistance = oldCrowDistance;
    
    while(temperature > 1){
        vector<DeliveryRequest> newDeliveries (currentSolution);
        
        int randPos1 = rand() % newDeliveries.size(); //creating 2 random indexes
        int randPos2 = rand() % newDeliveries.size();
        
        //swapping the elements at the 2 indexes
        DeliveryRequest temp = newDeliveries[randPos1];
        newDeliveries[randPos1] = newDeliveries[randPos2];
        newDeliveries[randPos2] = temp;
        
        //calculating the distance with the new order
        double newDistance = 0;
        newDistance += distanceEarthMiles(depot, newDeliveries[0].location);
        for(int i = 0; i < newDeliveries.size()-1; i++){
            newDistance += distanceEarthMiles(newDeliveries[i].location, newDeliveries[i+1].location);
        }
        newDistance += distanceEarthMiles(depot, newDeliveries[newDeliveries.size()-1].location);
        
       //random probability
        double randProb = (rand() % 100)/ 100;
        
        //accept the solution if it is shorter, or longer with a certain probability according to the acceptanceProbability function
        if(acceptanceProbability(newDistance, distance, temperature) > randProb){
            currentSolution = newDeliveries;
        }
        
        //calc current solution distance and compare with best
        double currentDistance = 0;
        currentDistance += distanceEarthMiles(depot, currentSolution[0].location);
        for(int i = 0; i < currentSolution.size()-1; i++){
            currentDistance += distanceEarthMiles(currentSolution[i].location, currentSolution[i+1].location);
        }
        currentDistance += distanceEarthMiles(depot, currentSolution[currentSolution.size()-1].location);
        
        //if the new solution is better than the best solution, store it as the best solution
        if(currentDistance < bestDistance){
            bestDistance = currentDistance;
            bestSolution = currentSolution;
        }
        
        temperature *= 1-coolingFactor; //decrease the temperature
    }
    
    for(int i = 0; i < deliveries.size(); i++){ //copy the bestSolution into deliveries
        deliveries[i] = bestSolution[i];
    }
    
    newCrowDistance = bestDistance; //update the distance of the new ordering
}


//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
