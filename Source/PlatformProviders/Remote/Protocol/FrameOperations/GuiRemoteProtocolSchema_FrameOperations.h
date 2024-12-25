/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

***********************************************************************/

#include "../Generated/GuiRemoteProtocolSchema.h"

namespace vl::presentation::remoteprotocol
{
	/*
	* dom id:
	* 
	* root:              -1
	* element:           (elementId << 2) + 0
	* parent of element: (elementId << 2) + 1
	* hittest:           (compositionId << 2) + 2
	* parent of hittest: (compositionId << 2) + 3
	*/

	class RenderingDomBuilder
	{
		using RenderingResultRef = Ptr<RenderingDom>;
		using RenderingResultRefList = collections::List<RenderingResultRef>;
	protected:
		RenderingResultRefList		domStack;
		collections::List<vint>		domBoundaries;
		Ptr<RenderingDom>			domRoot;
		Ptr<RenderingDom>			domCurrent;

		vint						GetCurrentBoundary();
		vint						Push(RenderingResultRef ref);
		void						PopTo(vint index);
		void						Pop();
		void						PopBoundary();

		template<typename TCallback>
		void						PrepareParentFromCommand(Rect commandBounds, Rect commandValidArea, vint newDomId, TCallback&& calculateValidAreaFromDom);
	public:
		RenderingDomBuilder() = default;
		~RenderingDomBuilder() = default;

		void						RequestRendererBeginRendering();
		void						RequestRendererBeginBoundary(const remoteprotocol::ElementBoundary& arguments);
		void						RequestRendererEndBoundary();
		void						RequestRendererRenderElement(const remoteprotocol::ElementRendering& arguments);
		Ptr<RenderingDom>			RequestRendererEndRendering();
	};

	extern Ptr<RenderingDom>		CopyDom(Ptr<RenderingDom> root);

	struct DomIndexItem
	{
		vint				id;
		vint				parentId;
		Ptr<RenderingDom>	dom;

		auto operator<=>(const DomIndexItem&) const = default;
	};
	using DomIndex = collections::Array<DomIndexItem>;

	extern void						BuildDomIndex(Ptr<RenderingDom> root, DomIndex& index);
	extern void						UpdateDomInplace(Ptr<RenderingDom> root, DomIndex& index, const RenderingDom_DiffsInOrder& diffs);
	extern void						DiffDom(Ptr<RenderingDom> domFrom, DomIndex& indexFrom, Ptr<RenderingDom> domTo, DomIndex& indexTo, RenderingDom_DiffsInOrder& diffs);
}