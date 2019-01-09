#include "BSPMap.h"
#include <sys/stat.h>
#include <cstdio>
#include <stdlib.h>

using std::cout;
using std::endl;

BSPMap* g_pBSP = new BSPMap;

BSPMap::~BSPMap()
{}

void BSPMap::unload()
{
    delete[]   m_data;
    
    *m_path    = NULL;
    *m_mapName = NULL;
    m_header   = NULL;
    m_plane    = NULL;
    m_node     = NULL;
    m_leaf     = NULL;
}

bool BSPMap::IsNull()
{
    if( m_data == NULL )
        return true;
    if( *m_path == NULL )
        return true;
    if( *m_mapName == NULL )
        return true;
    if( m_header == NULL )
        return true;
    if( m_plane == NULL )
        return true;
    if( m_node == NULL )
        return true;
    if( m_leaf == NULL )
        return true;
    
    return false;
}

int BSPMap::filesize(){
    struct stat st;
    stat("de_dust2.bsp", &st);
    return st.st_size;
}

bool BSPMap::load( const char* path, const char* mapName )
{
    //	strcpy( m_path, path );
    //	strcpy( m_mapName, mapName );
    //
    //	std::string fPath( m_path );
    //	fPath += "\\csgo\\maps\\";
    //	fPath += m_mapName;
    
    
    FILE * f = fopen("de_dust2.bsp","r");
    int size  = filesize();
    uint8_t * m_data;
    m_data = new uint8_t[size];
    fread(m_data, 1, size, f);
    
    
    
    m_header	= ( dheader_t* )m_data;
    
    m_node		= ( dnode_t*  )( m_data + m_header->lumps[ 5 ].fileofs );
    m_plane		= ( dplane_t* )( m_data + m_header->lumps[ 1 ].fileofs );
    m_leaf		= ( dleaf_t*  )( m_data + m_header->lumps[ 10 ].fileofs );
    
    return true;
}

void BSPMap::DisplayInfo()
{
    //	if( !g_pEngine->IsInGame() )
    //	{
    //		cout << "Please enter a game first!" << endl;
    //	}
    //	else
    //	{
    cout << "map version  : " << m_header->version << endl;
    cout << "map name     : " << m_mapName << endl;
    cout << "map Revision : " << m_header->mapRevision << endl;
    cout << endl << endl;
    //	}
}

void BSPMap::setPath( const char* path )
{
    strcpy( m_path, path );
}

int BSPMap::getVersion()
{
    return m_header->version;
}

int BSPMap::getRevision()
{
    return m_header->mapRevision;
}

const char* BSPMap::getPath()
{
    return m_path;
}

const char* BSPMap::getName()
{
    return m_mapName;
}

dnode_t* BSPMap::getNodeLump()
{
    return m_node;
}

dleaf_t* BSPMap::getLeafLump()
{
    return m_leaf;
}

dplane_t* BSPMap::getPlaneLump()
{
    return m_plane;
}

float BSPMap::Dot( const Vector3 &v1, Vector3 &v2 )
{
    return v1[ 0 ] * v2[ 0 ] + v1[ 1 ] * v2[ 1 ] + v1[ 2 ] * v2[ 2 ];
}

dleaf_t* BSPMap::GetLeafFromPoint( const Vector3 point )
{
    int nodenum = 0;
    dnode_t* node;
    dplane_t* plane;
    
    float d = 0.0f;
    
    while( nodenum >= 0 )
    {
        if( &m_node == NULL || &m_plane == NULL )
            return NULL;
        
        node = &m_node[ nodenum ];
        plane = &m_plane[ node->planenum ];
        d = Dot( point, plane->normal ) - plane->dist;
        if( d > 0 )
            nodenum = node->children[ 0 ];
        else
            nodenum = node->children[ 1 ];
    }
    
    return &m_leaf[ -nodenum - 1 ];
}

bool BSPMap::Visible( const Vector3 &vStart, const Vector3 &vEnd ) // added in const so it won't try to refresh the same value
{
    if( IsNull() )
        return false;
    
    Vector3 direction = vEnd - vStart;
    Vector3 point = vStart;
    
    int steps = (int)direction.Length();
    
    if( steps > 4000 )	// performence issue when checking long distances, 2000 too short
        return false;   // we'll assume we can't see someone at great lengths
    
    direction /= ( float )steps;
    
    dleaf_t* leaf = nullptr;
    
    while( steps )
    {
        point += direction;
        leaf = GetLeafFromPoint( point );
        // Tried differenent masks, none seem to work :/ becauce ur a bitch tbh tbf fam
        if( leaf->contents & CONTENTS_SOLID )	
            return false;
        
        --steps;
    }
    return true;
}