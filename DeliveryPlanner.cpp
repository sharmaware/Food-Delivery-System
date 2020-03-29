#include "provided.h"
#include <vector>
#include "ExpandableHashMap.h"
using namespace std;

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_sm;
    string getDir(const StreetSegment s) const;
    string getTurnDir(double angle) const;
    
    
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
{
    m_sm = sm;
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

string DeliveryPlannerImpl::getDir(const StreetSegment s) const{
    
    double angle = angleOfLine(s);
    
    if(0 <= angle && angle < 22.5){
        return "east";
    }else if(22.5 <= angle && angle < 67.5){
        return "northeast";
    }else if(67.5 <= angle && angle < 112.5){
        return "north";
    }else if(112.5 <= angle && angle < 157.5){
        return "northwest";
    }else if(157.5 <= angle && angle < 202.5){
        return "west";
    }else if(202.5 <= angle && angle < 247.5){
        return "southwest";
    }else if(247.5 <= angle && angle < 292.5){
        return "south";
    }else if(292.5 <= angle && angle <337.5){
        return "east";
    }
    else if(angle >= 337.5){
        return "southeast";
    }else{
        return "error";
    }
    
}

string DeliveryPlannerImpl::getTurnDir(double angle) const{
    if(angle < 1 || angle > 359){
        return "straight";
    }
    else if(angle >= 1 && angle < 180){
        return "left";
    }
    else{
        return "right";
    }

}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    DeliveryOptimizer d (m_sm);
    vector<DeliveryRequest> newDeliveries (deliveries);
    double oldCrowDistance, newCrowDistance = 0;
    d.optimizeDeliveryOrder(depot, newDeliveries, oldCrowDistance, newCrowDistance);
    
    
    //checking for invalid depot or delivery request location
    vector<StreetSegment> segs;
    if(!m_sm->getSegmentsThatStartWith(depot, segs))
        return BAD_COORD;
    for(int i = 0; i < deliveries.size(); i++){
        if(!m_sm->getSegmentsThatStartWith(deliveries[i].location, segs)){
            return BAD_COORD;
        }
    }
    
    vector<list<StreetSegment>> routes;
    
    GeoCoord start = depot;
    GeoCoord end;

    PointToPointRouter p (m_sm);
    
    //depot to 1st point
    list<StreetSegment> initRoute;
    double initDistance = 0;
    p.generatePointToPointRoute(depot, newDeliveries[0].location, initRoute, initDistance);
    routes.push_back(initRoute);
    
    for(int i = 0; i < newDeliveries.size()-1; i++){
        GeoCoord start = newDeliveries[i].location;
        GeoCoord end = newDeliveries[i+1].location;
        
        list<StreetSegment> route;
        double totalDist = 0;
        if(p.generatePointToPointRoute(start, end, route, totalDist) == NO_ROUTE){
            return NO_ROUTE;
        }
        routes.push_back(route);
    }
    
//    end point to depot
    list<StreetSegment> finalRoute;
    double finalDistance = 0;
    p.generatePointToPointRoute(newDeliveries[newDeliveries.size()-1].location, depot, finalRoute, finalDistance);
    routes.push_back(finalRoute);
    
    
    
    //planning starting here
    for(int j = 0; j < routes.size(); j++){ //for each route
        if(routes[j].size() != 0){ //case of duplicates
            string currentName = (*(routes[j].begin())).name;
            DeliveryCommand d;
            string dir = getDir((*routes[j].begin()));
            
            double dist = 0;
            


            d.initAsProceedCommand(dir, currentName, dist);

            for(auto it = routes[j].begin(); it != routes[j].end(); it++){ //for each segment in the route

                
                
                if(currentName != it->name){ //if new street reached
            
                    commands.push_back(d);

                    //make and push turn command
                    DeliveryCommand turn;
                    list<StreetSegment>::iterator prevIt = --it;
                    it++;
                    double turnAngle = angleBetween2Lines((*(prevIt)), (*it));
                    
                    if(getTurnDir(turnAngle) != "straight"){
                        
                        turn.initAsTurnCommand(getTurnDir(turnAngle), it->name);
                        commands.push_back(turn);
                    }

                    //make new proceed command
                    double newDist = 0;
                    
                    d.initAsProceedCommand(getDir(*it), it->name, newDist);
                    d.increaseDistance(distanceEarthMiles(it->start, it->end));
                    totalDistanceTravelled += distanceEarthMiles(it->start, it->end);
                    currentName = it->name;

                }
                else{
                    //increase the distance of the proceed command
                    d.increaseDistance(distanceEarthMiles(it->start, it->end));
                    totalDistanceTravelled += distanceEarthMiles(it->start, it->end);

                }
            }
            
            commands.push_back(d);
        }

        if(j != routes.size()-1){ //end of the route, create a delivery command
            DeliveryCommand delivery;
            delivery.initAsDeliverCommand(newDeliveries[j].item);
            commands.push_back(delivery);

        }
        
    }
         
    return DELIVERY_SUCCESS;
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
