//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsDecal.h"
#include "Private/RTTI/BsDecalRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Renderer/BsRenderer.h"
#include "Material/BsMaterial.h"
#include "CoreThread/BsCoreObjectSync.h"

namespace bs
{
	DecalBase::DecalBase()
	{
		updateBounds();
	}

	DecalBase::DecalBase(const Vector2& size, float maxDistance)
		: mSize(size), mMaxDistance(maxDistance)
	{
		updateBounds();
	}

	void DecalBase::updateBounds()
	{
		AABox localBounds(
			Vector3(-mSize.x, -mSize.y, 0.0f),
			Vector3(mSize.x, mSize.y, mMaxDistance)
		);

		localBounds.transformAffine(mTransform.getMatrix());

		mBounds = localBounds;
	}

	template <bool Core>
	template <class P>
	void TDecal<Core>::rttiEnumFields(P p)
	{
		p(mSize);
		p(mMaxDistance);
		p(mMaterial);
		p(mBounds);
	}

	Decal::Decal(const HMaterial& material, const Vector2& size, float maxDistance)
		:TDecal(material, size, maxDistance)
	{
		// Calling virtual method is okay here because this is the most derived type
		updateBounds();
	}

	SPtr<ct::Decal> Decal::getCore() const
	{
		return std::static_pointer_cast<ct::Decal>(mCoreSpecific);
	}

	SPtr<Decal> Decal::create(const HMaterial& material, const Vector2& size, float maxDistance)
	{
		Decal* decal = new (bs_alloc<Decal>()) Decal(material, size, maxDistance);
		SPtr<Decal> decalPtr = bs_core_ptr<Decal>(decal);
		decalPtr->_setThisPtr(decalPtr);
		decalPtr->initialize();

		return decalPtr;
	}

	SPtr<Decal> Decal::createEmpty()
	{
		Decal* decal = new (bs_alloc<Decal>()) Decal();
		SPtr<Decal> decalPtr = bs_core_ptr<Decal>(decal);
		decalPtr->_setThisPtr(decalPtr);

		return decalPtr;
	}

	SPtr<ct::CoreObject> Decal::createCore() const
	{
		SPtr<ct::Material> material;
		if(mMaterial.isLoaded(false))
			material = mMaterial->getCore();

		ct::Decal* decal = new (bs_alloc<ct::Decal>()) ct::Decal(material, mSize, mMaxDistance);
		SPtr<ct::Decal> decalPtr = bs_shared_ptr<ct::Decal>(decal);
		decalPtr->_setThisPtr(decalPtr);

		return decalPtr;
	}

	CoreSyncData Decal::syncToCore(FrameAlloc* allocator)
	{
		UINT32 size = 0;
		size += rttiGetElemSize(getCoreDirtyFlags());
		size += coreSyncGetElemSize((SceneActor&)*this);
		size += coreSyncGetElemSize(*this);

		UINT8* buffer = allocator->alloc(size);

		char* dataPtr = (char*)buffer;
		dataPtr = rttiWriteElem(getCoreDirtyFlags(), dataPtr);
		dataPtr = coreSyncWriteElem((SceneActor&)*this, dataPtr);
		dataPtr = coreSyncWriteElem(*this, dataPtr);

		return CoreSyncData(buffer, size);
	}

	void Decal::_markCoreDirty(ActorDirtyFlag flags)
	{
		markCoreDirty((UINT32)flags);
	}

	RTTITypeBase* Decal::getRTTIStatic()
	{
		return DecalRTTI::instance();
	}

	RTTITypeBase* Decal::getRTTI() const
	{
		return Decal::getRTTIStatic();
	}

	template class TDecal<true>;
	template class TDecal<false>;

	namespace ct
	{
	Decal::Decal(const SPtr<Material>& material, const Vector2& size, float maxDistance)
		: TDecal(material, size, maxDistance)
	{ }

	Decal::~Decal()
	{
		gRenderer()->notifyDecalRemoved(this);
	}

	void Decal::initialize()
	{
		updateBounds();
		gRenderer()->notifyDecalAdded(this);

		CoreObject::initialize();
	}

	void Decal::syncToCore(const CoreSyncData& data)
	{
		char* dataPtr = (char*)data.getBuffer();

		UINT32 dirtyFlags = 0;
		bool oldIsActive = mActive;

		dataPtr = rttiReadElem(dirtyFlags, dataPtr);
		dataPtr = coreSyncReadElem((SceneActor&)*this, dataPtr);
		dataPtr = coreSyncReadElem(*this, dataPtr);

		updateBounds();

		if (dirtyFlags == (UINT32)ActorDirtyFlag::Transform)
		{
			if (mActive)
				gRenderer()->notifyDecalUpdated(this);
		}
		else
		{
			if (oldIsActive != mActive)
			{
				if (mActive)
					gRenderer()->notifyDecalAdded(this);
				else
					gRenderer()->notifyDecalRemoved(this);
			}
			else
			{
				gRenderer()->notifyDecalRemoved(this);
				gRenderer()->notifyDecalAdded(this);
			}
		}
	}
}}
