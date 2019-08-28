//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "GlCanvas.h"
#include "TextRenderer.h"
#include "TextBox.h"
#include "OpenGl.h"
#include "Capture.h"
#include "Params.h"

//-----------------------------------------------------------------------------
TextBox::TextBox() : m_Pos( Vec2::Zero() )
                   , m_Size( Vec2(100.f, 10.f) )
                   , m_MainFrameCounter( -1 )
                   , m_Selected( false )
                   , m_TextY( FLT_MAX )
{
    Update();
}

//-----------------------------------------------------------------------------
TextBox::TextBox( const Vec2 & a_Pos
                , const Vec2 & a_Size
                , const std::string & a_Text
                , TextRenderer * a_Renderer
                , const Color & a_Color )
                : m_Pos( a_Pos )
                , m_Size( a_Size )
                , m_Text( a_Text )
                , m_Color( a_Color )
                , m_MainFrameCounter( -1 )
                , m_Selected( false )
{
    Update();
}

//-----------------------------------------------------------------------------
TextBox::TextBox( const Vec2 & a_Pos
                , const Vec2 & a_Size
                , TextRenderer * a_Renderer
                , const Color & a_Color )
                : m_Pos( a_Pos )
                , m_Size( a_Size )
                , m_Color( a_Color )
                , m_MainFrameCounter( -1 )
                , m_Selected( false )
{
    Update();
}

//-----------------------------------------------------------------------------
TextBox::TextBox( const Vec2 & a_Pos
                , const Vec2 & a_Size )
                : m_Pos( a_Pos )
                , m_Size( a_Size )
                , m_MainFrameCounter( -1 )
                , m_Selected( false )
{
    Update();
}



//-----------------------------------------------------------------------------
TextBox::~TextBox()
{

}

//-----------------------------------------------------------------------------
void TextBox::Update()
{
    m_Min = m_Pos;
    m_Max = m_Pos + Vec2( fabs(m_Size[0]), fabs(m_Size[1]) );
}

//-----------------------------------------------------------------------------
float TextBox::GetScreenSize( const TextRenderer & a_TextRenderer )
{
    float worldWidth = a_TextRenderer.GetSceneBox().m_Size[0];
    float screenSize = (float)a_TextRenderer.GetCanvas()->getWidth();

    return ( m_Size[0] / worldWidth ) * screenSize;
}

//-----------------------------------------------------------------------------
void TextBox::Draw( TextRenderer & a_TextRenderer
                  , float a_MinX
                  , bool a_Visible
                  , bool a_RightJustify
                  , bool isInactive
                  , unsigned int a_ID
                  , bool a_IsPicking 
                  , bool a_IsHighlighted )
{
    bool isCoreActivity = m_Timer.IsType(Timer::CORE_ACTIVITY);
    bool isSameThreadIdAsSelected = isCoreActivity &&
                                    m_Timer.m_TID == Capture::GSelectedThreadId;

    if( Capture::GSelectedThreadId != 0 && isCoreActivity && !isSameThreadIdAsSelected )
    {
        isInactive = true;
    }

    static unsigned char g = 100;
    Color grey(g, g, g, 255);
	static Color selectionColor( 0, 128, 255, 255 );
    
    Color col = isSameThreadIdAsSelected ? m_Color : isInactive ? grey : m_Color;

    if( this == Capture::GSelectedTextBox )
    {
        col = selectionColor;
    }

    float z = a_IsHighlighted ? GlCanvas::Z_VALUE_CONTEXT_SWITCH : isInactive ? GlCanvas::Z_VALUE_BOX_INACTIVE : GlCanvas::Z_VALUE_BOX_ACTIVE;
    
    if( !a_IsPicking )
    {
        glColor4ubv( &col[0] );
    }
    else
    {
        GLubyte* color = (GLubyte*)&a_ID;
        color[3] = 255;
        glColor4ubv( (GLubyte*)&a_ID );
    }

    if( a_Visible )
    {
        glBegin( GL_QUADS );
        glVertex3f( m_Pos[0], m_Pos[1], z );
        glVertex3f( m_Pos[0], m_Pos[1] + m_Size[1], z );
        glVertex3f( m_Pos[0] + m_Size[0], m_Pos[1] + m_Size[1], z );
        glVertex3f( m_Pos[0] + m_Size[0], m_Pos[1], z );
        glEnd();

        static Color s_Color(255, 255, 255, 255);
            
        float posX = std::max( m_Pos[0], a_MinX );
        if (a_RightJustify)
        {
            posX += m_Size[0];
        }

        float maxSize = m_Pos[0]+m_Size[0] - posX;

        Function* func = Capture::GSelectedFunctionsMap[m_Timer.m_FunctionAddress];
        std::string text = Format( "%s %s", func ? func->PrettyName().c_str() : "", m_Text.c_str() );

        if( !a_IsPicking && !isCoreActivity )
        {
            a_TextRenderer.AddText( text.c_str()
                                  , posX
                                  , m_TextY == FLT_MAX ? m_Pos[1] + 1.f : m_TextY
                                  , GlCanvas::Z_VALUE_TEXT
                                  , s_Color
                                  , maxSize
                                  , a_RightJustify );
        }

        glColor4ubv( &grey[0] );
    }

    glBegin( GL_LINES );
    glVertex3f( m_Pos[0], m_Pos[1], z );
    glVertex3f( m_Pos[0], m_Pos[1] + m_Size[1], z );
    glEnd();
}
