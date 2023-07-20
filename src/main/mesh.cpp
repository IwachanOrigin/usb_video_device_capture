
#include "stdafx.h"
#include "mesh.h"

using namespace renderer;

Mesh::Mesh()
  : m_vb(nullptr)
  , m_vertices(0)
  , m_bytesPerVertex(0)
  , m_topology(eTopology::UNDEFINED)
  , m_d3dDevice(nullptr)
  , m_immediateContext(nullptr)
{
}

Mesh::~Mesh()
{
}

bool Mesh::create(const void* vertices, uint32_t new_nvertices, uint32_t new_bytes_per_vertex, eTopology new_topology, ComPtr<ID3D11Device> d3dDevice, ComPtr<ID3D11DeviceContext> immediateContext)
{
  m_vertices = new_nvertices;
  m_topology = new_topology;
  m_bytesPerVertex = new_bytes_per_vertex;
  m_d3dDevice = d3dDevice;
  m_immediateContext = immediateContext;

  D3D11_BUFFER_DESC bufferDesc = {};
  bufferDesc.Usage = D3D11_USAGE_DEFAULT;
  bufferDesc.ByteWidth = m_vertices * m_bytesPerVertex;
  bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bufferDesc.CPUAccessFlags = 0;

  D3D11_SUBRESOURCE_DATA subresourceData = {};
  subresourceData.pSysMem = vertices;
  HRESULT hr = m_d3dDevice->CreateBuffer(&bufferDesc, &subresourceData, m_vb.GetAddressOf());
  if (FAILED(hr))
  {
    return false;
  }

  return true;
}

void Mesh::activate() const
{
  // Set vertex buffer
  UINT stride = m_bytesPerVertex;
  UINT offset = 0;
  m_immediateContext->IASetVertexBuffers(0, 1, m_vb.GetAddressOf(), &stride, &offset);
  m_immediateContext->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)m_topology);
}

void Mesh::render() const
{
  m_immediateContext->Draw(m_vertices, 0);
}

void Mesh::activateAndRender() const
{
  this->activate();
  this->render();
}

void Mesh::destroy()
{
  if (m_vb)
  {
    m_vb->Release();
    m_vb = nullptr;
  }
}

