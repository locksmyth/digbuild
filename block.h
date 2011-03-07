#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>
#include <assert.h>

#include <limits>
#include <vector>

#include "math.h"

enum BlockMaterial
{
    BLOCK_MATERIAL_NONE = 0xff,
    BLOCK_MATERIAL_GRASS = 0,
    BLOCK_MATERIAL_DIRT,
    BLOCK_MATERIAL_CLAY,
    BLOCK_MATERIAL_STONE,
    BLOCK_MATERIAL_BEDROCK,
    BLOCK_MATERIAL_MAGMA,
    BLOCK_MATERIAL_SIZE
};

struct Block
{
    static const Vector3i MAX_BRIGHTNESS;

    Block() :
        material_( BLOCK_MATERIAL_NONE ),
        sunlight_source_( 0 ),
        visited_( 0 ),
        light_level_r_( 0 ),
        light_level_g_( 0 ),
        light_level_b_( 0 )
    {
    }

    void set_material( const BlockMaterial material )
    {
        assert( material >= std::numeric_limits<uint8_t>::min() );
        assert( material <= std::numeric_limits<uint8_t>::max() );

        material_ = material;
    }

    BlockMaterial get_material() const { return BlockMaterial( material_ ); }

    void set_sunlight_source( const bool sunlight_source ) { sunlight_source_ = sunlight_source; }
    bool is_sunlight_source() const { return sunlight_source_; }

    void set_visited( const bool visited ) { visited_ = visited; }
    bool is_visited() const { return visited_; }

    void set_light_level( const Vector3i& light_level )
    {
        assert( light_level[0] <= MAX_BRIGHTNESS[0] );
        assert( light_level[1] <= MAX_BRIGHTNESS[1] );
        assert( light_level[2] <= MAX_BRIGHTNESS[2] );

        light_level_r_ = light_level[0];
        light_level_g_ = light_level[1];
        light_level_b_ = light_level[2];
    }

    Vector3i get_light_level() const
    {
        return Vector3i( light_level_r_, light_level_g_, light_level_b_ );
    }

private:

    uint8_t material_;

    uint8_t sunlight_source_ : 1;
    uint8_t visited_         : 1;
    uint8_t light_level_r_   : 4;
    uint8_t light_level_g_   : 4;
    uint8_t light_level_b_   : 4;
};

struct BlockFace
{
    enum { NUM_VERTICES = 4 };

    struct Vertex
    {
        Vertex()
        {
        }

        Vertex( const Vector3f& position, const Vector3f& lighting ) :
            position_( position ),
            lighting_( lighting )
        {
        }

        Vector3f position_;
        Vector3f lighting_;
    };

    BlockFace()
    {
    }

    BlockFace( const Vector3f& normal, const BlockMaterial material ) :
        normal_( normal ),
        material_( material )
    {
    }

    Vertex vertices_[NUM_VERTICES];

    Vector3f normal_;

    BlockMaterial material_;
};

typedef std::vector<BlockFace> BlockFaceV;

#endif // BLOCK_H
