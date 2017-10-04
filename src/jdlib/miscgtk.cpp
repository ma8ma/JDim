// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "miscgtk.h"
#include "miscutil.h"
#include "imgloader.h"


enum
{
    CHAR_BUF = 256 // char の初期化用
};


static struct color_map {
    const char *name;
    const char *rgb;
} constexpr const color_names[] = {
    // basic html colors
    { "red",            "#ff0000" },
    { "fuchsia",        "#ff00ff" },
    { "purple",         "#800080" },
    { "maroon",         "#800000" },
    { "yellow",         "#ffff00" },
    { "lime",           "#00ff00" },
    { "green",          "#008000" },
    { "olive",          "#808000" },
    { "blue",           "#0000ff" },
    { "aqua",           "#00ffff" },
    { "teal",           "#008080" },
    { "navy",           "#000080" },
    { "white",          "#ffffff" },
    { "silver",         "#c0c0c0" },
    { "gray",           "#808080" },
    { "black",          "#000000" },
    { "orange",         "#ffa500" }, // since CSS2.1

    // other X11 colors

    //Red color names
    { "indianred",      "#CD5C5C" },
    { "lightcoral",     "#F08080" },
    { "salmon",         "#FA8072" },
    { "darksalmon",     "#E9967A" },
    { "lightsalmon",    "#FFA07A" },
    { "crimson",        "#DC143C" },
//    { "red",            "#FF0000" },
    { "firebrick",      "#B22222" },
    { "darkred",        "#8B0000" },

    //Pink color names
    { "pink",           "#FFC0CB" },
    { "lightpink",      "#FFB6C1" },
    { "hotpink",        "#FF69B4" },
    { "deeppink",       "#FF1493" },
    { "mediumvioletred","#C71585" },
    { "palevioletred",  "#DB7093" },

    //Orange color names
    { "lightsalmon",    "#FFA07A" },
    { "coral",          "#FF7F50" },
    { "tomato",         "#FF6347" },
    { "orangered",      "#FF4500" },
    { "darkorange",     "#FF8C00" },
//    { "orange",         "#FFA500" },

    //Yellow color names
    { "gold",           "#FFD700" },
//    { "yellow",         "#FFFF00" },
    { "lightyellow",    "#FFFFE0" },
    { "lemonchiffon",   "#FFFACD" },
    { "lightgoldenrodyellow",   "#FAFAD2" },
    { "papayawhip",     "#FFEFD5" },
    { "moccasin",       "#FFE4B5" },
    { "peachpuff",      "#FFDAB9" },
    { "palegoldenrod",  "#EEE8AA" },
    { "khaki",          "#F0E68C" },
    { "darkkhaki",      "#BDB76B" },

    //Purple color names
    { "lavender",       "#E6E6FA" },
    { "thistle",        "#D8BFD8" },
    { "plum",           "#DDA0DD" },
    { "violet",         "#EE82EE" },
    { "orchid",         "#DA70D6" },
//    { "fuchsia",        "#FF00FF" },
    { "magenta",        "#FF00FF" },
    { "mediumorchid",   "#BA55D3" },
    { "mediumpurple",   "#9370DB" },
    { "blueviolet",     "#8A2BE2" },
    { "darkviolet",     "#9400D3" },
    { "darkorchid",     "#9932CC" },
    { "darkmagenta",    "#8B008B" },
//    { "purple",         "#800080" },
    { "indigo",         "#4B0082" },
    { "slateblue",      "#6A5ACD" },
    { "darkslateblue",  "#483D8B" },
    { "mediumslateblue","#7B68EE" },

    //Green color names
    { "greenyellow",    "#ADFF2F" },
    { "chartreuse",     "#7FFF00" },
    { "lawngreen",      "#7CFC00" },
//    { "lime",           "#00FF00" },
    { "limegreen",      "#32CD32" },
    { "palegreen",      "#98FB98" },
    { "lightgreen",     "#90EE90" },
    { "mediumspringgreen",  "#00FA9A" },
    { "springgreen",    "#00FF7F" },
    { "mediumseagreen", "#3CB371" },
    { "seagreen",       "#2E8B57" },
    { "forestgreen",    "#228B22" },
//    { "green",          "#008000" },
    { "darkgreen",      "#006400" },
    { "yellowgreen",    "#9ACD32" },
    { "olivedrab",      "#6B8E23" },
//    { "olive",          "#808000" },
    { "darkolivegreen", "#556B2F" },
    { "mediumaquamarine",   "#66CDAA" },
    { "darkseagreen",   "#8FBC8F" },
    { "lightseagreen",  "#20B2AA" },
    { "darkcyan",       "#008B8B" },
//    { "teal",           "#008080" },

    //Blue color names
//    { "aqua",           "#00FFFF" },
    { "cyan",           "#00FFFF" },
    { "lightcyan",      "#E0FFFF" },
    { "paleturquoise",  "#AFEEEE" },
    { "aquamarine",     "#7FFFD4" },
    { "turquoise",      "#40E0D0" },
    { "mediumturquoise","#48D1CC" },
    { "darkturquoise",  "#00CED1" },
    { "cadetblue",      "#5F9EA0" },
    { "steelblue",      "#4682B4" },
    { "lightsteelblue", "#B0C4DE" },
    { "powderblue",     "#B0E0E6" },
    { "lightblue",      "#ADD8E6" },
    { "skyblue",        "#87CEEB" },
    { "lightskyblue",   "#87CEFA" },
    { "deepskyblue",    "#00BFFF" },
    { "dodgerblue",     "#1E90FF" },
    { "cornflowerblue", "#6495ED" },
    { "mediumslateblue","#7B68EE" },
    { "royalblue",      "#4169E1" },
//    { "blue",           "#0000FF" },
    { "mediumblue",     "#0000CD" },
    { "darkblue",       "#00008B" },
//    { "navy",           "#000080" },
    { "midnightblue",   "#191970" },

    //Brown color names
    { "cornsilk",       "#FFF8DC" },
    { "blanchedalmond", "#FFEBCD" },
    { "bisque",         "#FFE4C4" },
    { "navajowhite",    "#FFDEAD" },
    { "wheat",          "#F5DEB3" },
    { "burlywood",      "#DEB887" },
    { "tan",            "#D2B48C" },
    { "rosybrown",      "#BC8F8F" },
    { "sandybrown",     "#F4A460" },
    { "goldenrod",      "#DAA520" },
    { "darkgoldenrod",  "#B8860B" },
    { "peru",           "#CD853F" },
    { "chocolate",      "#D2691E" },
    { "saddlebrown",    "#8B4513" },
    { "sienna",         "#A0522D" },
    { "brown",          "#A52A2A" },
//    { "maroon",         "#800000" },

    //White color names
//    { "white",          "#FFFFFF" },
    { "snow",           "#FFFAFA" },
    { "honeydew",       "#F0FFF0" },
    { "mintcream",      "#F5FFFA" },
    { "azure",          "#F0FFFF" },
    { "aliceblue",      "#F0F8FF" },
    { "ghostwhite",     "#F8F8FF" },
    { "whitesmoke",     "#F5F5F5" },
    { "seashell",       "#FFF5EE" },
    { "beige",          "#F5F5DC" },
    { "oldlace",        "#FDF5E6" },
    { "floralwhite",    "#FFFAF0" },
    { "ivory",          "#FFFFF0" },
    { "antiquewhite",   "#FAEBD7" },
    { "linen",          "#FAF0E6" },
    { "lavenderblush",  "#FFF0F5" },
    { "mistyrose",      "#FFE4E1" },

    //Grey color names
    { "gainsboro",      "#DCDCDC" },
    { "lightgrey",      "#D3D3D3" },
//    { "silver",         "#C0C0C0" },
    { "darkgray",       "#A9A9A9" },
//    { "gray",           "#808080" },
    { "dimgray",        "#696969" },
    { "lightslategray", "#778899" },
    { "slategray",      "#708090" },
    { "darkslategray",  "#2F4F4F" },
//    { "black",          "#000000" },
};


// Gdk::Color -> 16進数表記の文字列
std::string MISC::color_to_str( const Gdk::Color& color )
{
    // R,G,Bを取得
    int l_rgb[3];
    l_rgb[0] = color.get_red();
    l_rgb[1] = color.get_green();
    l_rgb[2] = color.get_blue();

    return color_to_str( l_rgb );
}

// int[3] -> 16進数表記の文字列
std::string MISC::color_to_str( const int* l_rgb )
{
    // 16進数表記で各色の文字列を連結して格納
    char str_value[ CHAR_BUF ];
    snprintf( str_value, CHAR_BUF, "#%04x%04x%04x", l_rgb[ 0 ], l_rgb[ 1 ], l_rgb[ 2 ] );

#ifdef _DEBUG
    std::cout << "MISC::color_to_str" << std::endl;
    std::cout << l_rgb[0] << ", " << l_rgb[1] << ", " << l_rgb[2] << "->"
              << str_value << std::endl;
#endif

    return str_value;
}

#if GTKMM_CHECK_VERSION(3,0,0)
// Gdk::RGBA -> 16進数表記の文字列
std::string MISC::color_to_str( const Gdk::RGBA& rgba )
{
    int l_rgb[ 3 ];
    l_rgb[ 0 ] = rgba.get_red_u();
    l_rgb[ 1 ] = rgba.get_green_u();
    l_rgb[ 2 ] = rgba.get_blue_u();
    return color_to_str( l_rgb );
}
#endif // GTKMM_CHECK_VERSION(3,0,0)


// htmlカラー (#ffffffなど) -> 16進数表記の文字列
std::string MISC::htmlcolor_to_str( const std::string& _htmlcolor )
{
    std::string htmlcolor = MISC::tolower_str( _htmlcolor );

    if( htmlcolor[ 0 ] != '#' ){
        bool found = false;
        for( const color_map& col : color_names ) {
            if( htmlcolor == col.name ) {
                htmlcolor = col.rgb;
                found = true;
                break;
            }
        }
        if( !found ) return {};
    }

    const int n = ( htmlcolor.length() == 4 ) ? 1 : 2;
    int rgb[ 3 ];

    for( int i = 0; i < 3; ++i ){
        constexpr int offset = 1;
        std::string tmpstr = htmlcolor.substr( offset + ( i * n ), n );
        for( int j = 0; j < ( 3 - n ); ++j ) tmpstr += tmpstr;
        rgb[ i ] = std::stol( tmpstr, nullptr, 16 );
    }

#ifdef _DEBUG
    std::cout << "MISC::htmlcolor_to_gdkcolor color = " << htmlcolor 
              << " r = " << rgb[ 0 ] << " g = " << rgb[ 1 ] << " b = " << rgb[ 2 ] << std::endl;
#endif

    return color_to_str( rgb );
}


// Gdk::Color -> int 変換
guint32 MISC::color_to_int( const Gdk::Color& color )
{
    guint32 red = color.get_red() >> 8;
    guint32 green = color.get_green() >> 8;
    guint32 blue = color.get_blue() >> 8;

    return ( red << 24 ) + ( green << 16 ) + ( blue << 8 );
}


// 使用可能なフォントの一覧を取得
std::set< std::string > MISC::get_font_families()
{
    std::set< std::string > set_out;

    Gtk::DrawingArea dummy;
    std::list< Glib::RefPtr< Pango::FontFamily > > list_families = dummy.get_pango_context()->list_families();
    std::list< Glib::RefPtr< Pango::FontFamily > >::iterator it = list_families.begin();
    for(; it != list_families.end(); ++it ){
#ifdef _DEBUG
        std::cout << (*it)->get_name() << std::endl;
#endif
        set_out.insert( (*it)->get_name() );
    }

    return set_out;
}


// gtk::entryのフォント名を取得
std::string MISC::get_entry_font()
{
    Gtk::Entry entry;
#if GTKMM_CHECK_VERSION(3,0,0)
    return entry.get_style_context()->get_font().to_string();
#else
    return entry.get_style()->get_font().to_string();
#endif
}


// gtk::entryの文字色を16進数表記の文字列で取得
std::string MISC::get_entry_color_text()
{
    Gtk::Entry entry;
#if GTKMM_CHECK_VERSION(3,0,0)
    auto rgba = entry.get_style_context()->get_color( Gtk::STATE_FLAG_NORMAL );
    return color_to_str( rgba );
#else
    Gtk::Window win( Gtk::WINDOW_POPUP );

    win.add( entry );
    win.move( 0,0 );
    win.resize( 1,1 );
    win.show_all();

    return color_to_str( entry.get_style()->get_text( Gtk::STATE_NORMAL ) );
#endif
}


// gtk::entryの背景色を16進数表記の文字列で取得
std::string MISC::get_entry_color_base()
{
    Gtk::Entry entry;
#if GTKMM_CHECK_VERSION(3,0,0)
    // REVIEW: get_background_color()が期待通りに背景色を返さない環境があった
    auto context = entry.get_style_context();
    Gdk::RGBA rgba;
    if( !context->lookup_color( "theme_base_color", rgba ) ) {
#ifdef _DEBUG
        std::cout << "ERROR:MISC::get_entry_color_base() "
                  << "lookup theme_base_color failed." << std::endl;
#endif
    }
    return color_to_str( rgba );
#else
    Gtk::Window win( Gtk::WINDOW_POPUP );

    win.add( entry );
    win.move( 0,0 );
    win.resize( 1,1 );
    win.show_all();

    return color_to_str( entry.get_style()->get_base( Gtk::STATE_NORMAL ) );
#endif
}



// str をクリップボードにコピー
void MISC::CopyClipboard( const std::string& str )
{
    Glib::RefPtr< Gtk::Clipboard > clip = Gtk::Clipboard::get();
    clip->set_text( str );
    clip = Gtk::Clipboard::get( GDK_SELECTION_PRIMARY );
    clip->set_text( str );
}

