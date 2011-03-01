#include <SDL/SDL_opengl.h>

#include <stdexcept>
#include <time.h>
#include <assert.h>

#include "sdl_gl_interface.h"
#include "log.h"

//////////////////////////////////////////////////////////////////////////////////
// Function definitions for SDL_GL_Window:
//////////////////////////////////////////////////////////////////////////////////

SDL_GL_Window::SDL_GL_Window( const int w, const int h, const int bpp, const Uint32 flags, const std::string &title ) :
    screen_( 0 ),
    screen_width_( w ), 
    screen_height_( h ), 
    screen_bpp_( bpp ),
    sdl_video_flags_( flags ),
    title_( title )
{
}

void SDL_GL_Window::init_GL()
{
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );

    glClearColor( 0.0f, 0.0f, 1.0f, 1.0f );
    glClearDepth( 1.0f );
    glDepthFunc( GL_LEQUAL );
    glShadeModel( GL_SMOOTH );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}

void SDL_GL_Window::create_window()
{
    bool ok = false;

    if( SDL_Init( SDL_INIT_VIDEO ) != -1 )
    {
        // TODO: Check return values here, and verify that they have not changed after the SDL_SetVideoMode() call.
        SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
        SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
        SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
        SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
        SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
        SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

        // TODO: Antialiasing level should be configurable.
        SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
        SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 4 );

        // TODO: Vsync should be configurable.
        SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 1 ); 

        if( ( screen_ = SDL_SetVideoMode( screen_width_, screen_height_, screen_bpp_, sdl_video_flags_ ) ) )
        {
            glEnable( GL_MULTISAMPLE );

            // Warp the mouse and then purge the event queue to hide the SDL_MOUSE_MOTION event generated by the warp.
            // TODO: Clean this up!!
            SDL_WarpMouse( 0, 0 );
            SDL_WM_SetCaption( title_.c_str(), NULL );
            SDL_FillRect( screen_, NULL, SDL_MapRGBA( screen_->format, 0, 0, 0, 0 ) );
        
            init_GL();
            reshape_window();
            ok = true;
        }
        else LOG( "SDL_SetVideoMode() failed: " << SDL_GetError() );
    }
    else LOG( "SDL_Init() failed: " << SDL_GetError() );

    if ( !ok ) throw std::runtime_error( "Error creating SDL window." );
}

void SDL_GL_Window::reshape_window( const int w, const int h )
{
    screen_width_ = w;
    screen_height_ = h;

    reshape_window();
}

void SDL_GL_Window::reshape_window()
{
    glViewport( 0, 0,( GLsizei )( screen_width_ ), ( GLsizei )( screen_height_ ) );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0f, ( GLfloat )( screen_width_ ) / ( GLfloat )( screen_height_ ), 1.0f, 1000.0f );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
}

//////////////////////////////////////////////////////////////////////////////////
// Function definitions for SDL_GL_Interface:
//////////////////////////////////////////////////////////////////////////////////

SDL_GL_Interface::SDL_GL_Interface( SDL_GL_Window &window, const int fps_limit ) :
    run_( false ),
    fps_limit_( fps_limit ),
    window_( window )
{
    window_.create_window();
}

SDL_GL_Interface::~SDL_GL_Interface()
{
    SDL_Quit();
}

void SDL_GL_Interface::toggle_fullscreen()
{
    SDL_Surface* s = SDL_GetVideoSurface();

    if( !s || ( SDL_WM_ToggleFullScreen( s ) != 1 ) )
    {
        LOG( "Unable to toggle fullscreen: " << SDL_GetError() );
    }
}

void SDL_GL_Interface::process_events()
{
    SDL_Event event;

    while( SDL_PollEvent( &event ) ) 
    {
        handle_event( event );
    }
}

void SDL_GL_Interface::handle_event( SDL_Event &event )
{
    switch ( event.type )
    {
        case SDL_KEYDOWN:
            handle_key_down_event( event.key.keysym.sym, event.key.keysym.mod );
            break;

        case SDL_KEYUP:
            handle_key_up_event( event.key.keysym.sym, event.key.keysym.mod );
            break;

        case SDL_MOUSEMOTION:
            handle_mouse_motion_event( event.button.button, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel );
            break;

        case SDL_MOUSEBUTTONDOWN:
            handle_mouse_down_event( event.button.button, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel );
            break;

        case SDL_MOUSEBUTTONUP:
            handle_mouse_up_event( event.button.button, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel );
            break;
    
        case SDL_VIDEORESIZE:
            window_.reshape_window( event.resize.w, event.resize.h );
            break;
    
        case SDL_QUIT:
            run_ = false;
            break;
    }
}

void SDL_GL_Interface::main_loop()
{
    run_ = true;

    // TODO: Pull the high-resolution clock stuff below out into a class, and add
    //       similar calculations for Windows using QueryPerformanceTimer.

    timespec
        resolution,
        last_time,
        current_time;

    int result = clock_getres( CLOCK_MONOTONIC_RAW, &resolution );
    if ( result == -1 )
        throw std::runtime_error( "Unable to determine clock resolution" );

    result = clock_gettime( CLOCK_MONOTONIC_RAW, &last_time );
    if ( result == -1 )
        throw std::runtime_error( "Unable to read clock" );

    while ( run_ )
    {
        result = clock_gettime( CLOCK_MONOTONIC_RAW, &current_time );
        if ( result == -1 )
            throw std::runtime_error( "Unable to read clock" );

        const double step_time_seconds = 
            double( current_time.tv_sec - last_time.tv_sec ) +
            double( current_time.tv_nsec - last_time.tv_nsec ) / 1000000000.0;

        if ( step_time_seconds > 1.0 / fps_limit_ )
        {
            last_time = current_time;
            
            process_events();
            do_one_step( float( step_time_seconds ) );
            
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glMatrixMode( GL_MODELVIEW );
            glLoadIdentity();
            render();
            glFlush();
            SDL_GL_SwapBuffers();
        }
    }
}
