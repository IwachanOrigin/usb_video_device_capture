
#include "mesh.h"
#include "dx11base.h"
#include "dxhelper.h"

using namespace Render;
using namespace dx_engine;

Mesh::Mesh()
  : m_vb(nullptr)
  , m_vertices(0)
  , m_bytesPerVertex(0)
  , m_topology(eTopology::UNDEFINED)
{
}

Mesh::~Mesh()
{
}

bool Mesh::create(const void* vertices, uint32_t new_nvertices, uint32_t new_bytes_per_vertex, eTopology new_topology)
{
  m_vertices = new_nvertices;
  m_topology = new_topology;
  m_bytesPerVertex = new_bytes_per_vertex;

  D3D11_BUFFER_DESC bufferDesc = {};
  bufferDesc.Usage = D3D11_USAGE_DEFAULT;
  bufferDesc.ByteWidth = m_vertices * m_bytesPerVertex;
  bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bufferDesc.CPUAccessFlags = 0;

  D3D11_SUBRESOURCE_DATA subresourceData = {};
  subresourceData.pSysMem = vertices;
  HRESULT hr = DX11Base::getInstance().getDevice()->CreateBuffer(&bufferDesc, &subresourceData, m_vb.GetAddressOf());
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
  DX11Base::getInstance().getDeviceContext()->IASetVertexBuffers(0, 1, m_vb.GetAddressOf(), &stride, &offset);
  DX11Base::getInstance().getDeviceContext()->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)m_topology);
}

void Mesh::render() const
{
  DX11Base::getInstance().getDeviceContext()->Draw(m_vertices, 0);
}

void Mesh::activateAndRender() const
{
  this->activate();
  this->render();
}

void Mesh::destroy()
{
  SAFE_RELEASE(m_vb);
}

