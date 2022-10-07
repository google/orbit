/**
@page text-rendering Text Rendering

@section simple   Simple text rendering

The following complete example creates a window that displays "Hello, World" centered vertically and horizontally:

@code
  ...

  label_t * label = label_new( "Hello, world", "Times New Roman", 36,
                               window_width/2, window_height/2,
                               "center", "center" )
  ...

  label_render( label );

  ...
@endcode




*/
