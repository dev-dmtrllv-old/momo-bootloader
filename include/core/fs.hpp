#pragma once

#include "core/types.hpp"
#include "core/macros.hpp"

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
		private:
		const char* name_;

	public:
		Driver(const char* name) : lba(0), name_(name) {};

		const char* name() { return name_; };

		void initDriver(uint32_t lba) { this->lba = lba; this->init(lba); };

		virtual void init(uint32_t UNUSED lba) {  };
		virtual bool canDrive(uint32_t lba) = 0;
		virtual bool getPathInfo(PathInfo* info, const char* path) = 0;
		virtual bool readFile(const char* path, void* dest) = 0;
		virtual void readDir(const char* path, void* dest, size_t maxItems) = 0;
		virtual void readDir(const char* path, ReadDirCallback callback) = 0;

	protected:
		uint32_t lba;
	};


	typedef void(*RegisterDriver)(const FS::Driver* driver);
	typedef void(*UnregisterDriver)(const FS::Driver* driver);

	void init();
	void loadDriver(size_t partitionIndex, FS::Driver* driver);

	bool getPathInfo(FS::PathInfo* pathInfo, const char* str);
	bool readFile(const char* path, void* file);
	void readDir(const char* path, ReadDirCallback callback);
	
	void registerDriver(const FS::Driver* driver);
	void unregisterDriver(const FS::Driver* driver);
};
