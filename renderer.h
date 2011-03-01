#ifndef RENDERER_H
#define RENDERER_H

#include <GL/gl.h>

#include <map>

#include <boost/utility.hpp>

#include "region.h"
#include "renderer_material.h"

struct BlockVertex
{
    BlockVertex()
    {
    }

    BlockVertex( const Vector3f& position, const Vector3f& normal, const Vector2f& texcoords ) :
        x_( position[0] ), y_( position[1] ), z_( position[2] ),
        nx_( normal[0] ), ny_( normal[1] ), nz_( normal[2] ),
        s_( texcoords[0] ), t_( texcoords[1] )
    {
    }

    GLfloat x_, y_, z_;
    GLfloat nx_, ny_, nz_;
    GLfloat s_, t_;

} __attribute__( ( packed ) );

typedef std::vector<BlockVertex> BlockVertexV;

struct ChunkVertexBuffer : public boost::noncopyable
{
    ChunkVertexBuffer();
    ChunkVertexBuffer( const BlockVertexV& vertices );
    ~ChunkVertexBuffer();

    void render();

private:

    void bind();

    GLuint
        vbo_id_,
        ibo_id_;

    GLsizei vertex_count_;
};

typedef boost::shared_ptr<ChunkVertexBuffer> ChunkVertexBufferSP;
typedef std::map<BlockMaterial, ChunkVertexBufferSP> ChunkVertexBufferMap;

struct ChunkRenderer
{
    ChunkRenderer();

    void render( const RendererMaterialV& materials );
    void initialize( const Chunk& chunk );
    bool initialized() const { return initialized_; }

private:

    bool initialized_;

    ChunkVertexBufferMap vbos_;
};

struct Renderer
{
    Renderer();

    void render( const RegionMap& regions );

protected:

    typedef std::map<Vector3i, ChunkRenderer, Vector3LexicographicLess<Vector3i> > ChunkRendererMap;
    ChunkRendererMap chunk_renderers_;

    RendererMaterialV materials_;
};

#endif // RENDERER_H
