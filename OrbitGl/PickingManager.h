//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "CoreMath.h"
#include <vector>
#include <unordered_map>

class GlCanvas;

//-----------------------------------------------------------------------------
class Pickable
{
public:
    virtual void OnPick( int a_X, int a_Y ) = 0;
    virtual void OnDrag( int a_X, int a_Y ){}
    virtual void OnRelease(){};
    virtual void Draw( GlCanvas* a_Canvas, bool a_Picking ) = 0;
    virtual bool Draggable() { return false; }
};

//-----------------------------------------------------------------------------
struct PickingID
{
    enum Type{ INVALID
             , LINE
             , EVENT
             , BOX
             , PICKABLE };

    static PickingID Get( Type a_Type, unsigned a_ID )
    {
        static_assert(sizeof(PickingID) == 4, "PickingID must be 32 bits");
        PickingID id;
        id.m_Type = a_Type;
        id.m_Id = a_ID;
        return id;
    }
    static Color GetColor( Type a_Type, unsigned a_ID )
    {
        PickingID id = Get( a_Type, a_ID );
        return *(reinterpret_cast<Color*>(&id));
    }
    static PickingID Get( uint32_t a_Value ){ return *(reinterpret_cast<PickingID*>(&a_Value)); }
    uint32_t m_Id   : 29;
    uint32_t m_Type : 3;
    
};

//-----------------------------------------------------------------------------
class PickingManager
{
public:
    PickingManager() : m_IdCounter( 0 ), m_Picked(nullptr), m_IdStart(0) {}

    PickingID CreatePickableId( Pickable* a_Pickable );
    void ClearIds();

    void Pick( uint32_t a_Id, int a_X, int a_Y );
    void Release();
    void Drag( int a_X, int a_Y  );
    Pickable* GetPicked() { return m_Picked; }
    bool IsDragging() const { return m_Picked && m_Picked->Draggable(); }

    static void SetPickingColor( PickingID a_ID );

protected:
    std::vector< Pickable* > m_Pickables;
    uint32_t                 m_IdCounter;
    uint32_t                 m_IdStart;
    std::unordered_map< Pickable*, uint32_t > m_PickableIdMap;
    std::unordered_map< uint32_t, Pickable* > m_IdPickableMap;
    Pickable*                m_Picked;
};
