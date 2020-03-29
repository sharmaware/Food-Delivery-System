
#ifndef EXPANDABLE_HASH_MAP_h
#define EXPANDABLE_HASH_MAP_h

#include <list>
#include <iostream>
// ExpandableHashMap.h

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
    ExpandableHashMap(double maximumLoadFactor = 0.5);
    ~ExpandableHashMap();
    void reset();
    int size() const;
    void associate(const KeyType& key, const ValueType& value);

      // for a map that can't be modified, return a pointer to const ValueType
    const ValueType* find(const KeyType& key) const;

      // for a modifiable map, return a pointer to modifiable ValueType
    ValueType* find(const KeyType& key)
    {
        return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
    }

      // C++11 syntax for preventing copying and assignment
    ExpandableHashMap(const ExpandableHashMap&) = delete;
    ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
    struct Bucket{
        KeyType key;
        ValueType value;
    };
    
    std::list<Bucket>* map;
    
    double maxLoad;
    int numElements;
    int sizeArray;
    
    unsigned int getBucketNumber(const KeyType& key) const;
    
    void resize(int size);
    
    
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor)
{
    map = new std::list<Bucket>[8];
    maxLoad = maximumLoadFactor;
    sizeArray = 8;
    numElements = 0;
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
    delete [] map;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
    delete [] map;
    map = new std::list<Bucket>[8];
    sizeArray = 8;
    numElements = 0;
    
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
    return numElements;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{

    //see if item with the given key already exists
    ValueType* foundValue = find(key);
    
    if(foundValue == nullptr){ //if an association with the given key does not exist
        
        //    if load > max load, resize
        double size = sizeArray;
        if((numElements + 1)/ size > maxLoad){
            resize(sizeArray * 2);
        }
        
        //create a new association by using the hash function to get the index, and place a new bucket with the key and value at the end of the linked list at the index in the array
        int index = getBucketNumber(key);
        Bucket b;
        b.key = key;
        b.value = value;
        map[index].push_back(b);
        numElements++;
    }
    else{
        *foundValue = value; //if an association with the given key exists, change the value to the provided value
    }
    
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
    int index = getBucketNumber(key); //get the index of the array that this key should be stored at
        for(auto it = map[index].begin(); it != map[index].end(); it++){ //check the linked list for the key
            if(it->key == key){ //if the key matches one of the buckets, return a pointer to the value
                return &(it->value);
            }
        }

    return nullptr;
}

template<typename KeyType, typename ValueType>
unsigned int ExpandableHashMap<KeyType, ValueType>::getBucketNumber(const KeyType& key) const
{
    unsigned int hasher(const KeyType& k); // prototype
    unsigned int h = hasher(key); //get the value returned by the hash function
    return h % sizeArray; //mod the value by the size of the array to make it fit in the array
}


template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::resize(int size){
    int currentSize = sizeArray;
    sizeArray = size; //change the size so that the getBucketNumber function will mod with the new size
    
    std::list<Bucket>* newMap = new std::list<Bucket> [size]; //create a new array of linked lists that will be the new map
   
    for(int i = 0; i < currentSize; i++){ //for each index in the array
        for(auto it = map[i].begin(); it != map[i].end(); it++){ // for each element in the linked list at the array
            Bucket b; //create a new bucket
            b.key = it->key;
            b.value = it->value;
            int index = getBucketNumber(b.key); //get the index for the newMap
            newMap[index].push_back(b); //place the new bucket the end of the linked list at the index in the new Map
            
        }
    }
    
    delete [] map; //delete the old map
    map = newMap; //set the map member to point to the new map
}

#endif
