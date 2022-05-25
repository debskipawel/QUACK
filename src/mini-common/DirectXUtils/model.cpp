#include "model.h"

using namespace std;
using namespace DirectX;
using namespace mini;

Model::NodeIterator::NodeIterator()
	: m_model(nullptr)
{
}

Model::NodeIterator::NodeIterator(const Model& m)
	: m_model(&m)
{
	if (m.m_nodes.size() != 0)
	{
		pushToLeaf(0);
		if (node().meshIndex == -1)
			++*this;
	}
}

bool Model::NodeIterator::operator==(const NodeIterator& other) const
{
	if (m_nodeStack.empty() && other.m_nodeStack.empty())
		return true;
	return m_model == other.m_model && m_nodeStack.top().first == other.m_nodeStack.top().first;
}

bool Model::NodeIterator::operator!=(const NodeIterator& other) const
{
	return !(*this == other);
}

Model::NodeIterator& Model::NodeIterator::operator++()
{
	if (m_nodeStack.empty())
		return *this;
	do
	{
		int currentNode = node().nextIndex;
		m_nodeStack.pop();
		if (currentNode != -1)
			pushToLeaf(currentNode);
	} while (!m_nodeStack.empty() && node().meshIndex == -1);
	return *this;
}

Model::NodeIterator Model::NodeIterator::operator++(int)
{
	NodeIterator copy = *this;
	++*this;
	return copy;
}

pair<const Mesh&, const XMFLOAT4X4&> Model::NodeIterator::operator*() const
{
	return{ mesh(), transform() };
}

void Model::NodeIterator::pushNode(int nodeIndex)
{
	XMFLOAT4X4 nodeTransform = m_model->getNode(nodeIndex).localTransform;
	if (!m_nodeStack.empty())
		XMStoreFloat4x4(&nodeTransform, XMLoadFloat4x4(&nodeTransform)*XMLoadFloat4x4(&transform()));
	m_nodeStack.push(make_pair(nodeIndex, nodeTransform));
}

void Model::NodeIterator::pushToLeaf(int nodeIndex)
{
	while (nodeIndex != -1)
	{
		pushNode(nodeIndex);
		nodeIndex = node().childIndex;
	}
}

Model::Model(vector<Mesh>&& meshes, vector<size_t>&& signatures, vector<ModelNode>&& nodes)
	: m_meshes(move(meshes)), m_meshSignatures(move(signatures)), m_nodes(move(nodes))
{
	assert(m_meshes.size() == m_meshSignatures.size());
}

int Model::addMesh(Mesh&& m, size_t signatureID)
{
	m_meshes.push_back(move(m));
	m_meshSignatures.push_back(signatureID);
	assert(m_meshes.size() - 1 < INT_MAX);
	return static_cast<int>(m_meshes.size() - 1);
}

int Model::addNode(const ModelNode& node, int parentNodeIndex)
{
	assert(node.meshIndex == -1 || (node.meshIndex >= 0 &&  static_cast<size_t>(node.meshIndex) < m_meshes.size()));
	assert(parentNodeIndex == -1 || (parentNodeIndex >= 0 && static_cast<size_t>(parentNodeIndex) < m_nodes.size()));
	assert(m_nodes.size() <= INT_MAX);
	const int nodeIndex = static_cast<int>(m_nodes.size());
	m_nodes.push_back(node);
	ModelNode& newNode = m_nodes.back();
	newNode.childIndex = newNode.nextIndex = -1;
	if (nodeIndex != 0)
	{
		int* pIndex = (parentNodeIndex == -1) ? &m_nodes[0].nextIndex : &m_nodes[parentNodeIndex].childIndex;
		while (*pIndex != -1)
			pIndex = &m_nodes[*pIndex].nextIndex;
		*pIndex = nodeIndex;
	}
	return nodeIndex;
}

Mesh& Model::getMesh(int meshIndex)
{
	return const_cast<Mesh&>(static_cast<const Model&>(*this).getMesh(meshIndex));
}

const Mesh& Model::getMesh(int meshIndex) const
{
	assert(meshIndex >= 0 && static_cast<size_t>(meshIndex) < m_meshes.size());
	return m_meshes[meshIndex];
}

size_t Model::getMeshSignatureID(int meshIndex) const
{
	assert(meshIndex >= 0 && static_cast<size_t>(meshIndex) < m_meshSignatures.size());
	return m_meshSignatures[meshIndex];
}

const ModelNode& Model::getNode(int nodeIndex) const
{
	assert(nodeIndex >= 0 && static_cast<size_t>(nodeIndex) < m_nodes.size());
	return m_nodes[nodeIndex];
}

void Model::setNodeTransform(int nodeIndex, const XMFLOAT4X4& transform)
{
	assert(nodeIndex >= 0 && static_cast<size_t>(nodeIndex) < m_nodes.size());
	m_nodes[nodeIndex].localTransform = transform;
}

void Model::applyTransform(const DirectX::XMFLOAT4X4& transform)
{
	if (m_nodes.empty())
		return;
	int currentIndex = 0;
	while (currentIndex != -1)
	{
		ModelNode& currentNode = m_nodes[currentIndex];
		XMStoreFloat4x4(&currentNode.localTransform, XMLoadFloat4x4(&currentNode.localTransform) * XMLoadFloat4x4(&transform));
		currentIndex = currentNode.nextIndex;
	}
}

Model::NodeIterator Model::begin() const
{
	return NodeIterator(*this);
}

Model::NodeIterator Model::end() const
{
	return NodeIterator();
}


