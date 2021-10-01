#pragma once

#include "core/types.hpp"

namespace FS
{
	struct PathInfo
	{
		bool isDirectory;
		uint32_t size;
	};

	typedef void(*ReadDirCallback)(const char* name, const PathInfo& pathInfo);

	class Driver
	{
	public:
		Driver() : lba(0) {};

		void initDriver(uint32_t lba) { this->lba = lba; this->init(lba); };

		virtual void init(uint32_t lba) {  };
		virtual bool getPathInfo(PathInfo* info, const char* path) = 0;
		virtual void readFile(const char* path, void* dest) = 0;
		virtual void readDir(const char* path, void* dest, size_t maxItems) = 0;
		virtual void readDir(const char* path, ReadDirCallback callback) = 0;

	protected:
		uint32_t lba;
	};

	void init();
	void loadDriver(size_t partitionIndex, FS::Driver* driver, size_t driverSize);

	bool getPathInfo(FS::PathInfo* pathInfo, const char* str);
	void readFile(const char* path, void* file);
	void readDir(const char* path, ReadDirCallback callback);
};
