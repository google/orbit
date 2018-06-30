//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once
#include <atomic>

//-----------------------------------------------------------------------------
template < class T, int BlockSize > struct BlockChain;

//-----------------------------------------------------------------------------
template < class T, int Size > struct Block
{
    Block(BlockChain<T, Size>* a_Chain, Block<T, Size>* a_Prev)
        : m_Prev(a_Prev)
        , m_Next(nullptr)
        , m_Chain(a_Chain)
        , m_Size(0)
    {
    }

    ~Block()
    {
    }

    void Add(const T & a_Item)
    {
        if (m_Size == Size)
        {
            if( m_Next == nullptr )
            {
                m_Next = new Block<T, Size>(m_Chain, this);
            }

            m_Chain->m_Current = m_Next;
            ++m_Chain->m_NumBlocks;
            m_Next->Add(a_Item);
            return;
        }

        assert( m_Size < Size );
        m_Data[m_Size] = a_Item;
        ++m_Size;
        ++m_Chain->m_NumItems;
    }

    Block<T, Size>*       m_Prev;
    Block<T, Size>*       m_Next;
    T                     m_Data[Size];
    BlockChain<T, Size>*  m_Chain;
    std::atomic<int>      m_Size;
};

//-----------------------------------------------------------------------------
template<class T, int BlockSize> struct BlockIterator
{
    BlockIterator(Block<T, BlockSize>* a_Block) : m_Block(a_Block)
    {
        m_Index = (m_Block && (m_Block->m_Size > 0)) ? 0 : -1;
    }

    T& operator*()
    {
        return m_Block->m_Data[m_Index];
    }

    bool operator!=(const BlockIterator & other) const
    {
        bool returnValue = m_Index != other.m_Index;
        return returnValue;
    }

    BlockIterator& operator++()
    {
        if (++m_Index == m_Block->m_Size)
        {
            if (m_Block->m_Next && m_Block->m_Next->m_Size > 0)
            {
                m_Index = 0;
                m_Block = m_Block->m_Next;
            }
            else
            {
                m_Index = -1;
            }
        }

        return *this;
    }

    Block<T, BlockSize>* m_Block;
    int m_Index;
};

//-----------------------------------------------------------------------------
template < class T, int BlockSize > struct BlockChain
{
    BlockChain() : m_NumBlocks(1), m_NumItems(0)
    {
        m_Root = m_Current = new Block<T, BlockSize>(this, nullptr);
    }

    ~BlockChain()
    {
        // Find last block in chain
        while( m_Current->m_Next )
            m_Current = m_Current->m_Next;

        Block<T, BlockSize>* prev = m_Current;
        while (prev)
        {
            prev = m_Current->m_Prev;
            delete m_Current;
            m_Current = prev;
        }
    }

    void push_back(const T & a_Item)
    {
        m_Current->Add(a_Item);
    }

    void push_back( const T* a_Array, unsigned int a_Num )
    {
        for( unsigned int i = 0; i < a_Num; ++i )
            m_Current->Add( a_Array[i] );
    }

    void push_back_n( const T & a_Item, unsigned int a_Num )
    {
        for( unsigned int i = 0; i < a_Num; ++i )
            m_Current->Add( a_Item );
    }

    void clear()
    {
        m_Root->m_Size = 0;
        m_Root->m_Next = nullptr;
        m_NumItems     = 0;
        m_NumBlocks    = 1;

        Block<T, BlockSize>* prev = m_Current;
        while (prev!=m_Root)
        {
            prev = m_Current->m_Prev;
            delete m_Current;
            m_Current = prev;
        }

        m_Current = m_Root;
    }

    void Reset()
    {
        Block<T, BlockSize>* blockPtr = m_Root;
        while( blockPtr )
        {
            blockPtr->m_Size = 0;
            blockPtr = blockPtr->m_Next;
        }

        m_NumItems = 0;
        m_NumBlocks = 1;
        m_Current = m_Root;
    }

    bool keep( int a_MaxElems )
    {
        bool hasDeleted = false;
        a_MaxElems = std::max( BlockSize + 1, a_MaxElems );

        while( m_NumItems > a_MaxElems )
        {
            --m_NumBlocks;
            m_NumItems -= BlockSize;

            m_Root = m_Root->m_Next;

            assert( m_Root->m_Prev );
            assert( m_Root->m_Prev != m_Current );

            delete m_Root->m_Prev;
            m_Root->m_Prev = nullptr;
            hasDeleted = true;
        }

        return hasDeleted;
    }

    int size() const { return m_NumItems; }
    
    T* SlowAt( int a_Index )
    {
        if( a_Index < m_NumItems && a_Index >= 0 )
        {
            int count = 1;
            Block<T, BlockSize>* block = m_Root;
            while( count * BlockSize < a_Index && block && block->m_Next )
            {
                block = block->m_Next;
                ++count;
            }

            int index = a_Index % BlockSize;
            return &block->m_Data[index];
        }

        return nullptr;
    }
    
    BlockIterator<T, BlockSize> begin(){ return BlockIterator<T, BlockSize>(m_Root); }
    BlockIterator<T, BlockSize> end(){ return BlockIterator<T, BlockSize>(nullptr); }

    Block<T, BlockSize>* m_Root;
    Block<T, BlockSize>* m_Current;
    std::atomic<int>     m_NumBlocks;
    std::atomic<int>     m_NumItems;
};
