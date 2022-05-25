#pragma once
#include <DirectXMath.h>
#include "mesh.h"
#include <functional>
#include <stack>
#include <iterator>

namespace mini
{
	struct ModelNode
	{
		DirectX::XMFLOAT4X4 localTransform{ 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
		int nextIndex = -1;
		int childIndex = -1;
		int meshIndex = -1;
	};

	class Model
	{
	public:
		class NodeIterator
		{
		public:
			using value_type = const std::pair<const Mesh&, const DirectX::XMFLOAT4X4&>;
			using reference = value_type;
			using difference_type = int;
			using pointer = int;
			using iterator_category = std::forward_iterator_tag;

			NodeIterator();
			explicit NodeIterator(const Model& m);
			NodeIterator(const NodeIterator& other) = default;
			NodeIterator(NodeIterator&& other) = default;

			NodeIterator& operator=(NodeIterator&& other) = default;
			NodeIterator& operator=(const NodeIterator& other) = default;

			bool operator==(const NodeIterator& other) const;
			bool operator!=(const NodeIterator& other) const;
			NodeIterator& operator++();
			NodeIterator operator++(int);

			std::pair<const Mesh&, const DirectX::XMFLOAT4X4&> operator*() const;

			int nodeIndex() const { return m_nodeStack.empty() ? -1 : m_nodeStack.top().first; }

			const ModelNode& node() const { assert(!m_nodeStack.empty()); return m_model->getNode(nodeIndex()); }

			int meshIndex() const { return m_nodeStack.empty() ? -1 : node().meshIndex; }

			const Mesh& mesh() const { assert(!m_nodeStack.empty()); return m_model->getMesh(meshIndex()); }

			size_t meshSignatureID() const { assert(!m_nodeStack.empty()); return m_model->getMeshSignatureID(meshIndex()); }

			const DirectX::XMFLOAT4X4& transform() const { assert(!m_nodeStack.empty()); return m_nodeStack.top().second; }

		private:

			void pushNode(int nodeIndex);
			void pushToLeaf(int nodeIndex);

			const Model* m_model;
			std::stack<std::pair<int, DirectX::XMFLOAT4X4>> m_nodeStack;
		};
		Model() = default;
		Model(std::vector<Mesh>&& meshes, std::vector<size_t>&& signatures, std::vector<ModelNode>&& nodes);
		Model(Model&& other) = default;
		Model(const Model& other) = delete;

		Model& operator=(Model&& other) = default;
		Model& operator=(const Model& other) = delete;

		int addMesh(Mesh&& m, size_t signatureID);
		int addNode(const ModelNode& node, int parentNodeIndex = -1);

		Mesh& getMesh(int meshIndex);
		const Mesh& getMesh(int meshIndex) const;

		size_t getMeshSignatureID(int meshIndex) const;

		const ModelNode& getNode(int nodeIndex) const;
		void setNodeTransform(int nodeIndex, const DirectX::XMFLOAT4X4& transform);
		//applies transformation to the whole model
		void applyTransform(const DirectX::XMFLOAT4X4& transform);

		NodeIterator begin() const;
		NodeIterator end() const;

		bool empty() const { return m_nodes.empty(); }

	private:
		std::vector<Mesh> m_meshes;
		std::vector<size_t> m_meshSignatures;
		std::vector<ModelNode> m_nodes;
	};
}
