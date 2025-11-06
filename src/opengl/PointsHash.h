/*
 PointsHash.h
 Seperates include files, structure definitions (for comparison argument)
 and typedefs for the hash table format used by FeatureSpace and FeatureSpaceGL.

 This was placed in a seperate header to isolate errors while porting sample
 code and finalizing the definition.
 */

#ifndef POINTS_HASH_H
#define POINTS_HASH_H

// Use std::unordered_map instead of deprecated __gnu_cxx::hash_map
#include <unordered_map>

struct eqUnsignedInt
{
	bool operator()(unsigned int u1, unsigned int u2) const
	{
		return u1 == u2;
	}
};

// Use std::unordered_map instead of deprecated hash_map
typedef std::unordered_map<unsigned int, unsigned int, std::hash<unsigned int>, eqUnsignedInt> points_hash_t;

#endif //POINTS_HASH_H