#ifndef _IMAGE_FILE_H
#define _IMAGE_FILE_H

#include "ImageProperties.h"
#include "CoordinateInfo.h"
#include "BandInfo.h"


class ImageFile
{
	public:
		ImageFile(const std::string& theFilename);
		int getifErr(void);
		virtual ~ImageFile(void);
		ImageProperties* getImageProperties(void);
		BandInfo* getBandInfo(int bandNumber);
		std::string getInfoString(void);
		void getRasterData(int width, int height, int xpos, int ypos, char* buffer, int outWidth, int outHeight);
		int getSampleSizeBytes() const; // 根据数据类型返回每像素样本字节数
		double getNoDataValue(int bandNumber, bool* hasNoData = nullptr) const; // 获取指定波段的 NoData 值
		CoordinateInfo* getCoordinateInfo() const { return coordInfo; }
		
	private:
		int ifErr;
		GDALDataset* ifDataset;
		std::string filename;
		std::string infoString;
		ImageProperties* properties;
		CoordinateInfo* coordInfo;
		std::vector<BandInfo*> theBands;
};

#endif
