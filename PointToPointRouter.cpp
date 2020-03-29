#include "provided.h"
#include <list>
#include <iostream>
#include <queue>
#include "ExpandableHashMap.h"


using namespace std;

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
    
private:
    const StreetMap* m_sm;
    struct Node{
        GeoCoord g;
        double f_value;
        double g_value;
        StreetSegment parent;
        
    
        bool operator< (const Node &other) const{ //the less than operator return true when its value is greater than the other to create a min heap based on the f value of the Node. By default, priority queue is a max heap.
            if(f_value > other.f_value){
                return true;
            }
            return false;
        }
        
        bool operator>= (const Node &other) const{ //the greater than operater returns true when its value is smaller than the other
            if(f_value <= other.f_value){
                return true;
            }
            return false;
        }
    
    };
    
    void returnPath(const GeoCoord& start, const GeoCoord& end,  ExpandableHashMap<GeoCoord, Node> &closedList, list<StreetSegment>& route, double& totalDistanceTravelled) const;
    
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
{
    m_sm = sm;
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}



DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route ,
        double& totalDistanceTravelled) const
{
    
    //If start is the same as end
    if(start == end){
        totalDistanceTravelled = 0;
        return DELIVERY_SUCCESS;
    }
    
    
    //checking for bad coord
    vector<StreetSegment> segs;
    if(m_sm->getSegmentsThatStartWith(start, segs) == false|| m_sm->getSegmentsThatStartWith(end, segs) == false){
        return BAD_COORD;
    }
    
    //creating a heap and map for openlist (for positions that you have yet to explore)
    priority_queue<Node> openListQueue;
    ExpandableHashMap<GeoCoord, Node> openListMap;

    //creating a map for closedlist
    ExpandableHashMap<GeoCoord, Node> closedList;

    totalDistanceTravelled = 0;
    Node startNode;
    startNode.g = start;
    startNode.f_value = 0;
    startNode.g_value = 0;
    openListQueue.push(startNode);
    openListMap.associate(startNode.g, startNode);



    while(!openListQueue.empty()){

        //find Node with lowest f value
        Node q = openListQueue.top(); //current position that we will explore in this iteration

        openListQueue.pop();
        Node* n = openListMap.find(q.g);
        n->f_value = -1; //-1 signifies a deleted value from the map

        vector<StreetSegment> successors; //vector to hold the successors from the current position
        m_sm->getSegmentsThatStartWith(q.g, successors);


        for(int i = 0; i < successors.size(); i++){
            if(successors[i].end == end){ //path found, end search
                //adding last nodes to closed list
                Node endNode;
                endNode.g = successors[i].end;
                endNode.parent = successors[i];

                closedList.associate(endNode.g, endNode);
                
                closedList.associate(q.g, q);
                
                returnPath(start, end, closedList, route, totalDistanceTravelled); //retracing steps to return the path
                return DELIVERY_SUCCESS;
            }

            //calculating the f value for the current successors
            double g = q.g_value + distanceEarthKM(q.g, successors[i].end);
            double h = distanceEarthKM(successors[i].end, end);
            double f = g + h;

            bool skip_this_successor = false;

            //if the position exists in openList with a smaller f value, skip this successor
            Node* found_in_openList = openListMap.find(successors[i].end);
            if(found_in_openList != nullptr && found_in_openList->f_value != -1  && found_in_openList->f_value < f  ){ //is already in openlist, skip
                skip_this_successor = true;
            }

            if(skip_this_successor){
                continue;
            }

            //if this position is in the closed list and has a smaller f value, skip this successor
            Node* found_in_closedList = closedList.find(successors[i].end);
            if(found_in_closedList != nullptr  && found_in_closedList->f_value < f  ){
                skip_this_successor = true;
            }

            //if this successor is not skipped, add it to the open list
            if(!skip_this_successor){
                Node newNode;
                newNode.f_value = f;
                newNode.g_value = g;
                newNode.g = successors[i].end;
                newNode.parent = successors[i];
                openListQueue.push(newNode);
                openListMap.associate(newNode.g, newNode);
                
                
            }
        }
        //add q to the closed list because it has been explored
        closedList.associate(q.g, q);

    }

    //all attempts have been exhausted and there is no route
    return NO_ROUTE;
}

void PointToPointRouterImpl::returnPath(const GeoCoord& start, const GeoCoord& end,  ExpandableHashMap<GeoCoord, Node> &closedList, list<StreetSegment>& route, double& totalDistanceTravelled) const{
    
    //starting from the end note, use the parent pointers to recreate the path that it took to get to the solution

    GeoCoord toFind = end;
    Node* node = closedList.find(toFind);

    while(node->parent.start != start){
        route.push_front(node->parent);
        totalDistanceTravelled += distanceEarthMiles(node->parent.end, node->parent.start);
        toFind = node->parent.start;

        node = closedList.find(toFind);
    }
    
    totalDistanceTravelled += distanceEarthMiles(node->parent.end, node->parent.start);
    route.push_front(node->parent);

}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}

