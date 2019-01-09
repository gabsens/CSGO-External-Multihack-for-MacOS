#include <cmath>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#define HEADER_LUMPS 64




class Vector3
{
public:
    float x, y, z;
    
    Vector3()
    {
        x = y = z = 0.0f;
    }
    
    Vector3( float X, float Y, float Z )
    {
        x = X; y = Y; z = Z;
    }
    
    Vector3( float XYZ )
    {
        x = XYZ; y = XYZ; z = XYZ;
    }
    
    Vector3( float* v )
    {
        x = v[ 0 ]; y = v[ 1 ]; z = v[ 2 ];
    }
    
    Vector3( const float* v )
    {
        x = v[ 0 ]; y = v[ 1 ]; z = v[ 2 ];
    }
    
    inline Vector3& operator=( const Vector3& v )
    {
        x = v.x; y = v.y; z = v.z; return *this;
    }
    
    inline Vector3& operator=( const float* v )
    {
        x = v[ 0 ]; y = v[ 1 ]; z = v[ 2 ]; return *this;
    }
    
    inline float& operator[]( int i )
    {
        return ( ( float* )this )[ i ];
    }
    
    inline float operator[]( int i ) const
    {
        return ( ( float* )this )[ i ];
    }
    
    inline Vector3& operator+=( const Vector3& v )
    {
        x += v.x; y += v.y; z += v.z; return *this;
    }
    
    inline Vector3& operator-=( const Vector3& v )
    {
        x -= v.x; y -= v.y; z -= v.z; return *this;
    }
    
    inline Vector3& operator*=( const Vector3& v )
    {
        x *= v.x; y *= v.y; z *= v.z; return *this;
    }
    
    inline Vector3& operator/=( const Vector3& v )
    {
        x /= v.x; y /= v.y; z /= v.z; return *this;
    }
    
    inline Vector3& operator+=( float v )
    {
        x += v; y += v; z += v; return *this;
    }
    
    inline Vector3& operator-=( float v )
    {
        x -= v; y -= v; z -= v; return *this;
    }
    
    inline Vector3& operator*=( float v )
    {
        x *= v; y *= v; z *= v; return *this;
    }
    
    inline Vector3& operator/=( float v )
    {
        x /= v; y /= v; z /= v; return *this;
    }
    
    inline Vector3 operator-( ) const
    {
        return Vector3( -x, -y, -z );
    }
    
    inline Vector3 operator+( const Vector3& v ) const
    {
        return Vector3( x + v.x, y + v.y, z + v.z );
    }
    
    inline Vector3 operator-( const Vector3& v ) const
    {
        return Vector3( x - v.x, y - v.y, z - v.z );
    }
    
    inline Vector3 operator*( const Vector3& v ) const
    {
        return Vector3( x * v.x, y * v.y, z * v.z );
    }
    
    inline Vector3 operator/( const Vector3& v ) const
    {
        return Vector3( x / v.x, y / v.y, z / v.z );
    }
    
    inline Vector3 operator+( float v ) const
    {
        return Vector3( x + v, y + v, z + v );
    }
    
    inline Vector3 operator-( float v ) const
    {
        return Vector3( x - v, y - v, z - v );
    }
    
    inline Vector3 operator*( float v ) const
    {
        return Vector3( x * v, y * v, z * v );
    }
    
    inline Vector3 operator/( float v ) const
    {
        return Vector3( x / v, y / v, z / v );
    }
    
    inline float Length() const
    {
        return sqrtf( x * x + y * y + z * z );
    }
    
    inline float LengthSqr() const
    {
        return ( x * x + y * y + z * z );
    }
    
    inline float LengthXY() const
    {
        return sqrtf( x * x + y * y );
    }
    
    inline float LengthXZ() const
    {
        return sqrtf( x * x + z * z );
    }
    
    inline float DistTo( const Vector3& v ) const
    {
        return ( *this - v ).Length();
    }
    
    inline float Dot( const Vector3& v ) const
    {
        return ( x * v.x + y * v.y + z * v.z );
    }
    
    inline Vector3 Cross( const Vector3& v ) const
    {
        return Vector3( y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x );
    }
    
    inline bool IsZero() const
    {
        return ( x > -0.01f && x < 0.01f
                &&	y > -0.01f && y < 0.01f
                &&	z > -0.01f && z < 0.01f );
    }
};


enum Contents
{
	CONTENTS_EMPTY					= 0,				//  No contents
	CONTENTS_SOLID					= 0x1,				//	an eye is never valid in a solid
	CONTENTS_WINDOW					= 0x2,				//	translucent, but not watery( glass )
	CONTENTS_AUX					= 0x4,				//
	CONTENTS_GRATE					= 0x8,				//	alpha - tested "grate" textures.Bullets / sight pass through, but solids don't
	CONTENTS_SLIME					= 0x10,				//
	CONTENTS_WATER					= 0x20,				//
	CONTENTS_MIST					= 0x40,				//
	CONTENTS_OPAQUE					= 0x80,				//	block AI line of sight
	CONTENTS_TESTFOGVOLUME			= 0x100,			//	things that cannot be seen through( may be non - solid though )
	CONTENTS_UNUSED					= 0x200,			//	unused
	CONTENTS_UNUSED6				= 0x400,			//  unused
	CONTENTS_TEAM1					= 0x800,			//  per team contents used to differentiate collisions between players and objects on different teams
	CONTENTS_TEAM2					= 0x1000,			//
	CONTENTS_IGNORE_NODRAW_OPAQUE	= 0x2000,			//	ignore CONTENTS_OPAQUE on surfaces that have SURF_NODRAW
	CONTENTS_MOVEABLE				= 0x4000,			//	hits entities which are MOVETYPE_PUSH( doors, plats, etc. )
	CONTENTS_AREAPORTAL				= 0x8000,			//	remaining contents are non - visible, and don't eat brushes
	CONTENTS_PLAYERCLIP				= 0x10000,			//
	CONTENTS_MONSTERCLIP			= 0x20000,			//
	CONTENTS_CURRENT_0				= 0x40000,			//	currents can be added to any other contents, and may be mixed
	CONTENTS_CURRENT_90				= 0x80000,			//
	CONTENTS_CURRENT_180			= 0x100000,			//
	CONTENTS_CURRENT_270			= 0x200000,			//
	CONTENTS_CURRENT_UP				= 0x400000,			//
	CONTENTS_CURRENT_DOWN			= 0x800000,			//
	CONTENTS_ORIGIN					= 0x1000000,		//	removed before bsping an entity
	CONTENTS_MONSTER				= 0x2000000,		//	should never be on a brush, only in game
	CONTENTS_DEBRIS					= 0x4000000,		//
	CONTENTS_DETAIL					= 0x8000000,		//	brushes to be added after vis leafs
	CONTENTS_TRANSLUCENT			= 0x10000000,		//	auto set if any surface has trans
	CONTENTS_LADDER					= 0x20000000,		//
	CONTENTS_HITBOX					= 0x40000000		//	use accurate hitboxes on trace
};

struct dleaf_t
{
	int					contents;			// OR of all brushes (not needed?)
	short				cluster;			// cluster this leaf is in
	short				area : 9;			// area this leaf is in
	short				flags : 7;			// flags
	short				mins[ 3 ];			// for frustum culling
	short				maxs[ 3 ];
	unsigned short		firstleafface;		// index into leaffaces
	unsigned short		numleaffaces;
	unsigned short		firstleafbrush;		// index into leafbrushes
	unsigned short		numleafbrushes;
	short				leafWaterDataID;	// -1 for not in water

	//!!! NOTE: for maps of version 19 or lower uncomment this block
	/*
	CompressedLightCube	ambientLighting;	// Precaculated light info for entities.
	short			padding;		// padding to 4-byte boundary
	*/
};

struct lump_t
{
	int	 fileofs;	// offset into file (bytes)
	int	 filelen;	// length of lump (bytes)
	int	 version;	// lump format version
	char fourCC[ 4 ];	// lump ident code
};

struct dheader_t
{
	int		ident;                // BSP file identifier
	int		version;              // BSP file version
	lump_t	lumps[ HEADER_LUMPS ];// lump directory array
	int		mapRevision;          // the map's revision (iteration, version) number
};

struct dnode_t
{
	int				planenum;	// index into plane array
	int		children[ 2 ];		// negative numbers are -(leafs + 1), not nodes
	short		mins[ 3 ];		// for frustum culling
	short		maxs[ 3 ];
	unsigned short	firstface;	// index into face array
	unsigned short	numfaces;	// counting both sides
	short			area;		// If all leaves below this node are in the same area, then
								// this is the area index. If not, this is -1.
	short			padding;	// pad to 32 bytes length
};

struct dplane_t
{
	Vector3	normal;	// normal vector
	float	dist;	// distance from origin
	int		type;	// plane axis identifier
};

class BSPMap
{
public:

	BSPMap()
	{}

	~BSPMap();
	bool load( const char* path, const char* mapName );
	void unload();
	bool IsNull();
	void setPath( const char* path );
	void DisplayInfo();
	int getVersion();
	int getRevision();
    int filesize();
    float Dot(const Vector3 &v1, Vector3 &v2);
	const char* getPath();
	const char* getName();
	dnode_t* getNodeLump();
	dplane_t* getPlaneLump();
	dleaf_t* getLeafLump();
	dleaf_t* GetLeafFromPoint( const Vector3 point );
	bool Visible( const Vector3 &vStart, const Vector3 &vEnd );

private:
	char m_path[ 255 ];
	char m_mapName[ 128 ];
	uint64_t* m_data;
	dheader_t* m_header;
	dnode_t* m_node;
	dplane_t* m_plane;
	dleaf_t* m_leaf;
};



extern BSPMap* g_pBSP;

