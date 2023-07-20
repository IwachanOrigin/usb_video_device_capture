
#ifndef MESH_H_
#define MESH_H_

#include "stdafx.h"

using namespace Microsoft::WRL;

namespace renderer
{

enum class eTopology
{
  UNDEFINED = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
  TRIANGLE_LIST = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
  LINE_LIST = D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
};

class Mesh
{
public:
  explicit Mesh();
  ~Mesh();

  bool create(const void* vertices, uint32_t new_nvertices, uint32_t new_bytes_per_vertex, eTopology new_topology, ComPtr<ID3D11Device> d3dDevice, ComPtr<ID3D11DeviceContext> immediateContext);
  void activateAndRender() const;
  void destroy();

private:
  ComPtr<ID3D11Buffer> m_vb;
  ComPtr<ID3D11Device> m_d3dDevice;
  ComPtr<ID3D11DeviceContext> m_immediateContext;
  uint32_t m_vertices;
  uint32_t m_bytesPerVertex;
  eTopology m_topology;

  void activate() const;
  void render() const;
};

} // renderer

#endif // MESH_H_
