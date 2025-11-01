#include "PchApp.h"

#include "CoordinateInfo.h"

#include "config.h"


/*
CoordinateInfo will return the correct geographic transform for a set of
data, if applicable.

It currently does nothing, but does it harmlessly. :)
*/

CoordinateInfo::CoordinateInfo(GDALDataset* dataset)
{
	if (dataset) {
		CPLErr err = GDALGetGeoTransform(dataset, geoTransform);
		m_hasTransform = (err == CE_None);
	} else {
		m_hasTransform = false;
		for (int i = 0; i < 6; ++i) geoTransform[i] = 0.0;
	}
}
		
CoordinateInfo::~CoordinateInfo(void)
{
	return;
}

void CoordinateInfo::pixelToWorld(double px, double py, double* outX, double* outY) const
{
	if (outX) *outX = geoTransform[0] + geoTransform[1] * px + geoTransform[2] * py;
	if (outY) *outY = geoTransform[3] + geoTransform[4] * px + geoTransform[5] * py;
}
