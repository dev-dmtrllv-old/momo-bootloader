#include "core/vesa.hpp"
#include "core/types.hpp"
#include "core/mm.hpp"

namespace Vesa
{
	struct VbeInfoBlock
	{
		char vbeSignature[4];	  // == "VESA"
		uint16_t vbeVersion;	  // == 0x0300 for VBE 3.0
		uint16_t oemStringPtr[2]; // isa vbeFarPtr
		uint8_t capabilities[4];
		uint16_t videoModePtr[2]; // isa vbeFarPtr
		uint16_t totalMemory;	  // as # of 64KB blocks
	} __attribute__((packed));

	namespace
	{
		bool isInitialized_ = false;
		VbeInfoBlock *vib = nullptr;
	};

	void init()
	{
		if(!isInitialized_)
		{
			vib = MM::alloc<VbeInfoBlock>(512);
			isInitialized_ = true;
		}
	}

	uint16_t findClosestMode()
	{
		return 0;
	}
};
