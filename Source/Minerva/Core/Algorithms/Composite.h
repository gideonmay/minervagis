
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Authors: Adam Kubach and Perry Miller
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_ALGORITHMS_COMPOSITE_H__
#define __MINERVA_CORE_ALGORITHMS_COMPOSITE_H__

#include "Minerva/Core/Algorithms/SubRegion.h"

#include "Usul/Functions/Color.h"
#include "Usul/Math/MinMax.h"
#include "Usul/Math/Vector4.h"

#include "osg/Image"

namespace Minerva {
namespace Core {
namespace Algorithms {
namespace Composite {
  
template<class Alphas, unsigned int bands> struct Compositor;
  
template<class Alphas>
struct Compositor<Alphas,1>
{
  static void composite ( osg::Image& result, const osg::Image& image, const Alphas &alphas, float alpha )
  {
    unsigned char       *dst ( result.data() );
    const unsigned char *src ( image.data()  );
    
    const unsigned int width  ( result.s() );
    const unsigned int height ( result.t() );
    
    // We only composite images of the same size.
    if ( ( static_cast<int> ( width ) != image.s() ) || ( static_cast<int> ( height ) != image.t() ) )
      return;
    
    const unsigned int size ( width * height );
    const bool alphaMapEmpty ( alphas.empty() );
    bool hasExtraAlpha ( false );
    typename Alphas::const_iterator iter ( alphas.end() );
    
    for ( unsigned int i = 0; i < size; ++i )
    {
      // Copy the color channels.
      unsigned char r ( src[0] );
      unsigned char g ( src[0] );
      unsigned char b ( src[0] );
      
      // Is the color in the alpha table?
      if ( false == alphaMapEmpty )
      {
        iter = alphas.find ( Usul::Functions::Color::pack ( r, g, b, 0 ) );
        hasExtraAlpha = ( alphas.end() != iter );
      }
      
      {
        // Get correct alpha.
        const unsigned char useThisAlpha ( ( hasExtraAlpha ) ? ( static_cast < unsigned char > ( iter->second ) ) : 
                                          ( static_cast < unsigned char > ( alpha * 255 ) ) );
        
        // Normalize between zero and one.
        const float a ( static_cast < float > ( useThisAlpha ) / 255.5f );
        
        // Composite.
        dst[0] = static_cast < unsigned char > ( dst[0] * ( 1 - a ) + ( r * a ) );
        dst[1] = static_cast < unsigned char > ( dst[1] * ( 1 - a ) + ( g * a ) );
        dst[2] = static_cast < unsigned char > ( dst[2] * ( 1 - a ) + ( b * a ) );
        
        // Since the alpha has been accounted for above, make the pixel completely opaque.
        if ( a > 0.0 )
          dst[3] = 255;
      }
      
      dst += 4;
      src += 1;
    }
  }
};


template<class Alphas>
struct Compositor<Alphas,2>
{
  static void composite ( osg::Image& result, const osg::Image& image, const Alphas &alphas, float alpha )
  {
    unsigned char       *dst ( result.data() );
    const unsigned char *src ( image.data()  );
    
    const unsigned int width  ( result.s() );
    const unsigned int height ( result.t() );
    
    // We only composite images of the same size.
    if ( ( static_cast<int> ( width ) != image.s() ) || ( static_cast<int> ( height ) != image.t() ) )
      return;
    
    const unsigned int size ( width * height );
    const bool alphaMapEmpty ( alphas.empty() );
    bool hasExtraAlpha ( false );
    typename Alphas::const_iterator iter ( alphas.end() );
    
    const bool hasOverallAlpha ( alpha < 1.0f );
    
    for ( unsigned int i = 0; i < size; ++i )
    {
      // Copy the color channels.
      unsigned char r ( src[0] );
      unsigned char g ( src[0] );
      unsigned char b ( src[0] );
      unsigned char a ( src[1] );
      
      // Is the color in the alpha table?
      if ( false == alphaMapEmpty )
      {
        iter = alphas.find ( Usul::Functions::Color::pack ( r, g, b, 0 ) );
        hasExtraAlpha = ( alphas.end() != iter );
      }
      
      {
        // Get correct alpha.
        const unsigned char useThisAlpha ( ( hasExtraAlpha )   ? ( static_cast < unsigned char > ( iter->second ) ) : 
                                          ( ( hasOverallAlpha ) ? ( static_cast < unsigned char > ( alpha * a ) ) : ( a ) ) );
        
        // Normalize between zero and one.
        const float a ( static_cast < float > ( useThisAlpha ) / 255.5f );
        
        // Composite.
        dst[0] = static_cast < unsigned char > ( dst[0] * ( 1 - a ) + ( r * a ) );
        dst[1] = static_cast < unsigned char > ( dst[1] * ( 1 - a ) + ( g * a ) );
        dst[2] = static_cast < unsigned char > ( dst[2] * ( 1 - a ) + ( b * a ) );
        
        // Since the alpha has been accounted for above, make the pixel completely opaque.
        if ( a > 0.0 )
          dst[3] = 255;
      }
      
      dst += 4;
      src += 2;
    }
  }
};
  
template<class Alphas>
struct Compositor<Alphas,3>
{
  static void composite ( osg::Image& result, const osg::Image& image, const Alphas &alphas, float alpha )
  {
    unsigned char       *dst ( result.data() );
    const unsigned char *src ( image.data()  );
    
    const unsigned int width  ( result.s() );
    const unsigned int height ( result.t() );
    
    // We only composite images of the same size.
    if ( ( static_cast<int> ( width ) != image.s() ) || ( static_cast<int> ( height ) != image.t() ) )
      return;
    
    const unsigned int size ( width * height );
    const bool alphaMapEmpty ( alphas.empty() );
    bool hasExtraAlpha ( false );
    typename Alphas::const_iterator iter ( alphas.end() );
    
    for ( unsigned int i = 0; i < size; ++i )
    {
      // Copy the color channels.
      unsigned char r ( src[0] );
      unsigned char g ( src[1] );
      unsigned char b ( src[2] );
      
      // Is the color in the alpha table?
      if ( false == alphaMapEmpty )
      {
        iter = alphas.find ( Usul::Functions::Color::pack ( r, g, b, 0 ) );
        hasExtraAlpha = ( alphas.end() != iter );
      }
      
      {
        // Get correct alpha.
        const unsigned char useThisAlpha ( ( hasExtraAlpha )   ? ( static_cast < unsigned char > ( iter->second ) ) : 
                                          ( static_cast < unsigned char > ( alpha * 255 ) ) );
        
        // Normalize between zero and one.
        const float a ( static_cast < float > ( useThisAlpha ) / 255.5f );
        
        // Composite.
        dst[0] = static_cast < unsigned char > ( dst[0] * ( 1 - a ) + ( r * a ) );
        dst[1] = static_cast < unsigned char > ( dst[1] * ( 1 - a ) + ( g * a ) );
        dst[2] = static_cast < unsigned char > ( dst[2] * ( 1 - a ) + ( b * a ) );
        
        // Since the alpha has been accounted for above, make the pixel completely opaque.
        if ( a > 0.0 )
          dst[3] = 255;
      }
      
      dst += 4;
      src += 3;
    }
  }
};
  
  
template<class Alphas>
struct Compositor<Alphas,4>
{
  static void composite ( osg::Image& result, const osg::Image& image, const Alphas &alphas, float alpha )
  {
    unsigned char       *dst ( result.data() );
    const unsigned char *src ( image.data()  );
    
    const unsigned int width  ( result.s() );
    const unsigned int height ( result.t() );
    
    // We only composite images of the same size.
    if ( ( static_cast<int> ( width ) != image.s() ) || ( static_cast<int> ( height ) != image.t() ) )
      return;
    
    const unsigned int size ( width * height );
    const bool alphaMapEmpty ( alphas.empty() );
    bool hasExtraAlpha ( false );
    typename Alphas::const_iterator iter ( alphas.end() );
    
    const bool hasOverallAlpha ( alpha < 1.0f );
    
    for ( unsigned int i = 0; i < size; ++i )
    {
      // Copy the color channels.
      unsigned char r ( src[0] );
      unsigned char g ( src[1] );
      unsigned char b ( src[2] );
      unsigned char a ( src[3] );
      
      // Is the color in the alpha table?
      if ( false == alphaMapEmpty )
      {
        iter = alphas.find ( Usul::Functions::Color::pack ( r, g, b, 0 ) );
        hasExtraAlpha = ( alphas.end() != iter );
      }
      
      {
        // Get correct alpha.
        const unsigned char useThisAlpha ( ( hasExtraAlpha )   ? ( static_cast < unsigned char > ( iter->second ) ) : 
                                         ( ( hasOverallAlpha ) ? ( static_cast < unsigned char > ( alpha * a ) ) : ( a ) ) );
        
        // Normalize between zero and one.
        const float a ( static_cast < float > ( useThisAlpha ) / 255.5f );
        
        // Composite.
        dst[0] = static_cast < unsigned char > ( dst[0] * ( 1 - a ) + ( r * a ) );
        dst[1] = static_cast < unsigned char > ( dst[1] * ( 1 - a ) + ( g * a ) );
        dst[2] = static_cast < unsigned char > ( dst[2] * ( 1 - a ) + ( b * a ) );
        
        // Since the alpha has been accounted for above, make the pixel completely opaque.
        if ( a > 0.0 )
          dst[3] = 255;
      }
      
      dst += 4;
      src += 4;
    }
  }
};
  
  
///////////////////////////////////////////////////////////////////////////////
//
//  Composite two raster images.
//
///////////////////////////////////////////////////////////////////////////////

template < class Alphas >
inline void raster ( osg::Image& result, const osg::Image& image, const Alphas &alphas, float alpha )
{
  // We only handle these cases.
  const GLenum format ( image.getPixelFormat() );
  switch ( format )
  {
    case GL_LUMINANCE:
      Compositor<Alphas,1>::composite ( result, image, alphas, alpha );
      break;
    case GL_LUMINANCE_ALPHA:
      Compositor<Alphas,2>::composite ( result, image, alphas, alpha );
      break;
    case GL_RGB:
      Compositor<Alphas,3>::composite ( result, image, alphas, alpha );
      break;
    case GL_RGBA:
      Compositor<Alphas,4>::composite ( result, image, alphas, alpha );
      break;
  };
}

 

}
}
}
}

#endif // __MINERVA_CORE_ALGORITHMS_COMPOSITE_H__
