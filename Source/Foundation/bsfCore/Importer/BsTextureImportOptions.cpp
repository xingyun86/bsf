//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Importer/BsTextureImportOptions.h"
#include "Private/RTTI/BsTextureImportOptionsRTTI.h"

namespace bs
{
	TextureImportOptions::TextureImportOptions()
		: mFormat(PF_RGBA8), mGenerateMips(false), mMaxMip(0), mCPUCached(false), mSRGB(false), mCubemap(false)
		, mCubemapSourceType(CubemapSourceType::Faces)
	{ }

	SPtr<TextureImportOptions> TextureImportOptions::create()
	{
		return bs_shared_ptr_new<TextureImportOptions>();
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/
	RTTITypeBase* TextureImportOptions::getRTTIStatic()
	{
		return TextureImportOptionsRTTI::instance();
	}

	RTTITypeBase* TextureImportOptions::getRTTI() const
	{
		return TextureImportOptions::getRTTIStatic();
	}
}