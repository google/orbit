//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

//-----------------------------------------------------------------------------
template< class T, int BUFFER_SIZE >
class RingBuffer
{
public:
    //-------------------------------------------------------------------------
    RingBuffer() : m_CurrentSize(0), m_CurrentIndex(0) 
    {
    }
    
    //-------------------------------------------------------------------------
    ~RingBuffer()
    {
    }

    //-------------------------------------------------------------------------
    inline void Clear()
    {
        m_CurrentSize = m_CurrentIndex = 0;
    }
    
    //-------------------------------------------------------------------------
    inline void Add( const T& a_Item )
    {
        m_Data[m_CurrentIndex++] = a_Item;
        m_CurrentIndex %= BUFFER_SIZE;
        ++m_CurrentSize;
    }

    //-------------------------------------------------------------------------
    inline void Fill( const T& a_Item )
    {
        for( int i = 0; i < BUFFER_SIZE; ++i )
        {
            Add( a_Item );
        }
    }

    //-------------------------------------------------------------------------
    inline bool Contains( const T& a_Item ) const
    {
        for( int i = 0; i < Size(); ++i )
        {
            if( m_Data[i] == a_Item )
                return true;
        }

        return false;
    }

    //-------------------------------------------------------------------------
    inline int Size() const
    { 
        return m_CurrentSize > BUFFER_SIZE ? BUFFER_SIZE : m_CurrentSize; 
    }

    //-------------------------------------------------------------------------
    inline int GetCurrentIndex() const
    {
        return m_CurrentIndex;
    }

    //-------------------------------------------------------------------------
    inline T* Data()
    { 
        return m_Data; 
    }

    //-------------------------------------------------------------------------
    inline int IndexOfOldest() const
    {
        return m_CurrentSize > BUFFER_SIZE ? m_CurrentIndex : 0;
    }

    //-------------------------------------------------------------------------
    T& operator[](int a_Index)
    {
        int index = ( IndexOfOldest() + a_Index ) % BUFFER_SIZE;
        return m_Data[index];
    }

    //-------------------------------------------------------------------------
    inline const T & Latest()
    {
        return (*this)[Size()-1];
    }

protected:
    T m_Data[BUFFER_SIZE];
    int m_CurrentSize;
    int m_CurrentIndex;
};

