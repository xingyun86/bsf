//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "CoreThread/BsCoreObject.h"
#include "Math/BsVector2.h"
#include "Math/BsAABox.h"
#include "Scene/BsSceneActor.h"

namespace bs
{
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/** @} */

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Base class for both core and sim thread implementations of Decal. */
	class BS_CORE_EXPORT DecalBase : public SceneActor
	{
	public:
		DecalBase();
		DecalBase(const Vector2& size, float maxDistance);
		virtual ~DecalBase() = default;

		/** Size of the decal in world space units. */
		void setSize(const Vector2& size) { mSize = size; _markCoreDirty(); updateBounds(); }

		/** @copydoc setSize */
		Vector2 getSize() const
		{
			return Vector2(mSize.x * mTransform.getScale().x, mSize.y * mTransform.getScale().y);
		}

		/** Determines the maximum distance (from its origin) at which the decal is displayed. */
		void setMaxDistance(float distance) { mMaxDistance = distance; _markCoreDirty(); updateBounds(); }

		/** @copydoc getSize */
		float getMaxDistance() const { return mMaxDistance * mTransform.getScale().z; }

	protected:
		/** Updates the internal bounds for the decal. Call this whenever a property affecting the bounds changes. */
		void updateBounds();

		Vector2 mSize = Vector2::ONE;
		float mMaxDistance = 10.0f;

		AABox mBounds;
	};

	/** Templated base class for both core and sim thread implementations of Decal. */
	template<bool Core>
	class BS_CORE_EXPORT TDecal : public DecalBase
	{
	public:
		using MaterialType = CoreVariantHandleType<Material, Core>;

		TDecal() = default;
		TDecal(const MaterialType& material, const Vector2& size, float maxDistance)
			:DecalBase(size, maxDistance), mMaterial(material)
		{ }
		virtual ~TDecal() = default;

		/** Determines the material to use when rendering the decal. */
		void setMaterial(const MaterialType& material) { mMaterial = material; _markCoreDirty(); }

		/** @copydoc setMaterial */
		const MaterialType& getMaterial() const { return mMaterial; }

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P p);

	protected:
		MaterialType mMaterial;
	};

	/** @} */
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	namespace ct { class Decal; }

	/** 
	 * Specifies a decal that will be projected onto scene geometry. User can set the material to use when rendering
	 * the decal, as well as control decal orientation and size.
	 */
	class BS_CORE_EXPORT Decal : public IReflectable, public CoreObject, public TDecal<false>
	{
	public:
		/**	Retrieves an implementation of the decal usable only from the core thread. */
		SPtr<ct::Decal> getCore() const;

		/**
		 * Creates a new decal.
		 *
		 * @param[in]	material		Material to use when rendering the decal.
		 * @param[in]	size			Size of the decal in world units.
		 * @param[in]	maxDistance		Maximum distance at which will the decal be visible (from the current decal origin,
		 *								along the negative Z axis).
		 * @returns						New decal object.
		 */
		static SPtr<Decal> create(const HMaterial& material, const Vector2& size = Vector2::ONE, float maxDistance = 10.0f);

	protected:
		Decal(const HMaterial& material, const Vector2& size, float maxDistance);

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		/** @copydoc DecalBase::_markCoreDirty */
		void _markCoreDirty(ActorDirtyFlag flags = ActorDirtyFlag::Everything) override;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData syncToCore(FrameAlloc* allocator) override;

		/**	Creates the object with without initializing it. Used for serialization. */
		static SPtr<Decal> createEmpty();

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class DecalRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		Decal() = default; // Serialization only
	};

	namespace ct
	{
	/** Core thread version of a bs::Decal */
	class BS_CORE_EXPORT Decal : public CoreObject, public TDecal<true>
	{
	public:
		~Decal();

		/**	Sets an ID that can be used for uniquely identifying this object by the renderer. */
		void setRendererId(UINT32 id) { mRendererId = id; }

		/**	Retrieves an ID that can be used for uniquely identifying this object by the renderer. */
		UINT32 getRendererId() const { return mRendererId; }

	protected:
		friend class bs::Decal;

		Decal(const SPtr<Material>& material, const Vector2& size, float maxDistance);

		/** @copydoc CoreObject::initialize */
		void initialize() override;

		/** @copydoc CoreObject::syncToCore */
		void syncToCore(const CoreSyncData& data) override;

		UINT32 mRendererId = 0;
	};
	}

	/** @} */
}
