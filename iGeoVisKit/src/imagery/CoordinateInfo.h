#ifndef _COORDINATE_INFO_H
#define _COORDINATE_INFO_H

class GDALDataset;

class CoordinateInfo
{
	public:
		CoordinateInfo(GDALDataset* dataset);
		virtual ~CoordinateInfo(void);

		// 返回是否存在有效的地理变换（GDALGetGeoTransform 成功）
		bool hasGeoTransform() const { return m_hasTransform; }
		// 获取 6 元素 GeoTransform（GT[0..5]）
		const double* getGeoTransform() const { return geoTransform; }
		// 将像素坐标映射到地理坐标（仿射变换）
		void pixelToWorld(double px, double py, double* outX, double* outY) const;

	private:
		double geoTransform[6];
		bool m_hasTransform{false};
};
#endif

